#include "xor.h"

extern csh csHandle;
extern ks_engine *ksHandle;
uint32_t randNumber = 0;

#define CALL_ZERO_ASM "call 5; add dword ptr ss:[esp], 0xA; push 0x%08X; ret\0"
#define CALL_EAX_ASM "MOV EAX, 0x%08X; CALL EAX\0"

OutsideInstruction *createOustideInstruction(){
	
	OutsideInstruction *tmpOI = NULL;
	tmpOI = malloc(sizeof(OutsideInstruction));
	if(!tmpOI){
		tmpOI = malloc(sizeof(OutsideInstruction));//one last try
		if(!tmpOI)
			return NULL;
	}
	
	memset(tmpOI, 0, sizeof(OutsideInstruction));
	return tmpOI;
}

bool addToOutisdeInstructionList(OutsideInstruction *list, cs_insn *insn, uint32_t insnId){
	
	if(!list){
		printf("Invalid instruction list.\n");
		return false;	
	}
	
	//only go forward if the instruction is a jump/call
	if(insn->id != X86_INS_CALL && insn->id != X86_INS_JMP){
		return false;
	}
	
	//search for empty spot
	OutsideInstruction *tmpOI = list;
	while(tmpOI->next){
		tmpOI = tmpOI->next;
	}
	
	//add it
	tmpOI->id = insnId;
	tmpOI->type = insn->id;
	tmpOI->destinationAddress = insn->detail->x86.operands[0].imm;
	
	//create a new spot
	tmpOI->next = createOustideInstruction();
	
	if(!tmpOI){
		printf("Failed to create a new spot for a new instruction..");
		return false;
	}
	return true;
}

bool fixOutisdeInstructionList(OutsideInstruction *list){
	
	OutsideInstruction *tmpOI = list;
	
	while(tmpOI){
		
		if(tmpOI->type == X86_INS_CALL){
			
			randNumber = rand()%2;
			
			if(!randNumber){
			
				if(!generateAsm(&tmpOI->fixedOI, &tmpOI->fixedOISize, CALL_ZERO_ASM, tmpOI->destinationAddress)){
					printf("Error generating Asm for OIList.\n");
					return false;
				}
			}
			else{
				if(!generateAsm(&tmpOI->fixedOI, &tmpOI->fixedOISize, CALL_EAX_ASM, tmpOI->destinationAddress)){
					printf("Error generating Asm for OIList.\n");
					return false;
				}
				
			}
		}
		else if(tmpOI->type == X86_INS_JMP){
			//TODO
		}
		else{
			printf("Unknown instruction type in OI list. %d\n", tmpOI->type);
			return false;
		}
		
		tmpOI = tmpOI->next;
	}
	return true;

}

