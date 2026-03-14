	[BITS 16]
	[ORG  0x8000]

%ifndef KERNEL_START_LBA
%error "KERNEL_START_LBA must be defined"
%endif

%ifndef KERNEL_SECTORS
%error "KERNEL_SECTORS must be defined"
%endif

%ifndef KERNEL_SIZE_BYTES
%error "KERNEL_SIZE_BYTES must be defined"
%endif

%define BOOT_INFO_ADDR      0x5000
%define MEMORY_MAP_ADDR     0x5400
%define KERNEL_LOAD_SEGMENT 0x1000
%define PAGE_TABLE_BASE     0x1000
%define PM_STACK_TOP        0x90000
%define LM_STACK_TOP        0x90000
%define MEMORY_MAP_LIMIT    64

%define BOOT_INFO_MAGIC_OFF          0
%define BOOT_INFO_VERSION_OFF        4
%define BOOT_INFO_COUNT_OFF          8
%define BOOT_INFO_MAP_ADDR_OFF      16
%define BOOT_INFO_KERNEL_START_OFF  24
%define BOOT_INFO_KERNEL_END_OFF    32
%define BOOT_INFO_KERNEL_VIRT_OFF   40
%define BOOT_INFO_PML4_OFF          48

stage2_start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7F00
	sti

	mov [boot_drive], dl
	mov si, loading_msg
	call print_string

	call collect_e820_map
	call load_kernel
	call fill_boot_info

	cli
	lgdt [gdt_descriptor]
	call enable_a20
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	jmp 0x08:protected_mode_start

print_string:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0E
	mov bx, 0x0007
	int 0x10
	jmp print_string
.done:
	ret

load_kernel:
	mov dword [kernel_lba_low], KERNEL_START_LBA
	mov dword [kernel_lba_high], 0
	mov word [kernel_sectors_left], KERNEL_SECTORS
	mov word [kernel_segment], KERNEL_LOAD_SEGMENT

.next_chunk:
	mov ax, [kernel_sectors_left]
	test ax, ax
	jz .done
	cmp ax, 127
	jbe .set_count
	mov ax, 127
.set_count:
	mov [disk_packet.count], ax
	mov word [disk_packet.offset], 0
	mov ax, [kernel_segment]
	mov [disk_packet.segment], ax
	mov eax, [kernel_lba_low]
	mov [disk_packet.lba_low], eax
	mov eax, [kernel_lba_high]
	mov [disk_packet.lba_high], eax

	mov si, disk_packet
	mov ah, 0x42
	mov dl, [boot_drive]
	int 0x13
	jc load_error

	movzx eax, word [disk_packet.count]
	sub [kernel_sectors_left], ax
	shl eax, 5
	add [kernel_segment], ax
	mov eax, [disk_packet.lba_low]
	movzx ecx, word [disk_packet.count]
	add eax, ecx
	mov [kernel_lba_low], eax
	adc dword [kernel_lba_high], 0
	jmp .next_chunk

.done:
	ret

load_error:
	mov si, load_error_msg
	call print_string
	cli
	hlt
	jmp $

collect_e820_map:
	xor ebx, ebx
	mov di, MEMORY_MAP_ADDR
	mov word [memory_map_count], 0

.loop:
	cmp word [memory_map_count], MEMORY_MAP_LIMIT
	jae .done
	mov eax, 0xE820
	mov edx, 0x534D4150
	mov ecx, 24
	mov dword [di + 20], 1
	int 0x15
	jc .done
	cmp eax, 0x534D4150
	jne .done
	add di, 24
	inc word [memory_map_count]
	test ebx, ebx
	jnz .loop

.done:
	ret

fill_boot_info:
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_MAGIC_OFF], 0xB007B007
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_VERSION_OFF], 1
	movzx eax, word [memory_map_count]
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_COUNT_OFF], eax
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_COUNT_OFF + 4], 0

	mov dword [BOOT_INFO_ADDR + BOOT_INFO_MAP_ADDR_OFF], MEMORY_MAP_ADDR
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_MAP_ADDR_OFF + 4], 0

	mov dword [BOOT_INFO_ADDR + BOOT_INFO_KERNEL_START_OFF], 0x10000
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_KERNEL_START_OFF + 4], 0
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_KERNEL_END_OFF], 0x10000 + KERNEL_SIZE_BYTES
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_KERNEL_END_OFF + 4], 0
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_KERNEL_VIRT_OFF], 0x10000
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_KERNEL_VIRT_OFF + 4], 0
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_PML4_OFF], PAGE_TABLE_BASE
	mov dword [BOOT_INFO_ADDR + BOOT_INFO_PML4_OFF + 4], 0
	ret

enable_a20:
	in al, 0x92
	or al, 2
	out 0x92, al
	ret

gdt_start:
	dq 0x0000000000000000
	dq 0x00CF9A000000FFFF
	dq 0x00CF92000000FFFF

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

gdt64_start:
	dq 0x0000000000000000
	dq 0x00209A0000000000
	dq 0x0000920000000000

gdt64_end:

gdt64_descriptor:
	dw gdt64_end - gdt64_start - 1
	dq gdt64_start

	[BITS 32]

protected_mode_start:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, PM_STACK_TOP

	call setup_page_tables
	lgdt [gdt64_descriptor]

	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	mov ecx, 0xC0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax
	jmp 0x08:long_mode_start

setup_page_tables:
	mov edi, PAGE_TABLE_BASE
	xor eax, eax
	mov ecx, 3072
	rep stosd

	mov eax, PAGE_TABLE_BASE
	mov cr3, eax

	mov dword [PAGE_TABLE_BASE + 0x0000], PAGE_TABLE_BASE + 0x1000 + 0x03
	mov dword [PAGE_TABLE_BASE + 0x1000], PAGE_TABLE_BASE + 0x2000 + 0x03

	mov edi, PAGE_TABLE_BASE + 0x2000
	mov eax, 0x00000083
	mov ecx, 512

.map_loop:
	mov [edi], eax
	mov dword [edi + 4], 0
	add eax, 0x200000
	add edi, 8
	loop .map_loop
	ret

	[BITS 64]

long_mode_start:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov rsp, LM_STACK_TOP
	mov rdi, BOOT_INFO_ADDR
	mov rax, 0x10000
	call rax

	cli
	hlt
	jmp $

loading_msg db 'Loading kernel...', 13, 10, 0
load_error_msg db 'Kernel load failed', 13, 10, 0
boot_drive db 0
memory_map_count dw 0
kernel_sectors_left dw 0
kernel_segment dw 0
kernel_lba_low dd 0
kernel_lba_high dd 0

disk_packet:
	db 0x10
	db 0
.count:
	dw 0
.offset:
	dw 0
.segment:
	dw 0
.lba_low:
	dd 0
.lba_high:
	dd 0
