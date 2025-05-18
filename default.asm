global main
extern ExitProcess
section .bss
section .data
section .text
main:
	sub rsp, 360
	mov qword [rsp+0], 10
	mov rdx, [rsp+0]
	add rdx, 10
	mov [rsp+0], rdx
	mov rcx, [rsp+0]
	call ExitProcess
