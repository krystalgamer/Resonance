# Resonance
A C polymorphic and metamorphic engine

This engine aims to convert non-polymorphic code into polymorphic by injecting some shellcode into the target process.
The target functions need to have the respective identifiers.
``` 
or eax, eax
or eax, eax
or eax, eax


CODE GOES HERE

or ecx, ecx
or ecx, ecx
or ecx, ecx
``` 

What has been done so far?
- Detects the target function and successfully dissassembles it.
- Fixes relative JMPs and CALLs
- Encrypts it to a new memory location
- Decrypts it at runtime and moves it around in memory

If you'd like to see it in action check my channel: https://www.youtube.com/user/namsZINGER

## How to use?
Currently it's usage is limited since it's in early development but if you really want to try do the following.
- Compile it
- Add the identifiers to one function in your program and compile it too.
-  Make sure that the program is named test.exe
-  Run Resonance!
