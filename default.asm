global main
extern ExitProcess
section .bss
section .data
section .text
main:
sub rsp, 360
call fun
mov qword [rsp+8], rax
fun3:
sub rsp, 320
mov qword [rsp+8], 10
add rsp, 320
ret

mov qword rcx, [rsp+8]
call ExitProcess
add rsp, 320
ret

fun:
sub rsp, 320
mov qword [rsp+8], 100
mov qword [rsp], 10
call fun2
add qword [rsp], rax
mov qword rax, [rsp]
add rsp, 320
ret

fun2:
sub rsp, 320
mov qword [rsp], 30
mov qword rax, [rsp]
add rsp, 320
ret

