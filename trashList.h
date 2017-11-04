#include <stdint.h>

//registers
#define BIT32_REGS_SIZE 8
x86_reg bit32Regs[] = {
X86_REG_EAX, X86_REG_EBP, X86_REG_EBX, X86_REG_ECX, 
X86_REG_EDI, X86_REG_EDX, X86_REG_ESI, X86_REG_ESP
};

#define BIT8_REGS_SIZE 8
x86_reg bit8Regs[] = {
X86_REG_AH, X86_REG_AL, X86_REG_BH, X86_REG_BL,
X86_REG_CH, X86_REG_CL, X86_REG_DH, X86_REG_DL
};

#define MM_REGS_SIZE 16
x86_reg mmRegs[] = {
X86_REG_MM0, X86_REG_MM1, X86_REG_MM2, X86_REG_MM3,
X86_REG_MM4, X86_REG_MM5, X86_REG_MM6, X86_REG_MM7,
X86_REG_XMM0, X86_REG_XMM1, X86_REG_XMM2, X86_REG_XMM3,
X86_REG_XMM4, X86_REG_XMM5, X86_REG_XMM6, X86_REG_XMM7
};

x86_reg *allRegs[] = { bit32Regs, bit8Regs, mmRegs};

uint32_t regSizes[] = {
BIT32_REGS_SIZE, BIT8_REGS_SIZE, MM_REGS_SIZE
};


//instruction trash
#define MOV_REG_REG "MOV $0s, $0s\0"
#define LEA_REG_REG "LEA $0s, [$0s]\0"
#define ADD_REG_ZERO "ADD $0s, 0x00000000\0"
#define SUB_REG_ZERO "SUB $0s, 0x00000000\0"
#define XOR_REG_ZERO "XOR $0s, 0x00000000\0"
#define AND_REG_NEG1 "AND $0s, 0xFFFFFFFF\0"
#define AND_REG_REG "AND $0s, $0s\0"
#define OR_REG_ZERO "OR $0s, 0x00000000\0"
#define OR_REG_REG "OR $0s, $0s\0"
#define XCHG_REG_REG "XCHG $0s, $0s\0"
#define ROR_REG32 "ROR $0s, 0x20\0"
#define ROR_REG8 "ROR %0s, 0x8\0"
#define ROL_REG32 "ROL $0s, 0x20\0"
#define ROL_REG8 "ROL %0s, 0x8\0"
#define PUSH_POP_REG "PUSH $0s; POP $0s"
#define PAND_REG_REG "PAND $0s, $0s\0"
#define POR_REG_REG "POR $0s, $0s\0"

//trash arrays
#define BIT32_TRASH_SIZE 12
uint8_t *bit32Trash[] = {
MOV_REG_REG, LEA_REG_REG, ADD_REG_ZERO, SUB_REG_ZERO,
XOR_REG_ZERO, AND_REG_NEG1, AND_REG_REG, OR_REG_ZERO,
OR_REG_REG, XCHG_REG_REG, ROR_REG32, ROL_REG32, PUSH_POP_REG
};

#define BIT8_TRASH_SIZE 9
uint8_t *bit8Trash[] = {
MOV_REG_REG, ADD_REG_ZERO, SUB_REG_ZERO, XOR_REG_ZERO,
AND_REG_NEG1, AND_REG_REG, OR_REG_ZERO, OR_REG_REG,
XCHG_REG_REG, ROR_REG8, ROL_REG8
};

#define MM_TRASH_SIZE 2
uint8_t *mmTrash[] = {
PAND_REG_REG, POR_REG_REG
};

#define TRASH_TYPES 3
uint8_t **trashList[] = {
	bit32Trash, bit8Trash, mmTrash
};

uint32_t trashSizes[] = {
	BIT32_TRASH_SIZE, BIT8_TRASH_SIZE, MM_TRASH_SIZE
};



//nop
#define NOP "NOP\0"
#define FNOP "FNOP\0"

