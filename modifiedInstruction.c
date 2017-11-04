#include "xor.h"
#include "modifiedInstruction.h"

extern csh csHandle;
extern ks_engine *ksHandle;
extern uint32_t randNumber;

ModifiedInstruction *createModifiedInstruction(){
	
	ModifiedInstruction *tmpMI = NULL;
	tmpMI = malloc(sizeof(ModifiedInstruction));
	if(!tmpMI){
		tmpMI = malloc(sizeof(ModifiedInstruction));//one last try
		if(!tmpMI)
			return NULL;
	}
	
	memset(tmpMI, 0, sizeof(ModifiedInstruction));
	return tmpMI;
}

bool addToModifiedInstructionList(ModifiedInstruction *list, cs_insn *insn, uint32_t insnId){
	
	static PolyFunc polyFuncResult = POLY_OK;
	
	if(!list){
		printf("Invalid modified instruction list.\n");
		return false;	
	}

	//search for empty spot
	ModifiedInstruction *tmpMI = list;
	while(tmpMI->next){
		tmpMI = tmpMI->next;
	}
	
	tmpMI->id = insnId;
	switch(insn->id){
		
		case X86_INS_MOV:
			polyFuncResult = movPolymorphic(tmpMI, insn);	break;
		case X86_INS_PUSH:
			polyFuncResult = pushPolymorphic(tmpMI, insn);	break;
		case X86_INS_OR:
			polyFuncResult = orPolymorphic(tmpMI, insn);	break;
		case X86_INS_RET:
			polyFuncResult = retPolymorphic(tmpMI, insn);	break;
		default: return true;
	}
	
	if(polyFuncResult == POLY_FAIL) return false;
	else if(polyFuncResult == POLY_UNIMPLEMENTED) return true;//avoid creating useless extra spots
	
	//create a new spot
	tmpMI->next = createModifiedInstruction();
	
	if(!tmpMI){
		printf("Failed to create a new spot for a new modified instruction..");
		return false;
	}
	return true;
}

PolyFunc movPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn){
	
	if(insn->id != X86_INS_MOV)
		return POLY_FAIL;
	
	if(insn->detail->x86.op_count != 2)
		return POLY_FAIL;//mov alwys has 2 operands
	
	if(insn->detail->x86.operands[0].type == X86_OP_MEM){
		x86_op_mem memOperand = insn->detail->x86.operands[0].mem;
		switch(insn->detail->x86.operands[1].type){
			case X86_OP_IMM:
				if(!generateAsm2(&modInsn->moddedInsn, &modInsn->moddedSize, movMemImmPoly[rand()%MOV_MEM_IMM_POLY_SIZE], MEM_PARAM(memOperand), insn->detail->x86.operands[0].imm))
					return POLY_FAIL;
			break;

			case X86_OP_REG:
				if(!generateAsm2(&modInsn->moddedInsn, &modInsn->moddedSize, movMemRegPoly[rand()%MOV_MEM_REG_POLY_SIZE], MEM_PARAM(memOperand), REG_NAME(insn->detail->x86.operands[0].reg)))
					return POLY_FAIL;
			break;
		}
		
	}
		
	switch(insn->detail->x86.operands[1].type){
		case X86_OP_IMM:
			if(!generateAsm2(&modInsn->moddedInsn, &modInsn->moddedSize, movRegImmPoly[rand()%MOV_REG_IMM_POLY_SIZE], REG_NAME(insn->detail->x86.operands[0].reg), insn->detail->x86.operands[1].imm))
				return POLY_FAIL;
			break;

		case X86_OP_REG:
			if(!generateAsm2(&modInsn->moddedInsn, &modInsn->moddedSize, movRegRegPoly[rand()%MOV_REG_REG_POLY_SIZE], REG_NAME(insn->detail->x86.operands[0].reg), REG_NAME(insn->detail->x86.operands[1].reg))) 
				return POLY_FAIL;
			break;
	}
	
	return POLY_OK;
}
PolyFunc pushPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn){
	
	if(insn->id != X86_INS_PUSH)
		return POLY_FAIL;
	
	if(insn->detail->x86.op_count != 1)
		return POLY_FAIL;	
		
	const uint8_t *regName = cs_reg_name(csHandle, insn->detail->x86.operands[0].reg);
	
	//randNumber = rand()%2;
	switch(insn->detail->x86.operands[0].type){
		
		case X86_OP_MEM:
			return POLY_UNIMPLEMENTED;
		case X86_OP_IMM:
			#define PUSH_SUB_MOV_IMM "SUB ESP, 4; MOV DWORD PTR SS:[ESP], 0x%08X\0"
			if(!generateAsm(&modInsn->moddedInsn, &modInsn->moddedSize, PUSH_SUB_MOV_IMM, insn->detail->x86.operands[0].imm)) return POLY_FAIL;
			break;
		case X86_OP_REG:
			#define PUSH_SUB_MOV_REG "SUB ESP, 4; MOV DWORD PTR SS:[ESP], %s\0"
			if(!generateAsm(&modInsn->moddedInsn, &modInsn->moddedSize, PUSH_SUB_MOV_REG, regName)) return POLY_FAIL;
			break;
		default: return POLY_FAIL;
	}
	return POLY_OK;
}
PolyFunc orPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn){
	
	if(insn->id != X86_INS_OR)
		return POLY_FAIL;
	
	if(insn->detail->x86.op_count != 2)
		return POLY_FAIL;	
		
	x86_op_type op1 = insn->detail->x86.operands[0].type, op2 = insn->detail->x86.operands[1].type;
	
	if(op1 == X86_OP_IMM)//never happens
		return POLY_FAIL;
		
	if(op1 == X86_OP_REG){
			switch(op2){
				case X86_OP_IMM:
				case X86_OP_MEM: 	return POLY_UNIMPLEMENTED;
				case X86_OP_REG:
					#define OR_NAND_REG_REG "PUSH $1s; NOT DWORD PTR SS:[ESP]; NOT $0s; AND $0s, [ESP]; NOT $0s; ADD ESP, 4\0"
					if(!generateAsm2(&modInsn->moddedInsn, &modInsn->moddedSize, OR_NAND_REG_REG, 
					REG_NAME(insn->detail->x86.operands[0].reg), REG_NAME(insn->detail->x86.operands[0].reg))) return POLY_FAIL;
					break;
				default: 			return POLY_FAIL;
			}
	}
	else
		return POLY_UNIMPLEMENTED;
	return POLY_OK;
}
PolyFunc retPolymorphic(ModifiedInstruction *modInsn, cs_insn *insn){
	
	if(insn->id != X86_INS_RET)
		return POLY_FAIL;
	
	if(insn->detail->x86.op_count > 1)
		return POLY_FAIL;	
	
	if(insn->detail->x86.op_count == 0){
		#define RET_ADD_JMP "ADD ESP, 4; JMP DWORD PTR SS:[ESP-4]\0"
		if(!generateAsm2(&modInsn->moddedInsn, &modInsn->moddedSize, RET_ADD_JMP)) return POLY_FAIL;
	}
	else
		return POLY_UNIMPLEMENTED;
	
	
	return POLY_OK;
}