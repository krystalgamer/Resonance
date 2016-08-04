#include <windows.h>
#include <stdio.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <inttypes.h>

typedef struct _function{
	
	DWORD startAddr;
	DWORD size;
	struct _function *next;
	
}FUNCTION, *PFUNCTION;

typedef struct __attribute__((packed)) _abs_Instruction{

	BYTE opcode;
	BYTE modReg;
	union{
		DWORD address;
		DWORD idInStorage; // diffrent name so my head doesn't explode
	};
	DWORD bytePosition; //Where in the buffer it should be replaced
	DWORD originalSize;
	BOOL alreadyStored;
	struct _abs_Instruction *next;

}ABS_INSTRUCTION, *PABS_INSTRUCTION;

BOOL getModuleHandle(DWORD processId, const char* name, DWORD* baseAddr, DWORD* moduleSize);
void freeLinkedList(void **entry, DWORD sizeOfEntry);

BOOL interptFunction(HANDLE hProcess, FUNCTION function);
BOOL fixRelativeJmpOrCall(int64_t jmpLocation, DWORD numBytes, PABS_INSTRUCTION *pAbsInstruction, DWORD originalSize,BOOLEAN isJmp);
BYTE* createNewFunction(const BYTE *originalFunction, DWORD newFunctionSize, PABS_INSTRUCTION pAbsInstruction);
BOOL getAbsoluteAddressStorage(HANDLE hProcess, PABS_INSTRUCTION pAbsInstruction);