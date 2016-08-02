segment .data
ola db "OLA",0

segment .text
global _useless
extern _printf

_useless:
push ebp
mov ebp, esp

or eax, eax
or eax, eax
or eax, eax

push ola
jmp derp
add dword [909090h], 2h

derp:
call _printf

or ecx, ecx
or ecx, ecx
or ecx, ecx

leave
ret
