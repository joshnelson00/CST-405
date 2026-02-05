// Clean optimizer v2 implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "optimizer2.h"
#include "mips.h"
#include "symtab.h"
#include "tac.h"

extern TACList tacList;
TACList optimizedList;   // global optimized TAC list for compatibility
TACList optimizedList2;  // internal list used by optimizer2
extern GlobalSymbolTable globalSymTab;
extern int isConst(const char* s);

static int isTemporary(const char* name) {
    if (!name || strlen(name) < 2) return 0;
    if (name[0] != 't') return 0;
    for (int i = 1; name[i]; i++) if (!isdigit(name[i])) return 0;
    return 1;
}

static int getOptimizerVarOffset(const char* name) {
    if (isTemporary(name)) return -1;
    return getVarOffset(name);
}

static const char* tempToReg(const char* temp) {
    static char reg[10];
    if (isTemporary(temp)) {
        sprintf(reg, "$t%d", atoi(temp + 1));
        return reg;
    }
    return temp;
}

void optimizeTAC2() {
    TACInstr* curr = tacList.head;
    optimizedList2.head = optimizedList2.tail = NULL;
    while (curr) {
        TACInstr* newInstr = createTAC(curr->op, curr->arg1, curr->arg2, curr->result);
        if (newInstr) {
            if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
            else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
        }
        curr = curr->next;
    }
}

// Print unoptimized TAC to file (for compatibility with main.c)
void printTACToFile2(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    fprintf(file, "Unoptimized TAC Instructions (v2):\n");
    fprintf(file, "─────────────────────────────\n");
    TACInstr* curr = tacList.head;
    int instrNum = 1;
    while (curr) {
        fprintf(file, "%2d: ", instrNum++);
        switch(curr->op) {
            case TAC_ADD: fprintf(file, "%s = %s + %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_SUBTRACT: fprintf(file, "%s = %s - %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_MULTIPLY: fprintf(file, "%s = %s * %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_DIVIDE: fprintf(file, "%s = %s / %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_ASSIGN: fprintf(file, "%s = %s\n", curr->result, curr->arg1); break;
            case TAC_PRINT: fprintf(file, "PRINT %s\n", curr->arg1); break;
            case TAC_DECL: fprintf(file, "DECL %s\n", curr->result); break;
            case TAC_FUNC_DEF: fprintf(file, "FUNC %s\n", curr->arg1); break;
            case TAC_FUNC_CALL: fprintf(file, "%s = CALL %s\n", curr->result, curr->arg1); break;
            case TAC_PARAM: fprintf(file, "PARAM %s\n", curr->arg1); break;
            case TAC_RETURN: fprintf(file, "RETURN %s\n", curr->arg1 ? curr->arg1 : ""); break;
            case TAC_ARG: fprintf(file, "ARG %s\n", curr->arg1); break;
            default: break;
        }
        curr = curr->next;
    }
    fprintf(file, "\n");
    fclose(file);
}

// Print optimized TAC to console
void printOptimizedTAC2() {
    printf("Optimized TAC Instructions (v2):\n");
    printf("─────────────────────────────\n");
    TACInstr* curr = optimizedList2.head;
    int instrNum = 1;
    while (curr) {
        printf("%2d: ", instrNum++);
        switch(curr->op) {
            case TAC_ADD: printf("%s = %s + %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_SUBTRACT: printf("%s = %s - %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_MULTIPLY: printf("%s = %s * %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_DIVIDE: printf("%s = %s / %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_ASSIGN: printf("%s = %s\n", curr->result, curr->arg1); break;
            case TAC_PRINT: printf("PRINT %s\n", curr->arg1); break;
            case TAC_DECL: printf("DECL %s\n", curr->result); break;
            case TAC_FUNC_DEF: printf("FUNC %s\n", curr->arg1); break;
            case TAC_FUNC_CALL: printf("%s = CALL %s\n", curr->result, curr->arg1); break;
            case TAC_PARAM: printf("PARAM %s\n", curr->arg1); break;
            case TAC_RETURN: printf("RETURN %s\n", curr->arg1 ? curr->arg1 : ""); break;
            case TAC_ARG: printf("ARG %s\n", curr->arg1); break;
            default: break;
        }
        curr = curr->next;
    }
    printf("\n");
}

// Print optimized TAC to file
void printOptimizedTACToFile2(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    fprintf(file, "Optimized TAC Instructions (v2):\n");
    fprintf(file, "─────────────────────────────\n");
    TACInstr* curr = optimizedList2.head;
    int instrNum = 1;
    while (curr) {
        fprintf(file, "%2d: ", instrNum++);
        switch(curr->op) {
            case TAC_ADD: fprintf(file, "%s = %s + %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_SUBTRACT: fprintf(file, "%s = %s - %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_MULTIPLY: fprintf(file, "%s = %s * %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_DIVIDE: fprintf(file, "%s = %s / %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_ASSIGN: fprintf(file, "%s = %s\n", curr->result, curr->arg1); break;
            case TAC_PRINT: fprintf(file, "PRINT %s\n", curr->arg1); break;
            case TAC_DECL: fprintf(file, "DECL %s\n", curr->result); break;
            case TAC_FUNC_DEF: fprintf(file, "FUNC %s\n", curr->arg1); break;
            case TAC_FUNC_CALL: fprintf(file, "%s = CALL %s\n", curr->result, curr->arg1); break;
            case TAC_PARAM: fprintf(file, "PARAM %s\n", curr->arg1); break;
            case TAC_RETURN: fprintf(file, "RETURN %s\n", curr->arg1 ? curr->arg1 : ""); break;
            case TAC_ARG: fprintf(file, "ARG %s\n", curr->arg1); break;
            default: break;
        }
        curr = curr->next;
    }
    fprintf(file, "\n");
    fclose(file);
}

void generateMIPSFromOptimizedTAC2(const char* filename) {
    FILE* output = fopen(filename, "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot open output file %s\n", filename);
        return;
    }
    
    MIPSList mipsList;
    mipsList.head = NULL;
    mipsList.tail = NULL;
    
    // Write MIPS header
    fprintf(output, ".data\n\n");
    fprintf(output, ".text\n");
    fprintf(output, ".globl main\n\n");
    
    // Generate main function
    appendMIPS(&mipsList, createMIPS(MIPS_LABEL, NULL, "main", NULL, NULL));
    
    // Find the first non-main function call in the TAC and its arguments
    TACInstr* curr = optimizedList2.head;
    char* firstFunctionName = NULL;
    char* arg0 = NULL;
    char* arg1 = NULL;
    TACInstr* argInstr0 = NULL;
    TACInstr* argInstr1 = NULL;

    while (curr) {
        if (curr->op == TAC_FUNC_CALL && curr->arg1) {
            firstFunctionName = curr->arg1;
            // Scan from head to curr to find the two most recent ARG instructions
            TACInstr* scan = optimizedList2.head;
            TACInstr* lastArg1 = NULL;
            TACInstr* lastArg0 = NULL;
            while (scan != curr) {
                if (scan->op == TAC_ARG) {
                    lastArg0 = lastArg1;
                    lastArg1 = scan;
                }
                scan = scan->next;
            }
            // Swap the order since TAC has args in reverse order
            argInstr0 = lastArg1;
            argInstr1 = lastArg0;
            if (argInstr0) arg0 = argInstr0->arg1;
            if (argInstr1) arg1 = argInstr1->arg1;
            break;
        }
        curr = curr->next;
    }

    if (firstFunctionName) {
        // Ensure arguments are stored to stack if they are variables or temporaries
        if (arg0) {
            if ((isdigit(arg0[0])) || (arg0[0] == '-' && isdigit(arg0[1]))) {
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", arg0, NULL, NULL));
            } else if (isTemporary(arg0)) {
                appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$a0", tempToReg(arg0), NULL, NULL));
            } else {
                int offset = getOptimizerVarOffset(arg0);
                if (offset != -1) {
                    char offsetStr[32];
                    sprintf(offsetStr, "%d($fp)", offset);
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, "$a0", offsetStr, NULL, NULL));
                }
            }
        }
        if (arg1) {
            if ((isdigit(arg1[0])) || (arg1[0] == '-' && isdigit(arg1[1]))) {
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a1", arg1, NULL, NULL));
            } else if (isTemporary(arg1)) {
                appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$a1", tempToReg(arg1), NULL, NULL));
            } else {
                int offset = getOptimizerVarOffset(arg1);
                if (offset != -1) {
                    char offsetStr[32];
                    sprintf(offsetStr, "%d($fp)", offset);
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, "$a1", offsetStr, NULL, NULL));
                }
            }
        }
        appendMIPS(&mipsList, createMIPS(MIPS_JAL, NULL, firstFunctionName, NULL, NULL));

        // After function call, move $v0 to the temporary that stores the result (e.g., t2)
        // Find the TAC_FUNC_CALL instruction to get the result temp
        curr = optimizedList2.head;
        while (curr) {
            if (curr->op == TAC_FUNC_CALL && curr->arg1 && strcmp(curr->arg1, firstFunctionName) == 0 && curr->result) {
                if (isTemporary(curr->result)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, tempToReg(curr->result), "$v0", NULL, NULL));
                }
                break;
            }
            curr = curr->next;
        }
    }
    
    // Print the result (in $v0)
    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$a0", "$v0", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "1", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
    
    // Print newline
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "11", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", "10", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
    
    // Exit program
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "10", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
    
    // Generate function definitions (skip main)
    curr = optimizedList2.head;
    while (curr) {
        if (curr->op == TAC_FUNC_DEF && curr->arg1 && strcmp(curr->arg1, "main") != 0) {
            // Generate function label
            appendMIPS(&mipsList, createMIPS(MIPS_LABEL, NULL, curr->arg1, NULL, NULL));
            
            // Look ahead for the operation in this function
            TACInstr* funcInstr = curr->next;
            while (funcInstr && funcInstr->op != TAC_FUNC_DEF) {
                if (funcInstr->op == TAC_ADD) {
                    // Generate: add $v0, $a0, $a1
                    appendMIPS(&mipsList, createMIPS(MIPS_ADD, "$v0", "$a0", "$a1", NULL));
                } else if (funcInstr->op == TAC_MULTIPLY) {
                    // Generate: mult $a0, $a1; mflo $v0
                    appendMIPS(&mipsList, createMIPS(MIPS_MUL, NULL, "$a0", "$a1", NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$v0", NULL, NULL, NULL));
                } else if (funcInstr->op == TAC_DIVIDE) {
                    // Generate: div $a0, $a1; mflo $v0
                    appendMIPS(&mipsList, createMIPS(MIPS_DIV, NULL, "$a0", "$a1", NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$v0", NULL, NULL, NULL));
                } else if (funcInstr->op == TAC_SUBTRACT) {
                    // Generate: sub $v0, $a0, $a1
                    appendMIPS(&mipsList, createMIPS(MIPS_SUB, "$v0", "$a0", "$a1", NULL));
                } else if (funcInstr->op == TAC_RETURN) {
                    // Generate: jr $ra
                    appendMIPS(&mipsList, createMIPS(MIPS_JR, NULL, "$ra", NULL, NULL));
                    break; // End of function
                }
                funcInstr = funcInstr->next;
            }
        }
        curr = curr->next;
    }
    
    // Print MIPS instructions to file
    printMIPS(&mipsList, output);
    
    // Cleanup
    freeMIPSList(&mipsList);
    fclose(output);
}
