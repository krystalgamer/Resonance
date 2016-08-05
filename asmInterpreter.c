#include "xor.h"
#include "include/capstone.h"


BOOL interptFunction(HANDLE hProcess, FUNCTION function){
	
	unsigned char *buffer;
	SIZE_T bytesRead = 0;
	
	if(!(buffer = malloc(function.size)))
		return FALSE;
	
	if(!ReadProcessMemory(hProcess, (void*)function.startAddr, buffer, function.size, &bytesRead) && bytesRead != function.size){
		MessageBoxA(NULL, "ReadProcessMemory2 failed!", "FAILED", 0);
		return FALSE;
	}

	csh handle;
	cs_insn *insn;
	size_t count;
 
	if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK)
      return -1;
	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
	
	
	count = cs_disasm(handle, buffer, function.size, function.startAddr, 0, &insn);
	
	
	PABS_INSTRUCTION pAbsInstruction = NULL;
	size_t byteCounter = 0;
	
	if (count > 0) {
		size_t currIns = 0;
		cs_x86 *x86_insn = NULL;
		
		for (currIns = 0; currIns < count; currIns++) {
			
			if(!strcmp(insn[currIns].mnemonic, "jmp") || !strcmp(insn[currIns].mnemonic, "call")){
				
				x86_insn = &(insn[currIns].detail->x86);
				
				if(x86_insn->op_count != 1)//Something went wrong
					return FALSE;
				
				cs_x86_op *operand = &(x86_insn->operands[0]);
				
				if(operand->type != X86_OP_IMM)
					continue;
				
				if(operand->imm > function.startAddr + function.size){
					
					BOOL fixRelativeJmpOrCallResult = 0;
					
					if(!(fixRelativeJmpOrCallResult = fixRelativeJmpOrCall(operand->imm, byteCounter, &pAbsInstruction, insn[currIns].size,strcmp(insn[currIns].mnemonic, "call"))))
						return FALSE;
					
					printf("Found one relative jmp/call to fix!\n");
					byteCounter += fixRelativeJmpOrCallResult;
				}
				else
					byteCounter += insn[currIns].size;
			}
			else
				byteCounter += insn[currIns].size;
		}
 
		cs_free(insn, count);
	}
	else
      printf("ERROR: Failed to disassemble given code!\n");
 
	cs_close(&handle);
	fflush(stdout);
	
	if(!getAbsoluteAddressStorage(hProcess, pAbsInstruction))
		return FALSE;
	
	BYTE* newFunction = createNewFunction(buffer, byteCounter,  pAbsInstruction);
	
	if(!newFunction)
		return FALSE;
	
	BYTE* encryptedFunction = malloc(byteCounter);
	if(!encryptedFunction)
		return FALSE;
	
	memcpy(encryptedFunction, newFunction, byteCounter);
	
	int encryptLocation = 0;
	while(encryptLocation<byteCounter){
		encryptedFunction[encryptLocation++] ^= 0xF2;
	}
	
	#define SHELL_SIZE 117
	unsigned char sexyShell[SHELL_SIZE] = {
		0x60, 0x9C, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x81, 0xF9, 0xFF, 0x00, 0x00, 0x00, 0x75, 0x55, 0xC7,
		0x05, 0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x69, 0x00, 0x00, 0x00, 0xB8, 0x68,
		0x00, 0x00, 0x00, 0xBF, 0x67, 0x00, 0x00, 0x00, 0x89, 0xDA, 0xD1, 0xE2, 0x01, 0xC2, 0x83, 0xC2,
		0x01, 0xBE, 0x78, 0x56, 0x34, 0x12, 0x39, 0xFA, 0x7E, 0x17, 0xC6, 0x07, 0x90, 0x83, 0xC7, 0x01,
		0x89, 0x3D, 0x78, 0x56, 0x34, 0x12, 0x89, 0xD9, 0xA4, 0x80, 0x77, 0xFF, 0xF2, 0xE2, 0xF9, 0xEB,
		0x1C, 0x89, 0xD9, 0xC6, 0x44, 0x0F, 0xFF, 0x90, 0xE2, 0xF9, 0x89, 0xC7, 0x89, 0x3D, 0x78, 0x56,
		0x34, 0x12, 0xEB, 0xE2, 0x83, 0xC1, 0x01, 0x89, 0x0D, 0x78, 0x56, 0x34, 0x12, 0x9D, 0x61, 0xFF,
		0x25, 0x78, 0x56, 0x34, 0x12 
	};




	HANDLE address2 = VirtualAllocEx(hProcess, NULL, byteCounter+sizeof(DWORD)+SHELL_SIZE+byteCounter*3, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);//4 extra bytes for the address
	
	
	if(!address2)
		return FALSE;
	
	if(!WriteProcessMemory(hProcess, (void*)address2, (void*)encryptedFunction, byteCounter, NULL))//Write the encrypted function to memory
		return FALSE;
	
	if(!WriteProcessMemory(hProcess, (void*)address2+byteCounter+sizeof(DWORD)+SHELL_SIZE, (void*)newFunction, byteCounter, NULL))//Write the function to memory
		return FALSE;
	
	DWORD olaAmigos = (DWORD)address2+byteCounter+sizeof(DWORD);
	if(!WriteProcessMemory(hProcess, (void*)address2+byteCounter, &olaAmigos, sizeof(DWORD), NULL))//Write the address of the shellcode
		return FALSE;
		
	//Convert shell code to self modyfing code
	//setting the correct counter address
	DWORD counterAddress = (DWORD)address2+byteCounter+sizeof(DWORD)+0x3;
	memcpy(sexyShell+0x11, &counterAddress,sizeof(DWORD));
	memcpy(sexyShell+0x69, &counterAddress,sizeof(DWORD));
	//setting the function size
	memcpy(sexyShell+0x1A, &byteCounter,sizeof(DWORD));
	//setting the startAddress
	DWORD startAddress = (DWORD)address2+byteCounter+sizeof(DWORD)+SHELL_SIZE;
	memcpy(sexyShell+0x1F, &startAddress,sizeof(DWORD));
	//setting addressOfUnencripted
	DWORD addressOfUnencripted = (DWORD)address2+byteCounter+sizeof(DWORD)+0x24;
	memcpy(sexyShell+0x24, &startAddress,sizeof(DWORD));
	memcpy(sexyShell+0x42, &addressOfUnencripted, sizeof(DWORD));
	memcpy(sexyShell+0x5E, &addressOfUnencripted, sizeof(DWORD));
	memcpy(sexyShell+0x71, &addressOfUnencripted, sizeof(DWORD));
	//functionStoreAddress
	memcpy(sexyShell+0x32, &address2,sizeof(DWORD));
	
	if(!WriteProcessMemory(hProcess, (void*)address2+byteCounter+sizeof(DWORD), sexyShell, SHELL_SIZE, NULL))//Write the shellcode
		return FALSE;
	

	DWORD fdxTmp = (DWORD)address2+byteCounter;
	
	unsigned char absJmp[6] = "\xFF\x25";
	memcpy(absJmp+2, &fdxTmp, sizeof(DWORD));//Copy the address that contains the address of the function
	
	
	if(!WriteProcessMemory(hProcess, (void*)function.startAddr, absJmp, 6, NULL))//Write the address of the function
		return FALSE;
	
	
	return TRUE;
	
}

BOOL fixRelativeJmpOrCall(int64_t jmpLocation, DWORD numBytes, PABS_INSTRUCTION *pAbsInstruction, DWORD originalSize, BOOLEAN isJmp){
	
	
	if(!(*pAbsInstruction))
		*pAbsInstruction = malloc(sizeof(ABS_INSTRUCTION));
	else
		return fixRelativeJmpOrCall(jmpLocation, numBytes, &((*pAbsInstruction)->next), originalSize,isJmp);
	
	if(!(*pAbsInstruction))//It failed
		return FALSE;
	
	memset((*pAbsInstruction), 0, sizeof(ABS_INSTRUCTION));
	
	(*pAbsInstruction)->type = ABS_CALL; //DEFAULT
	(*pAbsInstruction)->bytePosition = numBytes;
	(*pAbsInstruction)->originalSize = originalSize;
	(*pAbsInstruction)->next = NULL;
	
	if(!isJmp)
		(*pAbsInstruction)->type = (rand()%2 == 0) ? PUSH_RETN : ABS_CALL;
	
	
	//Name doesn't matter
	(*pAbsInstruction)->pPushRetn = malloc( ((*pAbsInstruction)->type == PUSH_RETN) ? sizeof(PUSH_RETN_STRUCT) : sizeof(ABS_CALL_STRUCT));
	if(!((*pAbsInstruction)->pPushRetn))
		return FALSE;
	
	(*pAbsInstruction)->newSize = ((*pAbsInstruction)->type == PUSH_RETN) ? sizeof(PUSH_RETN_STRUCT) : sizeof(ABS_CALL_STRUCT) - 4;
	
	if(((*pAbsInstruction)->type == PUSH_RETN)){
		(*pAbsInstruction)->pPushRetn->callOpcode = 0xE8;
		(*pAbsInstruction)->pPushRetn->emptyAddress = 0x00000000;
	
		(*pAbsInstruction)->pPushRetn->prepareJmp[0] = 0x83;
		(*pAbsInstruction)->pPushRetn->prepareJmp[1] = 0x04;
		(*pAbsInstruction)->pPushRetn->prepareJmp[2] = 0x24;
		(*pAbsInstruction)->pPushRetn->prepareJmp[3] = 10;
	
		(*pAbsInstruction)->pPushRetn->pushOpcode = 0x68;
		(*pAbsInstruction)->pPushRetn->pushAddress= jmpLocation;
		(*pAbsInstruction)->pPushRetn->retnOpcode= 0xC3;
	
		return (*pAbsInstruction)->newSize;
	}
	
	(*pAbsInstruction)->pAbsCall->opcode = 0xFF;
	(*pAbsInstruction)->pAbsCall->modReg = (((isJmp ? 4 : 2) << 3) | 5);
	(*pAbsInstruction)->pAbsCall->address = jmpLocation;
	(*pAbsInstruction)->pAbsCall->alreadyStored = FALSE;

	return (*pAbsInstruction)->newSize;
	
}

BOOL getAbsoluteAddressStorage(HANDLE hProcess, PABS_INSTRUCTION pAbsInstruction){
	
	DWORD counter = 0;
	PABS_INSTRUCTION tmp = pAbsInstruction;
	PABS_INSTRUCTION tmp2 = NULL;
	
	//Get how many addresses we'll need(doesnt exclude repeated ones)
	while(counter){
		tmp = tmp->next;
		counter++;
	}
	
	DWORD *addressBuffer = malloc(counter * sizeof(DWORD));
	if(!addressBuffer)
		return FALSE;
	
	//Give each abs instruction a unique id
	DWORD curId = 0;
	tmp = pAbsInstruction;
	while(tmp){
		
		if(curId > counter)//We fucked
			return FALSE;
		
		if(tmp->type != ABS_CALL){
			tmp = tmp->next;
			continue; //Dont increase the id because i'll get us problems
		}
		
		if(tmp->pAbsCall->alreadyStored){
			tmp = tmp->next;
			continue; 
		}
		
		//Copy to the address buffer
		addressBuffer[curId] = tmp->pAbsCall->address;
		tmp->pAbsCall->idInStorage = curId;
		tmp->pAbsCall->alreadyStored = TRUE;
		
		tmp2 = tmp->next;
		while(tmp2){
			
			if(tmp2->type != ABS_CALL){
				tmp2 = tmp2->next;
				continue;
			}
			if(tmp2->pAbsCall->alreadyStored){
				tmp2 = tmp2->next;
				continue;
			}
			
			if(tmp2->pAbsCall->address == addressBuffer[curId]){
				tmp2->pAbsCall->idInStorage = curId;
				tmp2->pAbsCall->alreadyStored = TRUE;
			}
			tmp2 = tmp2->next;
		}
		
		tmp = tmp->next;
		curId++;
	}
	
	HANDLE addressBufferStorage = VirtualAllocEx(hProcess, NULL, (curId+1)*sizeof(DWORD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	if(!addressBufferStorage)
		return FALSE;
	
	//Now that everything is setup we'll change the id to the correct memory address
	tmp = pAbsInstruction;
	while(tmp){
		
		if(tmp->type != ABS_CALL){
			tmp = tmp->next;
			continue;
		}
		
		if(!tmp->pAbsCall->alreadyStored)//??
			return FALSE;

		tmp->pAbsCall->address = (DWORD)(addressBufferStorage+tmp->pAbsCall->idInStorage*sizeof(DWORD));
		tmp = tmp->next;
	}
	
	if(!WriteProcessMemory(hProcess, (void*)addressBufferStorage, addressBuffer, (curId+1)*sizeof(DWORD), NULL))
		return FALSE;
	
	return TRUE;
}

BYTE* createNewFunction(const BYTE *originalFunction, DWORD newFunctionSize, PABS_INSTRUCTION pAbsInstruction){
	
	DWORD curPosOrig = 0, curPosNew = 0;
	DWORD sizeDifference = 0;
	
	PABS_INSTRUCTION tmpAbs;
	tmpAbs = pAbsInstruction;
	
	BYTE* newFunction = NULL;
	newFunction = malloc(newFunctionSize);
	
	if(!newFunction)//Failed
		return newFunction;
		
	memset(newFunction, 0, newFunctionSize);
	
	while(curPosNew != newFunctionSize){
		
		sizeDifference = 0;
		memcpy((void*)newFunction+curPosNew, (void*)originalFunction+curPosOrig, ((tmpAbs == NULL) ? newFunctionSize : tmpAbs->bytePosition) - curPosNew);
		sizeDifference = (((tmpAbs == NULL) ? newFunctionSize : tmpAbs->bytePosition) - curPosNew);
		curPosNew += sizeDifference;
		curPosOrig += sizeDifference;
		
		if(tmpAbs){
			memcpy((void*)newFunction+curPosNew, ((tmpAbs->type == ABS_CALL) ? (void*)tmpAbs->pAbsCall : (void*)tmpAbs->pPushRetn), tmpAbs->newSize);//Copy the fixed
			curPosNew += tmpAbs->newSize;
			curPosOrig += tmpAbs->originalSize;
			tmpAbs = tmpAbs->next;
		}
	}
	
	return newFunction;
}