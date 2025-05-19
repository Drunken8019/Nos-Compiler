global main
extern ExitProcess
section .bss
section .data
section .text
main:
sub rsp, 360
mov qword [rsp+0], 10
mov qword rdx, [rsp+0]
add qword rdx, 10
mov qword [rsp+0], rdx
mov qword rcx, [rsp+0]
call ExitProcess
