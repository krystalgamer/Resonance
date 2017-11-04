%define numReplace [ebp+0x18]
%define indexOrder [ebp+0x14]
%define asmString [ebp+0x10]
%define asmBuffer [ebp+0xC]
%define valist [ebp+0x8]

segment .text
extern _sprintf
global _newFormatAsmString

_newFormatAsmString:
push ebp
mov ebp, esp


mov dword ecx, numReplace

pushArgs:
mov eax, valist
mov edx, indexOrder
mov edx, [edx];edx now contains the current index of the array

mov eax, [eax+edx*4];eax now has the current agument
push eax
sub dword indexOrder,4;go to next element in the array
loop pushArgs

push dword asmString
push dword asmBuffer
call _sprintf

mov esp, ebp
pop ebp
ret