	[BITS   64]
	section .text
	global  _start

_start:
	cld

	mov    r12, rdi

	extern _bss_start
	extern _bss_end
	mov    rdi, _bss_start
	mov    rcx, _bss_end
	sub    rcx, rdi
	xor    rax, rax
	rep    stosb

	extern kernel_main
	mov    rdi, r12
	call   kernel_main

.hang:
	cli
	hlt
	jmp .hang
