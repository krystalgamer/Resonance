#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <inttypes.h>
#include <stdbool.h>
#include "include/capstone/capstone.h"
#include "include/keystone/keystone.h"



#ifndef XOR_H
#define XOR_H

#define REG_NAME(x) cs_reg_name(csHandle, x)

typedef struct _function{
	
	DWORD startAddr;
	DWORD size;
	struct _function *next;
	
}FUNCTION, *PFUNCTION;

//holds the info of jumps/calls that go to oustide the function
typedef struct _outsideinstruction{
	
	uint32_t id;//id of instruction
	
	uint32_t type;//jump or call opcode
	uint32_t destinationAddress;//where the call/jmp lands
	
	//this will hold the fixed instruction
	struct{
		size_t fixedOISize;
		uint8_t *fixedOI;
	};
	
	struct _outsideinstruction *next;
}OutsideInstruction;

//holds the modified instruction info
typedef struct _modInsn{
	
	uint32_t id;//cs_insn id
	
	struct {
		uint8_t *moddedInsn;
		uint32_t moddedSize;
	};
	
	struct _modInsn *next;
}ModifiedInstruction;


typedef struct _jmpInstruction{
	
	uint32_t id;
	
	uint8_t *jmpInsn;
	uint32_t jmpSize;
	
	struct _jmpInstruction *next;
	
}JmpInstruction;

typedef struct _trashInstruction{
	
	uint32_t id;//trash is added after the id
	
	uint8_t *trashInsn;
	uint32_t trashSize;
	
	struct _trashInstruction *next;
	
}TrashInstruction;

typedef enum{
	
	POLY_OK,
	POLY_FAIL,
	POLY_UNIMPLEMENTED
}PolyFunc;

uint8_t *createNewFunction(cs_insn *insn, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList, JmpInstruction *JIList, uint32_t numInstructions,uint32_t *sizeOfFixedFunction);
uint32_t getNewFunctionSize(cs_insn *insn, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList, JmpInstruction *JIList,uint32_t numInstructions);
bool generateAsm(uint8_t **encoding, size_t *encodingSize, const uint8_t *asmString, ...);
bool generateAsm2(uint8_t **encoding, size_t *encodingSize, const uint8_t *asmString, ...);

//outsideInstruction.c
OutsideInstruction *createOustideInstruction();
bool fixOutisdeInstructionList(OutsideInstruction *list);
bool addToOutisdeInstructionList(OutsideInstruction *list, cs_insn *insn, uint32_t insnId);

//modifiedInstruction.c
PolyFunc movPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn);
PolyFunc pushPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn);
PolyFunc orPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn);
PolyFunc retPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn);

//trashInstruction.c
TrashInstruction *createTrashInstruction();
bool addToTrashInstructionList(TrashInstruction *list, uint32_t insnId);
bool generateTrashInstruction(TrashInstruction *curTI);

//jmpInstruction.c
JmpInstruction *createJmpInstruction();
bool addToJmpInstructionList(JmpInstruction *list, uint32_t insnId);
uint32_t getOriginalJmpResult(cs_insn *insn);
uint32_t getNewJmpOffset(uint32_t jmpId, uint32_t destIdOff, cs_insn *insn, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList);
bool fixAffectedRelativeJmps(cs_insn *insn, JmpInstruction *JIList, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList);

bool addToModifiedInstructionList(ModifiedInstruction *list, cs_insn *insn, uint32_t insnId);
ModifiedInstruction *createModifiedInstruction();

void removeLastEntry(void **entry, uint32_t sizeOfEntry);


BOOL getModuleHandle(DWORD processId, const char* name, DWORD* baseAddr, DWORD* moduleSize);
void freeLinkedList(void **entry, DWORD sizeOfEntry);

bool interptFunction(HANDLE hProcess, FUNCTION function);

#endif