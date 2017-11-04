
segment .text
setEip:
push dword [esp]
add [esp+4], 7
ret