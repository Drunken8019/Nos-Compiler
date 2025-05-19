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
mov qword [rsp+16], 100
call fun
mov qword rdx, [rsp+16]
add qword rdx, 10
mov qword [rsp+16], rdx
mov qword rdx, [rsp+0]
add qword rdx, [rsp+16]
mov qword rcx, rdx
call ExitProcess
ret

fun:
ret

