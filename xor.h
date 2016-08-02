#include <windows.h>
#include <stdio.h>
#include <Psapi.h>
#include <tlhelp32.h>

typedef struct _function{
	
	DWORD startAddr;
	DWORD size;
	struct _function *next;
	
}FUNCTION, *PFUNCTION;

typedef struct __attribute__((packed)){

	BYTE opcode;
	BYTE modReg;
	union{
		DWORD address;
		DWORD idInStorage; // diffrent name so my head doesn't explode
	};

}ABS_INSTRUCTION, *PABS_INSTRUCTION;

typedef struct __attribute__((packed)) _asmInstruction{

	BYTE opcode;
	BYTE size;
	
	union{
		BOOL needToFix;
		PABS_INSTRUCTION fixedInstruction;
	};
	
	union{
		BOOL variableSize;
		BYTE size2; //used to store the "new" size of the needToFix fixed instruction
	};
	
	BOOL sharedOpcode;
	BYTE regValue;//Only used if sharedOpcode is used
	void (*defineSize)(struct _asmInstruction *pThis, const unsigned char *buffer);//Only used if variableSize is true
	struct _asmInstruction *next;

}ASM_INSTRUCTION, *PASM_INSTRUCTION;

typedef struct _absAddressStorage{

	PABS_INSTRUCTION instruction; //easier to edit it
	BOOLEAN alreadyInStorage;
	struct _absAddressStorage *next;
	

}ABS_ADDRESS_STORAGE, *PABS_ADDRESS_STORAGE;

BOOL getModuleHandle(DWORD processId, const char* name, DWORD* baseAddr, DWORD* moduleSize);

void freeLinkedList(void **entry, DWORD sizeOfEntry);

void addAsmInstruction(PASM_INSTRUCTION pAsmInstruction, BYTE opcode, BYTE size, BOOL needToFix, BOOL variableSize, BOOL sharedOpcode, BYTE regValue,void* fixSizeFunction);
void printAsmInstructions(PASM_INSTRUCTION pAsmInstruction);
void addAllAsmInstructions(PASM_INSTRUCTION *ppAsmInstruction);
void addAsmInstruction(PASM_INSTRUCTION pAsmInstruction, BYTE opcode, BYTE size, BOOL needToFix, BOOL variableSize, BOOL sharedOpcode, BYTE regValue,void* fixSizeFunction);


void getMovSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer);
void getOrSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer);
void getTestSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer);
void getXorSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer);

PASM_INSTRUCTION getCurrentInstruction(const unsigned char *buffer, PASM_INSTRUCTION pAsmInstructionList);
BOOL analyzeFunction(const unsigned char *buffer, DWORD size,PASM_INSTRUCTION pAsmInstructionList, PASM_INSTRUCTION final);
BOOL interptFunction(HANDLE hProcess, FUNCTION function, PASM_INSTRUCTION pAsmInstructionList);
BOOL fixRelativeAddresses(PASM_INSTRUCTION pAsmInstruction, FUNCTION function,const unsigned char* buffer);
BOOL addAbsoluteAddressesToStorage(PASM_INSTRUCTION pAsmInstruction, PABS_ADDRESS_STORAGE absAddrStorage);

unsigned char *convertFunction(PASM_INSTRUCTION pAsmInstruction,const unsigned char* buffer,DWORD size, DWORD allocatedMemoryAddress);
DWORD getConvertedFunctionSize(PASM_INSTRUCTION pAsmInstruction);

BOOL addAbsoluteAddressesToStorage(PASM_INSTRUCTION pAsmInstruction, PABS_ADDRESS_STORAGE absAddrStorage);
BOOL countStorageId(PABS_ADDRESS_STORAGE absAddrStorage);
BOOL fixAbsoluteAddresses(PABS_ADDRESS_STORAGE absAddrStorage, DWORD* counter);
		
//Only first 2 bits matter
#define REGISTER_ADDRESSING 3 	//11
#define FOUR_BYTE_DISPLACEMENT 2 //10
#define ONE_BYTE_DISPLACEMENT 1 	//01
#define REGISTER_INDIRECT_ADDRESSING (0)//00

//REGISTER_INDIRECT_ADDRESSING has more modes
#define SIB_NO_DISPLACEMENT (1<<2) //100
#define DISPLACEMENT_ONLY (5 << 0) //101


#define DEFAULT_SIZE(x) \
   pAsmInstruction->size = x; \
   tmp = buffer[1] & 7;
   
#define SIB_INSTRUCTION \
	if(tmp == SIB_NO_DISPLACEMENT)\
					pAsmInstruction->size += 1; 
				
#define DISPLACEMENT_ONLY_INSTRUCTION \
	if(tmp == DISPLACEMENT_ONLY)\
					pAsmInstruction->size += 4;
					
#define SIB_AND_DISPLACEMENT_INSTRUCTION \
	SIB_INSTRUCTION \
	DISPLACEMENT_ONLY_INSTRUCTION
  