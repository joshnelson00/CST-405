#ifndef MIPS_H
#define MIPS_H

/* MIPS INSTRUCTION TYPES */
typedef enum {
    MIPS_LI,        /* Load immediate: li $reg, imm */
    MIPS_LW,        /* Load word: lw $reg, offset($sp) */
    MIPS_SW,        /* Store word: sw $reg, offset($sp) */
    MIPS_L_S,       /* Load float: l.s $freg, offset($sp) */
    MIPS_S_S,       /* Store float: s.s $freg, offset($sp) */
    MIPS_ADD,       /* Add integers: add $dst, $src1, $src2 */
    MIPS_SUB,       /* Subtract integers: sub $dst, $src1, $src2 */
    MIPS_MUL,       /* Multiply integers: mult $src1, $src2; mflo $dst */
    MIPS_DIV,       /* Divide integers: div $src1, $src2; mflo $dst */
    MIPS_MFLO,      /* Move from lo: mflo $dst */
    MIPS_ADDI,      /* Add immediate: addi $dst, $src1, imm */
    MIPS_ADD_S,     /* Add floats: add.s $fdst, $fsrc1, $fsrc2 */
    MIPS_SUB_S,     /* Subtract floats: sub.s $fdst, $fsrc1, $fsrc2 */
    MIPS_MUL_S,     /* Multiply floats: mul.s $fdst, $fsrc1, $fsrc2 */
    MIPS_DIV_S,     /* Divide floats: div.s $fdst, $fsrc1, $fsrc2 */
    MIPS_MTC1,      /* Move to coprocessor 1: mtc1 $reg, $freg */
    MIPS_MFC1,      /* Move from coprocessor 1: mfc1 $reg, $freg */
    MIPS_CVT_S_W,   /* Convert int to float: cvt.s.w $fdest, $fsrc */
    MIPS_TRUNC_W_S, /* Truncate float to int: trunc.w.s $fdest, $fsrc */
    MIPS_MOVE,      /* Move register: move $dst, $src */
    MIPS_MOV_S,     /* Move float register: mov.s $fdst, $fsrc */
    MIPS_JAL,       /* Jump and link: jal label */
    MIPS_JR,        /* Jump register: jr $reg */
    MIPS_LABEL,     /* Label: name: */
    MIPS_SYSCALL,   /* System call */
    MIPS_COMMENT    /* Comment line */
} MIPSOp;

/* MIPS INSTRUCTION STRUCTURE */
typedef struct MIPSInstr {
    MIPSOp op;                     /* Operation type */
    char* result;                   /* Result register/destination */
    char* arg1;                     /* First operand/register */
    char* arg2;                     /* Second operand/register */
    char* comment;                  /* Optional comment */
    struct MIPSInstr* next;         /* Linked list pointer */
} MIPSInstr;

/* MIPS LIST MANAGEMENT */
typedef struct {
    MIPSInstr* head;                /* First instruction */
    MIPSInstr* tail;                /* Last instruction */
} MIPSList;

/* MIPS GENERATION FUNCTIONS */
MIPSInstr* createMIPS(MIPSOp op, char* result, char* arg1, char* arg2, char* comment);
void appendMIPS(MIPSList* list, MIPSInstr* instr);
int countMIPSInstructions(MIPSList* list);
void printMIPS(MIPSList* list, FILE* output);
void freeMIPSList(MIPSList* list);

#endif
