#include "xor.h"


bool generateAsm(uint8_t **encoding, size_t *encodingSize, const uint8_t *asmString, ...);
void formatAsmString(va_list valist, uint8_t *asmBuffer,const uint8_t *asmString, uint32_t numReplace);
void newFormatAsmString(va_list valist, uint8_t *asmBuffer,const uint8_t *asmString, uint32_t *indexOrder,uint32_t numReplace);

ks_engine *ksHandle = NULL;
csh csHandle;

bool initEngines(){
	
	if(ksHandle)//if this is set then capstone is set
		return true;
		
	//capstone
	if (cs_open(CS_ARCH_X86, CS_MODE_32, &csHandle) != CS_ERR_OK)
      return false;
	cs_option(csHandle, CS_OPT_DETAIL, CS_OPT_ON);
	
	//keystone
	if(ks_open(KS_ARCH_X86, KS_MODE_32, &ksHandle) != KS_ERR_OK)
		return false;
	ks_option(ksHandle, KS_OPT_SYNTAX, KS_OPT_SYNTAX_RADIX16);
	
	return true;
}

bool interptFunction(HANDLE hProcess, FUNCTION function){
	
	if(!initEngines()){
		printf("Couldn't init engines..\n");
		return false;
	}
	
	unsigned char *buffer;
	if(!(buffer = malloc(function.size)))
		return false;
	
	//Reads function from memory
	if(!ReadProcessMemory(hProcess, (void*)function.startAddr, buffer, function.size, NULL)){
		MessageBoxA(NULL, "Couldn't read the function!", "FAILED", 0);
		return false;
	}

	cs_insn *insn;
	size_t count;
 
	//capstone stuff
	count = cs_disasm(csHandle, buffer, function.size, function.startAddr, 0, &insn);
	
	OutsideInstruction *OIList = NULL;
	ModifiedInstruction *MIList = NULL;
	TrashInstruction *TIList = NULL;
	JmpInstruction *JIList = NULL;
	
	OIList = createOustideInstruction();
	MIList = createModifiedInstruction();
	TIList = createTrashInstruction();
	JIList = createJmpInstruction();
	
	if(count <= 0){
		printf("ERROR: Failed to disassemble given code!\n");
		return false;
	}
	
	size_t currIns = 0;
	cs_x86 *curInstruction = NULL;
		
	for (currIns = 0; currIns < count; currIns++) {
		if(insn[currIns].id == X86_INS_JMP || insn[currIns].id == X86_INS_CALL){
			
			curInstruction = &(insn[currIns].detail->x86);//curInstruction now holds the x86 details
			if(curInstruction->op_count != 1)//In jumps or calls op_count must always be 1
				return false;
			
			cs_x86_op *operand = &(curInstruction->operands[0]);
			if(operand->type != X86_OP_IMM)//only work with immediates
				continue;
			
			//Checks if jump goes to outside the function
			if(operand->imm > function.startAddr + function.size){
				addToOutisdeInstructionList(OIList, &insn[currIns], currIns);
				
				printf("Found relative %s to fix!\n", (insn[currIns].id == X86_INS_CALL ? "call" : "jmp"));
			}
			else{
				printf("One internal relative jmp might need to be fixed\n");
				if(!addToJmpInstructionList(JIList, currIns)){
					printf("Unknown error occured\n");
					return false;
				}
			}
		}
		else{
			if(!addToModifiedInstructionList(MIList, &insn[currIns], currIns)){
				printf("Error adding instruction to modified instruction list.\n");
				return false;
			}
			
		}
		
		if(!addToTrashInstructionList(TIList, currIns)){
				printf("Couldn't some trash to it :(\n");
				return false;
		}
	}
	
	removeLastEntry((void*)&OIList, sizeof(OutsideInstruction));
	removeLastEntry((void*)&MIList, sizeof(ModifiedInstruction));
	removeLastEntry((void*)&TIList, sizeof(TrashInstruction));
	removeLastEntry((void*)&JIList, sizeof(JmpInstruction));
	
	if(!fixOutisdeInstructionList(OIList)){
		printf("Couldn't fix OutsideInstructionList.\n");
		return false;
	}

	if(!fixAffectedRelativeJmps(insn, JIList, OIList, MIList, TIList)){
		printf("Coudln't fix relative jmps.\n");
		return false;
	}
	
		  
	
	uint32_t sizeOfFixedFunction = 0;
	uint8_t *fixedFunction = createNewFunction(insn, OIList, MIList, TIList, JIList, count, &sizeOfFixedFunction);
	HANDLE address2 = VirtualAllocEx(hProcess, NULL, sizeOfFixedFunction, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	printf("Function at address %08X\n", address2);
	if(!address2)
		return false;
	
	if(!WriteProcessMemory(hProcess, (void*)address2, (void*)fixedFunction, sizeOfFixedFunction, NULL))
		return false;
	
	uint8_t *encoding = NULL;
	size_t encodingSize = 0;
	size_t processedInsn = 0;
	
	//place a jmp
	#define PUSH_RETN_JMP "PUSH 0x%08X; RET\0"
	generateAsm(&encoding, &encodingSize, PUSH_RETN_JMP, address2);
	if(!WriteProcessMemory(hProcess, (void*)function.startAddr, encoding, encodingSize, NULL))
		return false;
  
	fflush(stdout);
	cs_free(insn, count);
	cs_close(&csHandle);
	return true;
	
	
}

bool generateAsm(uint8_t **encoding, size_t *encodingSize, const uint8_t *asmString, ...){
	
	size_t numInstructions = 0;
	uint32_t sizeOfAsmString = strlen(asmString);
	uint32_t counter = 0;
	uint8_t *curPosString = NULL;
	uint32_t numReplace = 0;
	static uint8_t asmBuffer[128];
	
	//how many things we have to replace
	while(counter < sizeOfAsmString){
		
		curPosString = strstr((char*)((uint32_t)asmString+counter), "%");
		if(!curPosString)
			break;
		
		numReplace++;
		
		counter = (uint32_t)curPosString - (uint32_t)asmString;
		counter++;
		
	}
	
	if(numReplace){
		//replace them only if there's anythint to replace
		va_list valist;
		va_start(valist, asmString);
		formatAsmString(valist, asmBuffer, asmString, numReplace);
	}
	else{
		strcpy(asmBuffer, asmString);
	}
	
	
	counter = numReplace = 0;
	//numReplace = processedInsn
	while(counter < sizeOfAsmString){
		
		curPosString = strstr((char*)((uint32_t)asmString+counter), ";");
		if(!curPosString)
			break;
		
		numReplace++;
		
		counter = (uint32_t)curPosString - (uint32_t)asmString;
		counter++;
		
	}
	numReplace++;//last doesn't have semicolon
	
	uint32_t processedInsn = 0;
	if(ks_asm(ksHandle, asmBuffer, 0, encoding, encodingSize, &processedInsn)){
		printf("Error %s\n", ks_strerror(ks_errno(ksHandle)));
	}
	if(processedInsn != numReplace){
		printf("Error couldn't process all instructions :(\n");
		return false;
	}
	return true;
}

uint32_t getNewFunctionSize(cs_insn *insn, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList, JmpInstruction *JIList,uint32_t numInstructions){
	
	
	OutsideInstruction *tmpOI = OIList;
	ModifiedInstruction *tmpMI = MIList;
	TrashInstruction *tmpTI = TIList;
	JmpInstruction *tmpJI = JIList;
	
	uint32_t finalSize = 0;
	uint32_t counter = 0;
	
	while(counter < numInstructions){
				
			if(!tmpOI && !tmpMI && !tmpTI && !tmpJI){
				finalSize += insn[counter].size;//no more instructions to fix, just add its regular size
			}
			else{
				
				//needs to be the first since doesn't substitute any function
				if(tmpTI){
					if(tmpTI->id == counter){
						finalSize += tmpTI->trashSize;
						tmpTI = tmpTI->next;
					}
				}
				
				if(tmpOI){
					if(tmpOI->id == counter){//wait until the ids match
						finalSize += tmpOI->fixedOISize;
						tmpOI = tmpOI->next;
						counter++;
						continue;
					}
				}
				if(tmpMI){
					if(tmpMI->id == counter){//wait until the ids match
						finalSize += tmpMI->moddedSize;
						tmpMI = tmpMI->next;
						counter++;
						continue;
					}
				}
				if(tmpJI){
					if(tmpJI->id == counter){
						finalSize += tmpJI->jmpSize;
						tmpJI = tmpJI->next;
						counter++;
						continue;
					}
				}
				finalSize += insn[counter].size;//both conditions up there weren't met
				
			}
			
		
		counter++;
	}
	return finalSize;
}

//creates with the fixed jmps/calls and modified the instructions
uint8_t *createNewFunction(cs_insn *insn, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList, JmpInstruction *JIList,uint32_t numInstructions, uint32_t *sizeOfFixedFunction){
	
	*sizeOfFixedFunction = 0;
	*sizeOfFixedFunction = getNewFunctionSize(insn, OIList, MIList, TIList, JIList, numInstructions);
	uint8_t *newFunction = NULL;
	
	newFunction = malloc(*sizeOfFixedFunction);
	if(!newFunction){
		printf("Failed to allocate space for the new function :(");
		return NULL;
	}
	
	OutsideInstruction *tmpOI = OIList;
	ModifiedInstruction *tmpMI = MIList;
	TrashInstruction *tmpTI = TIList;
	JmpInstruction *tmpJI = JIList;
	
	uint32_t counter = 0;
	uint32_t posInBuffer = 0;
	while(counter<numInstructions){
		
		if(tmpOI){
			if(tmpOI->id == counter){
				memcpy(&newFunction[posInBuffer], tmpOI->fixedOI, tmpOI->fixedOISize);
				posInBuffer += tmpOI->fixedOISize;
				
				//free the keystone stuff
				ks_free(tmpOI->fixedOI);
				tmpOI->fixedOI = NULL;
				tmpOI = tmpOI->next;
				
				goto trashPart;
			}
		}
		if(tmpMI){
			if(tmpMI->id == counter){
				memcpy(&newFunction[posInBuffer], tmpMI->moddedInsn, tmpMI->moddedSize);
				posInBuffer += tmpMI->moddedSize;
				
				ks_free(tmpMI->moddedInsn);
				tmpMI->moddedInsn = NULL;
				tmpMI = tmpMI->next;
				
				goto trashPart;
			}
		}
		if(tmpJI){
			if(tmpJI->id == counter){
				memcpy(&newFunction[posInBuffer], tmpJI->jmpInsn, tmpJI->jmpSize);
				posInBuffer += tmpJI->jmpSize;
				
				ks_free(tmpJI->jmpInsn);
				tmpJI->jmpInsn = NULL;
				tmpJI = tmpJI->next;
				
				goto trashPart;
			}
		}
		
		memcpy(&newFunction[posInBuffer], insn[counter].bytes, insn[counter].size);
		posInBuffer += insn[counter].size;
		
		trashPart:
		if(tmpTI){
			if(tmpTI->id == counter){
				memcpy(&newFunction[posInBuffer], tmpTI->trashInsn, tmpTI->trashSize);
				posInBuffer += tmpTI->trashSize;
				tmpTI = tmpTI->next;
			}
		}
		
		counter++;
	}
	
	return newFunction;
}

bool generateAsm2(uint8_t **encoding, size_t *encodingSize, const uint8_t *asmString, ...){
	
	uint32_t sizeOfAsmString = strlen(asmString);
	uint32_t sizeScanned = 0;
	uint32_t curNumber = 0;
	uint8_t *curPosString = NULL;
	uint32_t numReplace = 0;
	static uint8_t asmBuffer[128];
	static uint8_t asmBuffer2[128];
	static uint32_t indexOrder[32];//should be enough
	uint32_t curIndexOrder = 0;
	uint32_t asmBufferPos = 0;
	
	//gets the max index and creates indexOrderList
	while(sizeScanned < sizeOfAsmString){
		
		curPosString = strstr((&asmString[sizeScanned]), "$");
		if(!curPosString)
			break;
		
		numReplace++;
		
		sizeScanned = (uint32_t)curPosString - (uint32_t)asmString + 1;
		curNumber = atoi(&asmString[sizeScanned]);
		indexOrder[curIndexOrder++] = curNumber;
		
	}
	
	//creates a tmp string with the correct formatings
	sizeScanned = 0;
	while(sizeScanned <= sizeOfAsmString){
		if(!numReplace)//only create a tmp string if needed
			break;
		
		curPosString = strstr((&asmString[sizeScanned]), "$");
		if(!curPosString){
			while((asmBuffer2[asmBufferPos++] = asmString[sizeScanned++]));//copy the rest of the string must be NULL terminated
			break;
		}
		
		memcpy(&asmBuffer2[asmBufferPos], &asmString[sizeScanned], (uint32_t)curPosString - (uint32_t)asmString - sizeScanned);
		asmBufferPos += (uint32_t)curPosString - (uint32_t)asmString - sizeScanned;
		sizeScanned = (uint32_t)curPosString - (uint32_t)asmString;
		
		while(isdigit(asmString[++sizeScanned]));//ignore the index
		
		if(asmString[sizeScanned] == 's'){
			strcpy((char*)&asmBuffer2[asmBufferPos], "%s");
			asmBufferPos += 2;
		}
		else if(asmString[sizeScanned] == 'x'){
			strcpy((char*)&asmBuffer2[asmBufferPos], "0x%08X");
			asmBufferPos += 6;
		}
		else{
			printf("Unknown character type\n");
			return false;
		}
		
		sizeScanned++;
	}
	
	if(numReplace){
		va_list valist;
		va_start(valist, asmString);
		newFormatAsmString(valist, asmBuffer, asmBuffer2, &indexOrder[numReplace-1], numReplace);
	}
	else
		strcpy(asmBuffer, asmString);

	//numReplace=numInstructions
	sizeScanned = 0;
	numReplace = 1;//start at 1
	while(sizeScanned < sizeOfAsmString){
		
		curPosString = strstr(&asmString[sizeScanned], ";");
		if(!curPosString)
			break;
		
		sizeScanned = (uint32_t)curPosString - (uint32_t)asmString + 1;
		numReplace++;
	}
	
	uint32_t processedInsn = 0;
	if(ks_asm(ksHandle, asmBuffer, 0, encoding, encodingSize, &processedInsn)){
		printf("Error %s\n", ks_strerror(ks_errno(ksHandle)));
	}
	if(processedInsn != numReplace){
		printf("Error couldn't process all instructions :(\n");
		return false;
	}
	
	return true;
}
