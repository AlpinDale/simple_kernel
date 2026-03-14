	[BITS 16]
	[ORG  0x7C00]

%ifndef STAGE2_SECTORS
%error "STAGE2_SECTORS must be defined"
%endif

%define STAGE2_SEGMENT 0x0800
%define STAGE2_OFFSET  0x0000
%define STAGE2_LBA     1

start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00
	sti

	mov [boot_drive], dl
	mov si, boot_msg
	call print_string

	mov word [dap.count], STAGE2_SECTORS
	mov word [dap.offset], STAGE2_OFFSET
	mov word [dap.segment], STAGE2_SEGMENT
	mov dword [dap.lba_low], STAGE2_LBA
	mov dword [dap.lba_high], 0

	mov si, dap
	mov ah, 0x42
	mov dl, [boot_drive]
	int 0x13
	jc disk_error

	jmp STAGE2_SEGMENT:STAGE2_OFFSET

disk_error:
	mov si, error_msg
	call print_string
	cli
	hlt
	jmp $

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

boot_msg db 'Loading stage2...', 13, 10, 0
error_msg db 'Stage2 load failed', 13, 10, 0
boot_drive db 0

dap:
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

	times 510-($-$$) db 0
	dw 0xAA55
