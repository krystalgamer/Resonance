#include "xor.h"

void addAsmInstruction(PASM_INSTRUCTION pAsmInstruction, BYTE opcode, BYTE size, BOOL needToFix, BOOL variableSize, BOOL sharedOpcode, BYTE regValue,void* fixSizeFunction){
	
	
	if(!pAsmInstruction)
		return;
		
	if(pAsmInstruction->next) //Not empty
		addAsmInstruction(pAsmInstruction->next, opcode, size, needToFix, variableSize, sharedOpcode,regValue,fixSizeFunction);
	else{
		//Initialize it
		pAsmInstruction->opcode = opcode;
		pAsmInstruction->size = size;
		pAsmInstruction->needToFix = needToFix;
		pAsmInstruction->variableSize = variableSize;
		
		pAsmInstruction->sharedOpcode = sharedOpcode;
		
		pAsmInstruction->regValue = regValue;
		pAsmInstruction->defineSize = fixSizeFunction;
		
		if(!(pAsmInstruction->next = malloc(sizeof(ASM_INSTRUCTION)))) //Couldn't allocate space for the next
			return; //RIP
	
		memset(pAsmInstruction->next, 0, sizeof(ASM_INSTRUCTION)); //clear to avoid problems
		
	}
}

/*
	BYTE opcode;
	BYTE size;
	
	BOOL needToFix;
	BOOL variableSize;
	BOOL sharedOpcode;
	
	BYTE regValue;//Only used if sharedOpcode is used
	void (*defineSize)(struct _asmInstruction *pThis);//Only used if variableSize is true
	*/

void addAllAsmInstructions(PASM_INSTRUCTION *ppAsmInstruction){
	
	DWORD tmp = 0;
	*ppAsmInstruction = malloc(sizeof(ASM_INSTRUCTION));
	
	if(*ppAsmInstruction == NULL)
		exit(1);
	
	memset(*ppAsmInstruction, 0, sizeof(ASM_INSTRUCTION));//Clear the crap
	
	#define INC_EAX 0x40
	//All instructions in the middle are 1 byte sized
	#define POPAD 0x61
	tmp = INC_EAX;
	
	while(tmp <= POPAD)
		addAsmInstruction(*ppAsmInstruction, tmp++, 1, FALSE, FALSE, FALSE,0,NULL);
	
	#define PUSH 0x68
	addAsmInstruction(*ppAsmInstruction, PUSH, 5, FALSE, FALSE, FALSE,0,NULL);
	//MOV ZONE
	#define MOV_01 0x88
	#define MOV_LAST 0x8B
	tmp = MOV_01;
	
	while(tmp <= MOV_LAST)
		addAsmInstruction(*ppAsmInstruction, tmp++, 0, FALSE, TRUE, FALSE,0,getMovSize);
	
	#define MOV_C7 0xC7
		addAsmInstruction(*ppAsmInstruction, MOV_C7, 0, FALSE, TRUE, FALSE,0,getMovSize);
	
	//SUB ZONE
	#define SUB_AL_IMM8 0x2C
	#define SUB_EAX_IMM32 (SUB_AL_IMM8 + 1)
	tmp = SUB_AL_IMM8;
	
	while(tmp <= SUB_EAX_IMM32)
		addAsmInstruction(*ppAsmInstruction, tmp++, (tmp==SUB_AL_IMM8) ? 2 : 5, FALSE, FALSE, FALSE,0,NULL);
	
	#define SUB_R8_IMM8 0x80
	#define SUB_R32_IMM32 (SUB_R8_IMM8 + 1)
	#define SUB_R32_IMM8 (SUB_R32_IMM32 + 2)
	tmp = SUB_R8_IMM8;
	
	while(tmp<=SUB_R32_IMM8){
		if(tmp == 0x82){//Fuck x86
			tmp++;
			continue;
		}
		addAsmInstruction(*ppAsmInstruction, tmp++, 0, FALSE, TRUE, TRUE,5,getXorSize);
	}
	
	
	//OR ZONE
	#define OR_01 0x09
	tmp = OR_01;
	
	while(tmp <= OR_01)
		addAsmInstruction(*ppAsmInstruction, tmp++, 0, FALSE, TRUE, FALSE,0,getOrSize);
	
	//LEAVE & RETN & NOP
	
	#define LEAVE 0xC9
	#define RETN 0xC3
	#define NOP 0x90
	
	addAsmInstruction(*ppAsmInstruction, LEAVE, 1, FALSE, FALSE, FALSE,0,NULL);
	addAsmInstruction(*ppAsmInstruction, RETN, 1, FALSE, FALSE, FALSE,0,NULL);
	addAsmInstruction(*ppAsmInstruction, NOP, 1, FALSE, FALSE, FALSE,0,NULL);
	
	
	//CALL AND JMP ZONE
	
	#define ABS_NEAR_CALL 0xFF
	#define ABS_NEAR_CALL_REG ((2<<3) | (DISPLACEMENT_ONLY))
	#define REL_CALL 0xE8
	
	addAsmInstruction(*ppAsmInstruction, REL_CALL, 5, TRUE, FALSE, FALSE,0,NULL);
	
	#define REL_JMP 0xE9
	#define REL_SHORT_JMP 0xEB
	#define ABS_NEAR_JUMP ABS_NEAR_CALL
	#define ABS_NEAR_JUMP_REG ((1<<3) | (DISPLACEMENT_ONLY))
	
	addAsmInstruction(*ppAsmInstruction, REL_JMP, 5, TRUE, FALSE, FALSE,0,NULL);
	addAsmInstruction(*ppAsmInstruction, REL_SHORT_JMP, 2, TRUE, FALSE, FALSE,0,NULL);
	
	//MOV TODO
	addAsmInstruction(*ppAsmInstruction, 0xB8, 5, FALSE, FALSE, FALSE,0,NULL);
	
	
	//TEST ZONE
	#define TEST_AL_IMM8 0xA8
	#define TEST_EAX_IMM32 0xA9
	
	addAsmInstruction(*ppAsmInstruction, TEST_AL_IMM8, 2, FALSE, FALSE, FALSE,0,NULL);
	addAsmInstruction(*ppAsmInstruction, TEST_EAX_IMM32, 5, FALSE, FALSE, FALSE,0,NULL);//they have different sizes :|
	
	#define TEST_R_IMM8 0xF6
	#define TEST_R_IMM32 0xF7
	tmp = TEST_R_IMM8;
	
	while(tmp<=TEST_R_IMM32)
		addAsmInstruction(*ppAsmInstruction, tmp++, 3, FALSE, TRUE, TRUE,0,getTestSize);
	
	#define TEST_R_R8 0x84
	#define TEST_R_R32 0x85
	tmp = TEST_R_R8;
	
	while(tmp<=TEST_R_R32)
		addAsmInstruction(*ppAsmInstruction, tmp++, 2, FALSE, TRUE, TRUE,0,getTestSize);
	
	//XOR ZONE
	#define XOR_AL 0x34
	#define XOR_EAX 0x35
	tmp = XOR_AL;
	
	while(tmp<=XOR_EAX)
		addAsmInstruction(*ppAsmInstruction, tmp++, (tmp == XOR_AL) ? 2 : 5, FALSE, TRUE, TRUE,0,NULL);
	
	#define XOR_R8_IMM8 SUB_R8_IMM8
	#define XOR_R32_IMM32 0x81
	#define XOR_R32_IMM8 0x83
	tmp = XOR_R8_IMM8;
	
	while(tmp<=XOR_R32_IMM8){
		if(tmp == 0x82){//Fuck x86
			tmp++;
			continue;
		}
		addAsmInstruction(*ppAsmInstruction, tmp++, 2, FALSE, TRUE, TRUE,6,getXorSize);
	}
	
	//ADD ZONE
	#define ADD_AL_IMM8 0x04
	#define ADD_AL_IMM32 0x05
	tmp = ADD_AL_IMM8;
	
	while(tmp<=ADD_AL_IMM32)
		addAsmInstruction(*ppAsmInstruction, tmp++, (tmp == ADD_AL_IMM8) ? 2 : 5, FALSE, FALSE, FALSE,0,NULL);

	#define ADD_R8_IMM8 XOR_R8_IMM8
	#define ADD_R32_IMM32 (ADD_R8_IMM8 + 1)
	#define ADD_R32_IMM8 (ADD_R32_IMM32 + 2)
	tmp = ADD_R8_IMM8;
	
	while(tmp<=ADD_R32_IMM8){
		if(tmp == 0x82){//Fuck x86
			tmp++;
			continue;
		}
		addAsmInstruction(*ppAsmInstruction, tmp++, 2, FALSE, TRUE, TRUE,0, getXorSize);//Exact same thing so :D
	}
	
}	
	

/*
	BYTE opcode;
	BYTE size;
	
	BOOL needToFix;
	BOOL variableSize;
	BOOL sharedOpcode;
	
	BYTE regValue;//Only used if sharedOpcode is used
	void (*defineSize)(struct _asmInstruction *pThis);//Only used if variableSize is true
	*/
	
void getXorSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer){
	
	BYTE tmp = 0;
	if(XOR_R8_IMM8 <= buffer[0] && XOR_R32_IMM8 >= buffer[0]){//FUCK 0x82
		
		if(buffer[0] == XOR_R8_IMM8 || buffer[0] == XOR_R32_IMM8){ //they share the same opcodes so meh
			tmp = (buffer[1] >> 6);
		
			switch(tmp){
				//easier way than 2 if statements
				case REGISTER_ADDRESSING:
					DEFAULT_SIZE(3)
					break;
				case ONE_BYTE_DISPLACEMENT:
					DEFAULT_SIZE(4)
					SIB_INSTRUCTION
					break;
				case FOUR_BYTE_DISPLACEMENT:
					DEFAULT_SIZE(7)
					SIB_INSTRUCTION
					break;
				case REGISTER_INDIRECT_ADDRESSING:
					DEFAULT_SIZE(3)
					SIB_AND_DISPLACEMENT_INSTRUCTION 
					break;
			}
		
		}
		else if(buffer[0] == XOR_R32_IMM32){
			tmp = (buffer[1] >> 6);
		
			switch(tmp){
				//easier way than 2 if statements
				case REGISTER_ADDRESSING:
					DEFAULT_SIZE(6)
					break;
				case ONE_BYTE_DISPLACEMENT:
					DEFAULT_SIZE(7)
					SIB_INSTRUCTION
					break;
				case FOUR_BYTE_DISPLACEMENT:
					DEFAULT_SIZE(10)
					SIB_INSTRUCTION
					break;
				case REGISTER_INDIRECT_ADDRESSING:
					DEFAULT_SIZE(6)
					SIB_AND_DISPLACEMENT_INSTRUCTION 
					break;
			}
			
		}
			
	}
}
	
void getTestSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer){
	
	BYTE tmp = 0;
	if(TEST_R_IMM8 <= buffer[0] && TEST_R_IMM32 >= buffer[0]){
		
		tmp = (buffer[1] >> 6);
		
		switch(tmp){
			//easier way than 2 if statements
			case REGISTER_ADDRESSING:
				DEFAULT_SIZE(3 + ((buffer[0] == TEST_R_IMM32) ? 3 : 0))
				break;
			case ONE_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(4 + ((buffer[0] == TEST_R_IMM32) ? 3 : 0))
				SIB_INSTRUCTION
				break;
				
			case FOUR_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(7 + ((buffer[0] == TEST_R_IMM32) ? 3 : 0))
				SIB_INSTRUCTION
				break;
			case REGISTER_INDIRECT_ADDRESSING:
				DEFAULT_SIZE(3 + ((buffer[0] == TEST_R_IMM32) ? 3 : 0))
				SIB_AND_DISPLACEMENT_INSTRUCTION 
				break;
		}
			
	}
	else if(TEST_R_R8 <= buffer[0] && TEST_R_R32 >= buffer[0]){
		
		tmp = (buffer[1] >> 6);
		
		switch(tmp){
			//easier way than 2 if statements
			case REGISTER_ADDRESSING:
				DEFAULT_SIZE(2)
				break;
			case ONE_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(3)
				SIB_INSTRUCTION
				break;
			case FOUR_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(6)
				SIB_INSTRUCTION
				break;
			case REGISTER_INDIRECT_ADDRESSING:
				DEFAULT_SIZE(2)
				SIB_AND_DISPLACEMENT_INSTRUCTION 
				break;
		}
		
		
	}
}

void getMovSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer){
	
	BYTE tmp = 0;
	if( MOV_01 <= buffer[0] && MOV_LAST >= buffer[0]){
		
		tmp = (buffer[1] >> 6);
		
		switch(tmp){
			
			case REGISTER_ADDRESSING:
				DEFAULT_SIZE(2)
				break;
			case ONE_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(3)
				break;
			case FOUR_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(6)
				break;
			case REGISTER_INDIRECT_ADDRESSING:
				DEFAULT_SIZE(2)
				
				SIB_AND_DISPLACEMENT_INSTRUCTION
					
				break;
		}
			
	}
	else if(buffer[0] == MOV_C7){
		
		tmp = ((buffer[1] >> 3) & 7); // get the bits in the the 6,5,4 position
		
		if(!tmp){ //C7 mov requires reg to be 0
			
			tmp = buffer[1] >> 6;
			
			switch(tmp){
			
			case REGISTER_ADDRESSING:
				DEFAULT_SIZE(6)
				break;
			case ONE_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(7)
	
				SIB_INSTRUCTION
				
				break;
			case FOUR_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(10)
				SIB_INSTRUCTION
					
				break;
			case REGISTER_INDIRECT_ADDRESSING:
				DEFAULT_SIZE(6)
				SIB_AND_DISPLACEMENT_INSTRUCTION
				break;
			
			}
			
		}
	}
}	


void getOrSize(PASM_INSTRUCTION pAsmInstruction, const unsigned char *buffer){
	
	BYTE tmp = 0;
	if( OR_01 <= buffer[0] && OR_01 >= buffer[0]){
		
	
			
		tmp = (buffer[1] >> 6);
		
		switch(tmp){
			
			case REGISTER_ADDRESSING:
				DEFAULT_SIZE(2)
				break;
			case ONE_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(3)
				SIB_INSTRUCTION
				break;
				
			case FOUR_BYTE_DISPLACEMENT:
				DEFAULT_SIZE(6)
				SIB_INSTRUCTION
				break;
			case REGISTER_INDIRECT_ADDRESSING:
				DEFAULT_SIZE(2)
				SIB_AND_DISPLACEMENT_INSTRUCTION 
				break;
		}
			
	}
}

void printAsmInstructions(PASM_INSTRUCTION pAsmInstruction){
	
	if(!pAsmInstruction)
		return;
	
	printf("%02X %08X\n", pAsmInstruction->opcode, pAsmInstruction->next);
	fflush(stdout);
	
	printAsmInstructions(pAsmInstruction->next);
}

BOOL interptFunction(HANDLE hProcess, FUNCTION function, PASM_INSTRUCTION pAsmInstructionList){
	
	unsigned char *buffer;
	SIZE_T bytesRead = 0;
	
	
	if(!(buffer = malloc(function.size)))
		return FALSE;
	
	if(!ReadProcessMemory(hProcess, (void*)function.startAddr, buffer, function.size, &bytesRead) && bytesRead != function.size){
		MessageBoxA(NULL, "ReadProcessMemory2 failed!", "FAILED", 0);
		return FALSE;
	}

	ASM_INSTRUCTION converted; //The raw function will be stored here
	memset(&converted, 0, sizeof(ASM_INSTRUCTION));
	
	
	if(!analyzeFunction(buffer, function.size,pAsmInstructionList, &converted))
		return FALSE;
	
	freeLinkedList((void*)&(pAsmInstructionList->next), sizeof(ASM_INSTRUCTION));//The first is not allocated with malloc
	
	if(!fixRelativeAddresses(&converted, function, buffer))
		return FALSE;
	
	
	ABS_ADDRESS_STORAGE absAddrStorage; //will be used to fix the absolute addresses
	memset(&absAddrStorage, 0, sizeof(ABS_ADDRESS_STORAGE));
	
	
	
	if(!addAbsoluteAddressesToStorage(&converted,&absAddrStorage)) //first add these babies
		return FALSE;
		
		
	DWORD convertFunctionSize = getConvertedFunctionSize(&converted);
	if(countStorageId(&absAddrStorage)){ //Only do this if there are any
		
		DWORD tmpCounter = 0;
		DWORD addressBuffer = 0;
		
		if(!(addressBuffer = fixAbsoluteAddresses(&absAddrStorage, &tmpCounter)))//then fix them
			return FALSE;
	
		HANDLE address = VirtualAllocEx(hProcess, NULL, sizeof(DWORD)*tmpCounter, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
		if(!address)
			return FALSE;
	
		if(!WriteProcessMemory(hProcess, (void*)address, (void*)addressBuffer,  tmpCounter*sizeof(DWORD), NULL))
			return FALSE;
		
		if(!(buffer = (convertFunction(&converted, buffer, convertFunctionSize, (DWORD)address))))//write addressBuffer to memmory
			return FALSE;
			
	}
	else{
		if(!(buffer = (convertFunction(&converted, buffer, convertFunctionSize, (DWORD)0))))//no address buffer :D
			return FALSE;
	}
	
	#define SHELL_SIZE 101
	unsigned char sexyShell[SHELL_SIZE] = {
		0x60, 0x9C, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x81, 0xF9, 0xFF, 0x00, 0x00, 0x00, 0x75, 0x45, 0xC7,
		0x05, 0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x69, 0x00, 0x00, 0x00, 0xB8, 0x68,
		0x00, 0x00, 0x00, 0xBF, 0x67, 0x00, 0x00, 0x00, 0x89, 0xDA, 0xD1, 0xE2, 0x01, 0xC2, 0x83, 0xC2,
		0x01, 0xBE, 0x78, 0x56, 0x34, 0x12, 0x39, 0xFA, 0x7E, 0x10, 0x83, 0xC7, 0x01, 0x89, 0x3D, 0x78,
		0x56, 0x34, 0x12, 0x89, 0xD9, 0xA4, 0xE2, 0xFD, 0xEB, 0x13, 0x89, 0xC7, 0x89, 0x3D, 0x78, 0x56,
		0x34, 0x12, 0xEB, 0xEF, 0x83, 0xC1, 0x01, 0x89, 0x0D, 0x78, 0x56, 0x34, 0x12, 0x9D, 0x61, 0xFF,
		0x25, 0x78, 0x56, 0x34, 0x12 
	};
	
	HANDLE address2 = VirtualAllocEx(hProcess, NULL, convertFunctionSize+sizeof(DWORD)+SHELL_SIZE+convertFunctionSize*3, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);//4 extra bytes for the address
	
	
	if(!address2)
		return FALSE;
	
	if(!WriteProcessMemory(hProcess, (void*)address2, (void*)buffer, convertFunctionSize, NULL))//Write the function to memory
		return FALSE;
	
	if(!WriteProcessMemory(hProcess, (void*)address2+convertFunctionSize+sizeof(DWORD)+SHELL_SIZE, (void*)buffer, convertFunctionSize, NULL))//Write the function to memory
		return FALSE;
	
	DWORD olaAmigos = (DWORD)address2+convertFunctionSize+sizeof(DWORD);
	if(!WriteProcessMemory(hProcess, (void*)address2+convertFunctionSize, &olaAmigos, sizeof(DWORD), NULL))//Write the address of the shellcode
		return FALSE;
		
	//Convert shell code to self modyfing code
	//setting the correct counter address
	DWORD counterAddress = (DWORD)address2+convertFunctionSize+sizeof(DWORD)+0x3;
	memcpy(sexyShell+0x11, &counterAddress,sizeof(DWORD));
	memcpy(sexyShell+0x59, &counterAddress,sizeof(DWORD));
	//setting the function size
	memcpy(sexyShell+0x1A, &convertFunctionSize,sizeof(DWORD));
	//setting the startAddress
	DWORD startAddress = (DWORD)address2+convertFunctionSize+sizeof(DWORD)+SHELL_SIZE;
	memcpy(sexyShell+0x1F, &startAddress,sizeof(DWORD));
	//setting addressOfUnencripted
	DWORD addressOfUnencripted = (DWORD)address2+convertFunctionSize+sizeof(DWORD)+0x24;
	memcpy(sexyShell+0x24, &startAddress,sizeof(DWORD));
	memcpy(sexyShell+0x3F, &addressOfUnencripted, sizeof(DWORD));
	memcpy(sexyShell+0x4E, &addressOfUnencripted, sizeof(DWORD));
	memcpy(sexyShell+0x61, &addressOfUnencripted, sizeof(DWORD));
	//functionStoreAddress
	memcpy(sexyShell+0x32, &address2,sizeof(DWORD));
	
	if(!WriteProcessMemory(hProcess, (void*)address2+convertFunctionSize+sizeof(DWORD), sexyShell, SHELL_SIZE, NULL))//Write the shellcode
		return FALSE;
	

	DWORD fdxTmp = (DWORD)address2+convertFunctionSize;
	
	unsigned char absJmp[6] = "\xFF\x25";
	memcpy(absJmp+2, &fdxTmp, sizeof(DWORD));//Copy the address that contains the address of the function
	
	
	if(!WriteProcessMemory(hProcess, (void*)function.startAddr, absJmp, 6, NULL))//Write the address of the function
		return FALSE;
	
	
	return TRUE;
	
	
}

BOOL analyzeFunction(const unsigned char *buffer, DWORD size,PASM_INSTRUCTION pAsmInstructionList, PASM_INSTRUCTION final){
	
	
	static DWORD counter = 0;
	static PASM_INSTRUCTION tmpAsm = NULL;

	
	if(!final)
		return FALSE;
	
	if(counter > size)
		return FALSE; //we fucked
	
	
	tmpAsm = NULL; //Clean the fucker
	tmpAsm = getCurrentInstruction(buffer+counter, pAsmInstructionList);

	if(!tmpAsm){
		char error[69];
		sprintf(error, "Unknown instruction wtf %02X", buffer[counter]);
		MessageBoxA(NULL, error, "fuck", 0);
		return FALSE;
	}
	
	memcpy(final, tmpAsm, sizeof(ASM_INSTRUCTION)); // we altered the original so we must have a copy of it
	counter += tmpAsm->size;
	final->next = NULL;
	
	if(counter == size)//the end
		return TRUE;
	
	//not the end... keep copying
	final->next = malloc(sizeof(ASM_INSTRUCTION));
	
	if(!(final->next)){
		MessageBoxA(NULL, "Failed to allocate space for the next instruction", "FAIL", 0);
		return FALSE;
	}
	
	memset(final->next, 0, sizeof(ASM_INSTRUCTION));
	return analyzeFunction(buffer, size, pAsmInstructionList, final->next);
}

PASM_INSTRUCTION getCurrentInstruction(const unsigned char *buffer, PASM_INSTRUCTION pAsmInstructionList){
	
	if(!pAsmInstructionList)
		return NULL;
	
	if(buffer[0] == pAsmInstructionList->opcode){
		
		if(pAsmInstructionList->variableSize || pAsmInstructionList->sharedOpcode){
			
			if(pAsmInstructionList->variableSize) //fix the size
				pAsmInstructionList->defineSize(pAsmInstructionList, buffer);
			
			if(pAsmInstructionList->sharedOpcode){ //make sure it's the correct instruction
			
				if(((buffer[1] >> 3) & 7) == pAsmInstructionList->regValue)//key point
					return pAsmInstructionList;
			
				return getCurrentInstruction(buffer, pAsmInstructionList->next);
			}
		
		}
		
		return pAsmInstructionList;
	}
	
		return getCurrentInstruction(buffer, pAsmInstructionList->next);
	
}

BOOL fixRelativeAddresses(PASM_INSTRUCTION pAsmInstruction, FUNCTION function,const unsigned char* buffer){
	
	static PABS_INSTRUCTION tmpAbs = NULL;
	static DWORD counter = 0;
	
	tmpAbs = NULL;
	
	if(!pAsmInstruction && function.size == counter)
		return TRUE;
	
	if(pAsmInstruction->needToFix){
		
		if(pAsmInstruction->opcode == REL_CALL ||
		pAsmInstruction->opcode == REL_JMP || pAsmInstruction->opcode == REL_SHORT_JMP){
			
			
			DWORD* jmpLocation = (DWORD*)(buffer+counter+1);
			if((function.size - counter) >= ((pAsmInstruction->size > 2) ? *jmpLocation : buffer[counter+1])){//only jump if the jmp is to outside the function
				pAsmInstruction->needToFix = FALSE;//No need to fix then
				counter += pAsmInstruction->size;
				return fixRelativeAddresses(pAsmInstruction->next, function,buffer);
			}
				
			
			tmpAbs = malloc(sizeof(ABS_INSTRUCTION));
		
			if(!tmpAbs){
				MessageBoxA(NULL, "Couldnt allocate space for abs instruction", "nop", 0);
				return FALSE;
			}
			tmpAbs->opcode = ABS_NEAR_JUMP;
			
			tmpAbs->modReg = 0; //Clear it
			
			tmpAbs->modReg |= (pAsmInstruction->opcode == REL_CALL) ?  ABS_NEAR_CALL_REG : ABS_NEAR_JUMP_REG;
			
			DWORD* tmpDword = NULL;
			tmpDword = (DWORD*)(buffer+counter+1); //Only works this way??
			
			tmpAbs->address = *tmpDword + function.startAddr + counter + 5;
			
			pAsmInstruction->fixedInstruction = tmpAbs;
			
			pAsmInstruction->size2 = 6;
			
		}
	}
	
	counter += pAsmInstruction->size;
	
	return fixRelativeAddresses(pAsmInstruction->next, function,buffer);
}

DWORD getConvertedFunctionSize(PASM_INSTRUCTION pAsmInstruction){
	
	if(!pAsmInstruction)
		return 0;
	
	return (pAsmInstruction->fixedInstruction ? pAsmInstruction->size2 : pAsmInstruction->size) + getConvertedFunctionSize(pAsmInstruction->next);
}


unsigned char *convertFunction(PASM_INSTRUCTION pAsmInstruction,const unsigned char* buffer,DWORD size, DWORD allocatedMemoryAddress){
	
	static DWORD fixedCounter = 0; //Used for the new function
	static DWORD bufferCounter = 0;//They move at different speeds
	
	static unsigned char *fixedFunction = NULL;
	
	if(size == fixedCounter)
		return fixedFunction;
	else if(size < fixedCounter) //we fucked up
		return NULL;
	
	if(!fixedFunction){
		fixedFunction = malloc(size);
		
		if(!fixedFunction)
			return NULL;
	}
	
	
	//Choose which to copy
	if(pAsmInstruction->fixedInstruction){
		
				pAsmInstruction->fixedInstruction->address = sizeof(DWORD) * pAsmInstruction->fixedInstruction->idInStorage + allocatedMemoryAddress; // set the correct address
				memcpy((void*)fixedFunction+fixedCounter, pAsmInstruction->fixedInstruction, pAsmInstruction->size2);
				
				fixedCounter += pAsmInstruction->size2;
	}
	else{
		
		memcpy((void*)fixedFunction+fixedCounter, buffer+bufferCounter, pAsmInstruction->size);
		fixedCounter += pAsmInstruction->size;
	}
	
	bufferCounter += pAsmInstruction->size; //Need to move as the original
	
	return convertFunction(pAsmInstruction->next, buffer, size, allocatedMemoryAddress);
	
	
}

BOOL addAbsoluteAddressesToStorage(PASM_INSTRUCTION pAsmInstruction, PABS_ADDRESS_STORAGE absAddrStorage){
	
	
	if(!pAsmInstruction)//cant be sure enoguh
		return TRUE;
	
	
	if(pAsmInstruction->fixedInstruction){
		
		if(!absAddrStorage)//Wow
			return FALSE;
		
		absAddrStorage->instruction = pAsmInstruction->fixedInstruction;
		absAddrStorage->next = NULL;
		
		if(!pAsmInstruction->next) //no need to reserve more space in memory if it's the end
			return TRUE;
		
		absAddrStorage->next = malloc(sizeof(ABS_ADDRESS_STORAGE));
		
		if(!absAddrStorage->next)
			return FALSE;
		
		memset(absAddrStorage->next, 0, sizeof(ABS_ADDRESS_STORAGE));
		
		return addAbsoluteAddressesToStorage(pAsmInstruction->next, absAddrStorage->next);
			
	}
	
	return addAbsoluteAddressesToStorage(pAsmInstruction->next, absAddrStorage);//nothing was added to absAddrStorage
}

//Get how many addresses it should store it doesn't ignore the repeated
BOOL countStorageId(PABS_ADDRESS_STORAGE absAddrStorage){
	
	static DWORD counter = 0;
	
	if(!absAddrStorage || !absAddrStorage->instruction)//the end m8
		return counter;
		
	counter++;
	return countStorageId(absAddrStorage->next);
	
}


BOOL fixAbsoluteAddresses(PABS_ADDRESS_STORAGE absAddrStorage, DWORD* counter){
	
	DWORD storageIdCount = countStorageId(absAddrStorage);
	
	if(!storageIdCount)//No need to fix anything
		return TRUE;
		
	DWORD *addressBuffer = malloc(sizeof(DWORD) * storageIdCount);
	
	if(!addressBuffer)//fail
		return FALSE;
		
	
	PABS_ADDRESS_STORAGE tmpStorage = NULL, tmpStorage2 = NULL;
	
	//Tired of recursion
	tmpStorage = absAddrStorage;
	
	while(tmpStorage){
		
		
		if(tmpStorage->alreadyInStorage){ //No need to add again
			tmpStorage = tmpStorage->next;
			continue;
		}
		
		if(tmpStorage->instruction){
			
		
			addressBuffer[*counter] = tmpStorage->instruction->address;//put it in the buffer
			tmpStorage->instruction->idInStorage = *counter;
			tmpStorage->alreadyInStorage = TRUE;
		
			tmpStorage2 = tmpStorage->next;
		
			while(tmpStorage2){ //same logic
			
				if(tmpStorage2->instruction){
					if(!tmpStorage2->alreadyInStorage && addressBuffer[*counter] == tmpStorage2->instruction->address){
						tmpStorage2->alreadyInStorage = TRUE;
						tmpStorage2->instruction->idInStorage = *counter;
					}
				}
		
				tmpStorage2 = tmpStorage2->next;
			
			}
		}
		
		
		tmpStorage = tmpStorage->next;
		(*counter)++;
	}
	
	return (BOOL)addressBuffer;
}