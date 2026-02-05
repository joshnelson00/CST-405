#ifndef TAC_H
#define TAC_H

#include "ast.h"

/* THREE-ADDRESS CODE (TAC)
 * Intermediate representation between AST and machine code
 * Each instruction has at most 3 operands (result = arg1 op arg2)
 * Makes optimization and code generation easier
 */

/* TAC INSTRUCTION TYPES */
typedef enum {
    TAC_ADD,         /* Addition: result = arg1 + arg2 */
    TAC_SUBTRACT,
    TAC_MULTIPLY,
    TAC_DIVIDE,
    TAC_ASSIGN,      /* Assignment: result = arg1 */
    TAC_ARRAY_WRITE, /* Array write: arr[arg1] = arg2 */
    TAC_ARRAY_READ,  /* Array read: result = arr[arg1] */
    TAC_PRINT,       /* Print: print(arg1) */
    TAC_DECL,        /* Declaration: declare result */
    TAC_ARRAY_DECL,  /* Array declaration: declare result[size] */
    TAC_FUNC_DEF,    /* Function definition: func name */
    TAC_FUNC_CALL,   /* Function call: result = call func(args) */
    TAC_PARAM,       /* Function parameter: param name */
    TAC_RETURN,      /* Return statement: return expr */
    TAC_ARG,         /* Function argument: arg expr */
    TAC_ARRAY_DECL,  /* Array declaration: declare array name[size] */  
    TAC_ARRAY_ASSIGN, /* Array assignment: name[index] = value */
    TAC_ARRAY_ACCESS  /* Array access: result = name[index] */
} TACOp;

/* TAC INSTRUCTION STRUCTURE */
typedef struct TACInstr {
    TACOp op;               /* Operation type */
    char* arg1;             /* First operand (if needed) */
    char* arg2;             /* Second operand (for binary ops) */
    char* result;           /* Result/destination */
    struct TACInstr* next;  /* Linked list pointer */
} TACInstr;

/* TAC LIST MANAGEMENT */
typedef struct {
    TACInstr* head;    /* First instruction */
    TACInstr* tail;    /* Last instruction (for efficient append) */
    int tempCount;     /* Counter for temporary variables (t0, t1, ...) */
} TACList;

/* TAC GENERATION FUNCTIONS */
void initTAC();                                                    /* Initialize TAC lists */
char* newTemp();                                                   /* Generate new temp variable */
TACInstr* createTAC(TACOp op, char* arg1, char* arg2, char* result); /* Create TAC instruction */
void appendTAC(TACInstr* instr);                                  /* Add instruction to list */
void generateTAC(ASTNode* node);                                  /* Convert AST to TAC */
char* generateTACExpr(ASTNode* node);                             /* Generate TAC for expression */
char* generateTACFuncCall(ASTNode* node);                         /* Generate TAC for function call */
void generateTACArgList(ASTNode* node);                           /* Generate TAC for argument list */

/* TAC OUTPUT FUNCTIONS */
void printTAC();                                                   /* Display unoptimized TAC */
void appendOptimizedTAC(TACInstr* instr);                         /* Add instruction to optimized list */
void freeTACList(TACList* list);                                  /* Free TAC instruction list memory */

#endif
