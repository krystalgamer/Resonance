#include "xor.h"
#include "trashList.h"

extern csh csHandle;
extern ks_engine *ksHandle;
extern uint32_t randNumber;


TrashInstruction *createTrashInstruction(){
	
	TrashInstruction *tmpTI = NULL;
	tmpTI = malloc(sizeof(TrashInstruction));
	if(!tmpTI){
		tmpTI = malloc(sizeof(TrashInstruction));
		if(!tmpTI)
			return NULL;
	}
	
	memset(tmpTI, 0, sizeof(TrashInstruction));
	return tmpTI;
}

bool addToTrashInstructionList(TrashInstruction *list, uint32_t insnId){
	TrashInstruction *tmpTI = list;
	
	while(tmpTI->next)
		tmpTI = tmpTI->next;
	
	
	tmpTI->id = insnId;
	
	if(!generateTrashInstruction(tmpTI)) return false;
	
	tmpTI->next = createTrashInstruction();
	if(!tmpTI->next)
		return false;
	
	return true;
}

bool generateTrashInstruction(TrashInstruction *curTI){
	
	#define MAX_TRASH_INS 10
	randNumber = rand()%TRASH_TYPES;
	
	uint8_t *tmpTrashBuffer = NULL;
	uint32_t tmpTrashSize = 0, totalTrashSize = 0;
	static uint8_t trashBuffer[64];
	
	uint32_t trashNum = rand()%MAX_TRASH_INS + 1;
	for(int i = 0; i < trashNum; i++){
		if(!generateAsm2(&tmpTrashBuffer, &tmpTrashSize, trashList[randNumber][rand()%trashSizes[randNumber]], cs_reg_name(csHandle, allRegs[randNumber][rand()%regSizes[randNumber]])))	return false;
		
		memcpy(&trashBuffer[totalTrashSize], tmpTrashBuffer, tmpTrashSize);
		ks_free(tmpTrashBuffer);
		totalTrashSize += tmpTrashSize;
	}
	
	curTI->trashInsn = NULL;
	curTI->trashInsn = malloc(totalTrashSize);
	
	if(!curTI->trashInsn) return false;
	
	memcpy(curTI->trashInsn, trashBuffer, totalTrashSize);
	curTI->trashSize = totalTrashSize;
	
	return true;
}