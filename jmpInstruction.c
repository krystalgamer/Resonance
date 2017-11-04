#include "xor.h"

JmpInstruction *createJmpInstruction(){
	
	JmpInstruction *tmpJI = NULL;
	tmpJI = malloc(sizeof(JmpInstruction));
	if(!tmpJI){
		tmpJI = malloc(sizeof(JmpInstruction));
		
		if(!tmpJI) return NULL;
	}
	
	memset(tmpJI, 0, sizeof(JmpInstruction));
	return tmpJI;
	
	
}

bool addToJmpInstructionList(JmpInstruction *list, uint32_t insnId){
	
	JmpInstruction *tmpJI = list;
	
	while(tmpJI->next)
		tmpJI = tmpJI->next;
	
	tmpJI->id = insnId;
	
	tmpJI->next = createJmpInstruction();
	
	if(!tmpJI->next) return false;
	
	return true;
}

//todo add in the future if the immediate is bigger
uint32_t getOriginalJmpResult(cs_insn *insn){
	
	uint32_t origOffset = insn->detail->x86.operands[0].imm - insn->address - insn->size;//jmp starts at the end
	uint32_t counter = 0, destinyId = 0;
	for(destinyId = 0; counter < origOffset; destinyId++)
		counter += insn[destinyId].size;
	
	return destinyId;
	
}

void setListsInCorrectId(uint32_t id, OutsideInstruction **OIList, ModifiedInstruction **MIList, TrashInstruction **TIList){
	
	while(*OIList){
		
		if((*OIList)->id > id) break;//shouldn't be equal in this case lol
		
		*OIList = (*OIList)->next;
	}
	
	while(*MIList){
		
		if((*MIList)->id > id) break;
		
		*MIList = (*MIList)->next;
	}
	
	while(*TIList){
		if((*TIList)->id >= id) break;
		
		*TIList = (*TIList)->next;
	}
	
}

uint32_t getNewJmpOffset(uint32_t jmpId, uint32_t destIdOff, cs_insn *insn, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList){
	
	OutsideInstruction *tmpOI = OIList;
	ModifiedInstruction *tmpMI = MIList;
	TrashInstruction *tmpTI = TIList;
	
	bool customInsn = false;
	uint32_t newOffset = 0;
	for(int i = 0; i <  destIdOff; i++){
		
		customInsn = false;
		setListsInCorrectId(jmpId+i, &tmpOI, &tmpMI, &tmpTI);//tash instructions are tricky
		
		
		if(tmpTI){
			if(tmpTI->id == (jmpId + i)){
				newOffset += tmpTI->trashSize;
				tmpTI = tmpTI->next;
			}
		}
		if(tmpOI){
			if(tmpOI->id == (jmpId + i)){
				newOffset += tmpOI->fixedOISize;
				tmpOI = tmpOI->next;
				customInsn = true;
			}	
		}
		if(tmpMI){
			if(tmpMI->id == (jmpId + i)){
				newOffset += tmpMI->moddedSize;
				tmpMI = tmpMI->next;
				customInsn = true;
			}
			
		}
		
		//if this instruction hasn't been modified then use the regular size
		if(!customInsn)
			newOffset += (i ? insn[i].size : 0);
	}
	return newOffset;
	
}


bool fixAffectedRelativeJmps(cs_insn *insn, JmpInstruction *JIList, OutsideInstruction *OIList, ModifiedInstruction *MIList, TrashInstruction *TIList){
	
	JmpInstruction *curJmpInsn = JIList;
	uint32_t destinyId = 0;
	uint32_t newJmpOffset = 0;
	
	while(curJmpInsn){
		
		newJmpOffset = destinyId = 0;
		destinyId = getOriginalJmpResult(&insn[curJmpInsn->id]);
		
		newJmpOffset = getNewJmpOffset(curJmpInsn->id, destinyId, &insn[curJmpInsn->id], OIList, MIList, TIList);
		printf("New offset: %d\n", newJmpOffset);
		
		#define NEW_JMP "%s 0x%08X\n"
		if(!generateAsm(&curJmpInsn->jmpInsn, &curJmpInsn->jmpSize, NEW_JMP, insn[curJmpInsn->id].mnemonic, newJmpOffset+(newJmpOffset > 0xFF ? 5 : 2))) return false;
		
		curJmpInsn = curJmpInsn->next;
	}
	
	
	return true;
	
}

