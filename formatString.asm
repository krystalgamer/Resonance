%define numReplace [ebp+0x14]
%define asmString [ebp+0x10]
%define asmBuffer [ebp+0xC]
%define valist [ebp+0x8]

segment .text
extern _sprintf
global _formatAsmString

_formatAsmString:
push ebp
mov ebp, esp


;pushes everything and calls sprintf
mov ecx, numReplace

sub dword numReplace, 1;already starts on the first
shl dword numReplace, 2
xor eax, eax
add eax, numReplace
add eax, valist
mov valist, eax ;start from top to bottom

pushArgs:
mov eax, valist
mov eax, [eax]
sub dword valist, 4
push eax
loop pushArgs

push dword asmString
push dword asmBuffer
call _sprintf

mov esp, ebp
pop ebp
ret