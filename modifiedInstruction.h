#include <stdint.h>

//mov

//REG XXX
//reg imm
#define MOV_XOR_REG_IMM "XOR $0s, $0s; XOR $0s, $1x\0"
#define MOV_AND_REG_IMM "MOV $0s, 0xFFFFFFFF;AND $0s, $1x\0"
#define MOV_XOR_ADD_REG_IMM "XOR $0s, $0s; ADD $0s, $1x\0"
#define MOV_XOR_NEG_AND_REG_IMM "XOR $0s, $0s; NEG $0s; AND $0s, $1s"
#define MOV_PUSH_POP_REG_IMM "PUSH $1x; POP $0s\0"

#define MOV_REG_IMM_POLY_SIZE 5
uint8_t *movRegImmPoly[] = {
MOV_XOR_REG_IMM, MOV_AND_REG_IMM, MOV_XOR_ADD_REG_IMM, MOV_XOR_NEG_AND_REG_IMM,
MOV_PUSH_POP_REG_IMM
};

//reg reg
#define MOV_XOR_REG_REG "XOR $0s, $0s; XOR $0s, $1s\0"
#define MOV_PUSH_POP_REG_REG "PUSH $1s; POP $0s\0"
#define MOV_MOV_XCHG_MOV_REG_REG "XCHG $0s, $1s; MOV $1s, $0s\0"

#define MOV_REG_REG_POLY_SIZE 3
uint8_t *movRegRegPoly[] = {
MOV_XOR_REG_REG, MOV_PUSH_POP_REG_REG, MOV_MOV_XCHG_MOV_REG_REG
};

//MEM XXX
//mem imm
#define MOV_PUSH_POP_MEM_IMM "PUSH $5x; POP $0s:[$1s+$2s*$3x+$4x]"

#define MOV_MEM_IMM_POLY_SIZE 1
uint8_t *movMemImmPoly[] = {
MOV_PUSH_POP_MEM_IMM
};

//mem reg
#define MOV_PUSH_POP_MEM_REG "PUSH $5s; POP $0s:[$1s+$2s*$3x+$4x]"

#define MOV_MEM_REG_POLY_SIZE 1
uint8_t *movMemRegPoly[] = {
MOV_PUSH_POP_MEM_REG
};

#define MEM_PARAM(x) ((x.segment == X86_REG_INVALID ? "SS" : REG_NAME(x.segment)), (x.base == X86_REG_INVALID ? "0" : REG_NAME(x.base)), (x.index == X86_REG_INVALID ? "0" : REG_NAME(x.index)), x.scale, (uint32_t)x.disp)
/*

	unsigned int segment; // segment register (or X86_REG_INVALID if irrelevant)
	unsigned int base;	// base register (or X86_REG_INVALID if irrelevant)
	unsigned int index;	// index register (or X86_REG_INVALID if irrelevant)
	int scale;	// scale for index register
	int64_t disp;	// displacement value
	*/