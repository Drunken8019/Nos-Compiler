@ECHO off
start C:\Users\Adrian\AppData\Local\bin\NASM\nasm.exe -f win64 E:\Nos_IR\default.asm -o E:\Nos_IR\default.obj
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
link E:\Nos_IR\default.obj libcmt.lib kernel32.lib /subsystem:console /out:E:\Nos_IR\default.exe