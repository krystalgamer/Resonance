%define counter ecx
%define functionSize ebx
%define startAddress eax
%define addressOfUnencripted edi
%define tmp edx
%define functionStoreAddress esi ;where the original function is stored

segment .text

justDoIt:;couldnt think of anything
pushad
pushfd


mov counter, 0h ; this value will change
cmp counter, 0x000000FF
jne increaseCounter

;incase that they're equal
mov dword [12345678h], 0 ;restore the counter

mov dword functionSize, 69h ;wont change
mov dword startAddress, 68h ;same
mov dword addressOfUnencripted, 67h ;this will change

mov tmp, functionSize
shl tmp, 1 ;double it
add tmp, startAddress
add tmp, 1

mov functionStoreAddress, 12345678h

cmp tmp, addressOfUnencripted
jle resetAddressOfUnencripted


mov byte [addressOfUnencripted], 0x90;nop old shit

;time to move it
add addressOfUnencripted, 1 ;increase it
mov [12345678h], addressOfUnencripted ; update it

copyFunction:
mov ecx, functionSize

copy:
movsb
xor byte [addressOfUnencripted-1], 0xf2
loop copy

jmp endJustDoIt

resetAddressOfUnencripted:
mov ecx, functionSize

nopEndOfMemory:;this is needed because when it goes back to start there are leftovers
mov byte [addressOfUnencripted+ecx-1], 0x90
loop nopEndOfMemory

mov addressOfUnencripted, startAddress
mov [12345678h], addressOfUnencripted ;address where addressOfUnencripted is stored
jmp copyFunction

increaseCounter:
add counter, 1h
mov [12345678h], counter

endJustDoIt:
popfd
popad
jmp [12345678h]