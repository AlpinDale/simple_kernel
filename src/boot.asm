	; Minimal x86_64 bootloader - Stage 1
	; Boots from a simple disk image (floppy-like)

	[BITS 16]
	[ORG  0x7C00]

stage1_start:
	;   Set up segment registers
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00
	sti

	;   Save boot drive (both for stage 1 and pass to stage 2)
	mov [boot_drive], dl
	mov [boot_drive_for_stage2], dl

	;    Print boot message
	mov  si, boot_msg
	call print_string

	;   Load stage 2 bootloader (3 sectors starting at sector 2)
	mov ax, 0x07E0; Load to 0x7E00 (right after boot sector)
	mov es, ax
	xor bx, bx

	mov ah, 0x02; Read sectors
	mov al, 3; 3 sectors for stage 2
	mov ch, 0; Cylinder
	mov cl, 2; Start at sector 2
	mov dh, 0; Head
	mov dl, [boot_drive]
	int 0x13

	jc disk_error

	;   Jump to stage 2
	jmp 0x07E0:0x0000

disk_error:
	mov  si, error_msg
	call print_string
	cli
	hlt

print_string:
	lodsb
	or  al, al
	jz  .done
	mov ah, 0x0E
	mov bh, 0
	int 0x10
	jmp print_string

.done:
	ret

boot_msg db 'Booting...', 13, 10, 0
error_msg db 'Disk error!', 13, 10, 0
boot_drive db 0
boot_drive_for_stage2 db 0

	;     Pad to 510 bytes and add boot signature
	times 510-($-$$) db 0
	dw    0xAA55

	;     ============================================
	;     Stage 2 - Protected/Long mode setup
	;     ============================================
	[BITS 16]

stage2_start:
	;   Get boot drive from stage 1
	mov dl, [0x7C00 + (boot_drive_for_stage2 - stage1_start)]
	mov [boot_drive_stage2], dl

	;    Print loading message
	mov  si, loading_msg
	call print_string_stage2

	;   Load kernel from disk to 0x10000
	mov ax, 0x1000
	mov es, ax
	xor bx, bx

	mov ah, 0x02; Read sectors
	mov al, 64; Read 64 sectors (32KB)
	mov ch, 0; Cylinder
	mov cl, 5; Start at sector 5 (after boot + stage2)
	mov dh, 0; Head
	mov dl, [boot_drive_stage2]
	int 0x13

	jc disk_error_stage2

	;    Print success
	mov  si, loaded_msg
	call print_string_stage2

	;    Enter protected mode
	cli
	lgdt [gdt_descriptor]

	;   Enable A20
	in  al, 0x92
	or  al, 2
	out 0x92, al

	;   Set PE bit
	mov eax, cr0
	or  eax, 1
	mov cr0, eax

	;   Far jump to 32-bit code
	jmp 0x08:protected_mode_start

print_string_stage2:
	pusha
	mov ah, 0x0E

.loop:
	lodsb
	or  al, al
	jz  .done
	int 0x10
	jmp .loop

.done:
	popa
	ret

disk_error_stage2:
	mov  si, error_msg_stage2
	call print_string_stage2
	cli
	hlt

	loading_msg db 'Loading kernel...', 13, 10, 0
	loaded_msg db 'Kernel loaded!', 13, 10, 0
	error_msg_stage2 db 'Load error!', 13, 10, 0
	boot_drive_stage2 db 0

	; GDT for protected mode

gdt_start:
	dq 0x0000000000000000; Null descriptor
	dq 0x00CF9A000000FFFF; Code segment
	dq 0x00CF92000000FFFF; Data segment

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd 0x7E00 + (gdt_start - stage2_start)

	[BITS 32]

protected_mode_start:
	;   Set up segments
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	;   Set up stack
	mov esp, 0x90000

	;   Set up page tables for long mode
	;   Clear 16KB for page tables
	mov edi, 0x1000
	mov cr3, edi
	xor eax, eax
	mov ecx, 4096
	rep stosd

	;   Set up page tables (identity map first 2MB)
	mov edi, 0x1000

	;   PML4[0] -> PDPT at 0x2000
	mov dword [edi], 0x2003
	add edi, 0x1000

	;   PDPT[0] -> PD at 0x3000
	mov dword [edi], 0x3003
	add edi, 0x1000

	;   PD[0] -> PT at 0x4000
	mov dword [edi], 0x4003
	add edi, 0x1000

	;   PT - Identity map 512 pages (2MB)
	mov ebx, 0x00000003
	mov ecx, 512

.set_page:
	mov  [edi], ebx
	add  ebx, 0x1000
	add  edi, 8
	loop .set_page

	;    Load 64-bit GDT
	lgdt [gdt64_descriptor]

	;   Enable PAE
	mov eax, cr4
	or  eax, 1 << 5
	mov cr4, eax

	;   Enable long mode
	mov ecx, 0xC0000080
	rdmsr
	or  eax, 1 << 8
	wrmsr

	;   Enable paging
	mov eax, cr0
	or  eax, 1 << 31
	mov cr0, eax

	;   Jump to 64-bit code
	jmp 0x08:long_mode_start

	; 64-bit GDT

gdt64_start:
	dq 0x0000000000000000; Null
	dq 0x00209A0000000000; 64-bit code segment
	dq 0x0000920000000000; Data segment

gdt64_end:

gdt64_descriptor:
	dw gdt64_end - gdt64_start - 1
	dq 0x7E00 + (gdt64_start - stage2_start)

	[BITS 64]

long_mode_start:
	;   Set up segments
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	;   Set up stack
	mov rsp, 0x200000

	;    Jump to kernel at 0x10000
	mov  rax, 0x10000
	call rax

	; If kernel returns, halt
	cli

.hang:
	hlt
	jmp .hang

	;     Pad stage 2 to 3 sectors (1536 bytes total)
	times (1536 - ($ - stage2_start)) db 0
