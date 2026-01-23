#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "optimizer.h"
#include "mips.h"
#include "symtab.h"
#include "tac.h"

/* External declarations */
extern TACList tacList;
TACList optimizedList;

// External function declaration for isConst from tac.c
extern int isConst(const char* s);

void optimizeTAC() {
    TACInstr* curr = tacList.head;
    
    // Track which variables are actually assigned (not just declared)
    char* assignedVars[100];
    int assignedCount = 0;
    
    // First pass: identify variables that get assigned
    TACInstr* scan = tacList.head;
    while (scan) {
        if (scan->op == TAC_ASSIGN) {
            // Check if already tracked
            int found = 0;
            for (int i = 0; i < assignedCount; i++) {
                if (strcmp(assignedVars[i], scan->result) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                assignedVars[assignedCount++] = strdup(scan->result);
            }
        }
        scan = scan->next;
    }
    
    // Copy propagation table
    typedef struct {
        char* var;
        char* value;
    } VarValue;
    
    VarValue values[100];
    int valueCount = 0;
    
    while (curr) {
        TACInstr* newInstr = NULL;
        
        switch(curr->op) {
            case TAC_DECL: {
                // Only emit DECL if variable is actually assigned (dead code elimination)
                int isAssigned = 0;
                for (int i = 0; i < assignedCount; i++) {
                    if (strcmp(assignedVars[i], curr->result) == 0) {
                        isAssigned = 1;
                        break;
                    }
                }
                if (isAssigned) {
                    newInstr = createTAC(TAC_DECL, NULL, NULL, curr->result);
                }
                // Otherwise, dead code - don't emit
                break;
            }
                
            case TAC_ADD: {
                // Check if both operands are constants
                char* left = curr->arg1;
                char* right = curr->arg2;
                
                // Look up values in propagation table (search from most recent)
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, left) == 0) {
                        left = values[i].value;
                        break;
                    }
                }
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, right) == 0) {
                        right = values[i].value;
                        break;
                    }
                }
                
                // Constant folding
                if (isdigit(left[0]) && isdigit(right[0])) {
                    int result = atoi(left) + atoi(right);
                    char* resultStr = malloc(20);
                    sprintf(resultStr, "%d", result);
                    
                    // Store for propagation
                    values[valueCount].var = strdup(curr->result);
                    values[valueCount].value = resultStr;
                    valueCount++;
                    
                    newInstr = createTAC(TAC_ASSIGN, resultStr, NULL, curr->result);
                } else {
                    newInstr = createTAC(TAC_ADD, left, right, curr->result);
                }
                break;
            }
            case TAC_SUBTRACT: {
                char* left = curr->arg1;
                char* right = curr->arg2;

                /* Look up propagated values (most recent wins) */
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, left) == 0) {
                        left = values[i].value;
                        break;
                    }
                }
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, right) == 0) {
                        right = values[i].value;
                        break;
                    }
                }

                /* Constant folding */
                if ((isdigit(left[0]) || left[0] == '-') &&
                    (isdigit(right[0]) || right[0] == '-')) {

                    int result = atoi(left) - atoi(right);
                    char* resultStr = malloc(20);
                    sprintf(resultStr, "%d", result);

                    /* Store for propagation */
                    values[valueCount].var = strdup(curr->result);
                    values[valueCount].value = resultStr;
                    valueCount++;

                    newInstr = createTAC(TAC_ASSIGN, resultStr, NULL, curr->result);
                } else {
                    newInstr = createTAC(TAC_SUBTRACT, left, right, curr->result);
                }
                break;
            }
            case TAC_MULTIPLY: {
                char* left = curr->arg1;
                char* right = curr->arg2;

                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, left) == 0) {
                        left = values[i].value;
                        break;
                    }
                }
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, right) == 0) {
                        right = values[i].value;
                        break;
                    }
                }

                if (isConst(left) && isConst(right)) {
                    double result = atof(left) * atof(right);
                    char* resultStr = malloc(32);

                    // Preserve float formatting
                    sprintf(resultStr, "%.6f", result);

                    values[valueCount].var = strdup(curr->result);
                    values[valueCount].value = resultStr;
                    valueCount++;

                    newInstr = createTAC(TAC_ASSIGN, resultStr, NULL, curr->result);
                } else {
                    newInstr = createTAC(TAC_MULTIPLY, left, right, curr->result);
                }
                break;
            }
            case TAC_DIVIDE: {
                char* left = curr->arg1;
                char* right = curr->arg2;

                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, left) == 0) {
                        left = values[i].value;
                        break;
                    }
                }
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, right) == 0) {
                        right = values[i].value;
                        break;
                    }
                }

                if (isConst(left) && isConst(right)) {
                    double result = atof(left) / atof(right);
                    char* resultStr = malloc(20);
                    sprintf(resultStr, "%.6f", result);

                    values[valueCount].var = strdup(curr->result);
                    values[valueCount].value = resultStr;
                    valueCount++;

                    newInstr = createTAC(TAC_ASSIGN, resultStr, NULL, curr->result);
                } else {
                    newInstr = createTAC(TAC_DIVIDE, left, right, curr->result);
                }
                break;
            }
            case TAC_ASSIGN: {
                char* value = curr->arg1;
                
                // Look up value in propagation table (search from most recent)
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, value) == 0) {
                        value = values[i].value;
                        break;
                    }
                }
                
                // Store for propagation
                values[valueCount].var = strdup(curr->result);
                values[valueCount].value = strdup(value);
                valueCount++;
                
                newInstr = createTAC(TAC_ASSIGN, value, NULL, curr->result);
                break;
            }
            case TAC_PRINT: {
                char* value = curr->arg1;
                
                // Look up value in propagation table
                for (int i = valueCount - 1; i >= 0; i--) {  // Search from most recent
                    if (strcmp(values[i].var, value) == 0) {
                        value = values[i].value;
                        break;
                    }
                }
                
                newInstr = createTAC(TAC_PRINT, value, NULL, NULL);
                break;
            }
        }
        
        if (newInstr) {
            appendOptimizedTAC(newInstr);
        }
        
        curr = curr->next;
    }
}

void printTACToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "Unoptimized TAC Instructions:\n");
    fprintf(file, "─────────────────────────────\n");
    TACInstr* curr = tacList.head;
    int lineNum = 1;
    while (curr) {
        fprintf(file, "%2d: ", lineNum++);
        switch(curr->op) {
            case TAC_DECL:
                fprintf(file, "DECL %s", curr->result);
                fprintf(file, "          // Declare variable '%s'\n", curr->result);
                break;
            case TAC_ADD:
                fprintf(file, "%s = %s + %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Add: store result in %s\n", curr->result);
                break;
            case TAC_SUBTRACT:
                fprintf(file, "%s = %s - %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Subtract: store result in %s\n", curr->result);
                break;
            case TAC_MULTIPLY:
                fprintf(file, "%s = %s * %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Multiply: store result in %s\n", curr->result);
                break;
            case TAC_DIVIDE:
                fprintf(file, "%s = %s / %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Divide: store result in %s\n", curr->result);
                break;
            case TAC_ASSIGN:
                fprintf(file, "%s = %s", curr->result, curr->arg1);
                fprintf(file, "           // Assign value to %s\n", curr->result);
                break;
            case TAC_PRINT:
                fprintf(file, "PRINT %s", curr->arg1);
                fprintf(file, "          // Output value of %s\n", curr->arg1);
                break;
            default:
                break;
        }
        curr = curr->next;
    }
    
    fclose(file);
}

void printOptimizedTAC() {
    printf("Optimized TAC Instructions:\n");
    printf("─────────────────────────────\n");
    TACInstr* curr = optimizedList.head;
    int lineNum = 1;
    while (curr) {
        printf("%2d: ", lineNum++);
        switch(curr->op) {
            case TAC_DECL:
                printf("DECL %s\n", curr->result);
                break;
            case TAC_ADD:
                printf("%s = %s + %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Runtime addition needed\n");
                break;
            case TAC_SUBTRACT:
                printf("%s = %s - %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Runtime subtraction needed\n");
                break;
            case TAC_MULTIPLY:
                printf("%s = %s * %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Runtime multiplication needed\n");
                break;
            case TAC_DIVIDE:
                printf("%s = %s / %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Runtime division needed\n");
                break;
            case TAC_ASSIGN:
                printf("%s = %s", curr->result, curr->arg1);
                // Check if it's a constant
                if (curr->arg1[0] >= '0' && curr->arg1[0] <= '9') {
                    printf("           // Constant value: %s\n", curr->arg1);
                } else {
                    printf("           // Copy value\n");
                }
                break;
            case TAC_PRINT:
                printf("PRINT %s", curr->arg1);
                // Check if it's a constant
                if (curr->arg1[0] >= '0' && curr->arg1[0] <= '9') {
                    printf("          // Print constant: %s\n", curr->arg1);
                } else {
                    printf("          // Print variable\n");
                }
                break;
            default:
                break;
        }
        curr = curr->next;
    }
}

void printOptimizedTACToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "Optimized TAC Instructions:\n");
    fprintf(file, "─────────────────────────────\n");
    TACInstr* curr = optimizedList.head;
    int lineNum = 1;
    while (curr) {
        fprintf(file, "%2d: ", lineNum++);
        switch(curr->op) {
            case TAC_DECL:
                fprintf(file, "DECL %s\n", curr->result);
                break;
            case TAC_ADD:
                fprintf(file, "%s = %s + %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Runtime addition needed\n");
                break;
            case TAC_SUBTRACT:
                fprintf(file, "%s = %s - %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Runtime subtraction needed\n");
                break;
            case TAC_MULTIPLY:
                fprintf(file, "%s = %s * %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Runtime multiplication needed\n");
                break;
            case TAC_DIVIDE:
                fprintf(file, "%s = %s / %s", curr->result, curr->arg1, curr->arg2);
                fprintf(file, "     // Runtime division needed\n");
                break;
            case TAC_ASSIGN:
                fprintf(file, "%s = %s", curr->result, curr->arg1);
                // Check if it's a constant
                if (curr->arg1[0] >= '0' && curr->arg1[0] <= '9') {
                    fprintf(file, "           // Constant value: %s\n", curr->arg1);
                } else {
                    fprintf(file, "           // Copy value\n");
                }
                break;
            case TAC_PRINT:
                fprintf(file, "PRINT %s", curr->arg1);
                // Check if it's a constant
                if (curr->arg1[0] >= '0' && curr->arg1[0] <= '9') {
                    fprintf(file, "          // Print constant: %s\n", curr->arg1);
                } else {
                    fprintf(file, "          // Print variable\n");
                }
                break;
            default:
                break;
        }
        curr = curr->next;
    }
}

// Register allocation for MIPS generation
typedef struct {
    int tempRegs[8];     // $t0-$t7
    int floatRegs[32];   // $f0-$f31
    int tempNext;
    int floatNext;
} OptimizerRegAllocator;

void initOptimizerRegAllocator(OptimizerRegAllocator* ra) {
    for (int i = 0; i < 8; i++) {
        ra->tempRegs[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        ra->floatRegs[i] = 0;
    }
    ra->tempNext = 0;
    ra->floatNext = 0;
}

int allocateOptimizerTempReg(OptimizerRegAllocator* ra) {
    for (int i = 0; i < 8; i++) {
        if (ra->tempRegs[i] == 0) {
            ra->tempRegs[i] = 1;
            return i;
        }
    }
    return 0; // fallback
}

int allocateOptimizerFloatReg(OptimizerRegAllocator* ra) {
    for (int i = 0; i < 32; i++) {
        if (ra->floatRegs[i] == 0) {
            ra->floatRegs[i] = 1;
            return i;
        }
    }
    return 0; // fallback
}

void releaseOptimizerTempReg(OptimizerRegAllocator* ra, int reg) {
    if (reg >= 0 && reg < 8) {
        ra->tempRegs[reg] = 0;
    }
}

void releaseOptimizerFloatReg(OptimizerRegAllocator* ra, int reg) {
    if (reg >= 0 && reg < 32) {
        ra->floatRegs[reg] = 0;
    }
}

// Generate MIPS from optimized TAC
void generateMIPSFromOptimizedTAC(const char* filename) {
    FILE* output = fopen(filename, "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot open output file %s\n", filename);
        return;
    }
    
    MIPSList mipsList;
    mipsList.head = NULL;
    mipsList.tail = NULL;
    
    OptimizerRegAllocator ra;
    initOptimizerRegAllocator(&ra);
    
    // Initialize symbol table for variable tracking
    initSymTab();
    
    // First pass: collect variable declarations
    TACInstr* curr = optimizedList.head;
    while (curr) {
        if (curr->op == TAC_DECL) {
            addVar(curr->result, TYPE_INT); // Default to int, could be enhanced
        }
        curr = curr->next;
    }
    
    // MIPS header
    fprintf(output, ".data\n");
    fprintf(output, "\n.text\n");
    fprintf(output, ".globl main\n");
    fprintf(output, "main:\n");
    
    // Allocate stack space
    appendMIPS(&mipsList, createMIPS(MIPS_COMMENT, NULL, NULL, NULL, "Allocate stack space"));
    appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$sp", "$sp", "-400", NULL));
    
    // Generate MIPS instructions from optimized TAC
    curr = optimizedList.head;
    while (curr) {
        switch(curr->op) {
            case TAC_DECL: {
                char comment[100];
                sprintf(comment, "Declared %s", curr->result);
                appendMIPS(&mipsList, createMIPS(MIPS_COMMENT, NULL, NULL, NULL, comment));
                break;
            }
            
            case TAC_ASSIGN: {
                if (isConst(curr->arg1)) {
                    // Direct constant assignment
                    char reg[10];
                    int regNum = allocateOptimizerTempReg(&ra);
                    sprintf(reg, "$t%d", regNum);
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, reg, curr->arg1, NULL, NULL));
                    
                    // Store to stack
                    int offset = getVarOffset(curr->result);
                    char offsetStr[20];
                    sprintf(offsetStr, "%d($sp)", offset);
                    appendMIPS(&mipsList, createMIPS(MIPS_SW, reg, offsetStr, NULL, NULL));
                    
                    releaseOptimizerTempReg(&ra, regNum);
                } else {
                    // Variable assignment (copy propagation already handled)
                    int srcOffset = getVarOffset(curr->arg1);
                    int dstOffset = getVarOffset(curr->result);
                    
                    int regNum = allocateOptimizerTempReg(&ra);
                    char reg[10];
                    sprintf(reg, "$t%d", regNum);
                    
                    char srcOffsetStr[20], dstOffsetStr[20];
                    sprintf(srcOffsetStr, "%d($sp)", srcOffset);
                    sprintf(dstOffsetStr, "%d($sp)", dstOffset);
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, reg, srcOffsetStr, NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_SW, reg, dstOffsetStr, NULL, NULL));
                    
                    releaseOptimizerTempReg(&ra, regNum);
                }
                break;
            }
            
            case TAC_ADD:
            case TAC_SUBTRACT:
            case TAC_MULTIPLY:
            case TAC_DIVIDE: {
                // Binary operations
                int reg1 = allocateOptimizerTempReg(&ra);
                int reg2 = allocateOptimizerTempReg(&ra);
                int reg3 = allocateOptimizerTempReg(&ra);
                
                char r1[10], r2[10], r3[10];
                sprintf(r1, "$t%d", reg1);
                sprintf(r2, "$t%d", reg2);
                sprintf(r3, "$t%d", reg3);
                
                // Load operands
                if (isConst(curr->arg1)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, r1, curr->arg1, NULL, NULL));
                } else {
                    int offset = getVarOffset(curr->arg1);
                    char offsetStr[20];
                    sprintf(offsetStr, "%d($sp)", offset);
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, r1, offsetStr, NULL, NULL));
                }
                
                if (isConst(curr->arg2)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, r2, curr->arg2, NULL, NULL));
                } else {
                    int offset = getVarOffset(curr->arg2);
                    char offsetStr[20];
                    sprintf(offsetStr, "%d($sp)", offset);
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, r2, offsetStr, NULL, NULL));
                }
                
                // Perform operation
                switch(curr->op) {
                    case TAC_ADD:
                        appendMIPS(&mipsList, createMIPS(MIPS_ADD, r3, r1, r2, NULL));
                        break;
                    case TAC_SUBTRACT:
                        appendMIPS(&mipsList, createMIPS(MIPS_SUB, r3, r1, r2, NULL));
                        break;
                    case TAC_MULTIPLY:
                        appendMIPS(&mipsList, createMIPS(MIPS_MUL, NULL, r1, r2, NULL));
                        appendMIPS(&mipsList, createMIPS(MIPS_MFLO, r3, NULL, NULL, NULL));
                        break;
                    case TAC_DIVIDE:
                        appendMIPS(&mipsList, createMIPS(MIPS_DIV, NULL, r1, r2, NULL));
                        appendMIPS(&mipsList, createMIPS(MIPS_MFLO, r3, NULL, NULL, NULL));
                        break;
                    default:
                        break; // Handle TAC_ASSIGN, TAC_PRINT, TAC_DECL
                }
                
                // Store result
                int offset = getVarOffset(curr->result);
                char offsetStr[20];
                sprintf(offsetStr, "%d($sp)", offset);
                appendMIPS(&mipsList, createMIPS(MIPS_SW, r3, offsetStr, NULL, NULL));
                
                releaseOptimizerTempReg(&ra, reg1);
                releaseOptimizerTempReg(&ra, reg2);
                releaseOptimizerTempReg(&ra, reg3);
                break;
            }
            
            case TAC_PRINT: {
                if (isConst(curr->arg1)) {
                    // Print constant directly
                    int regNum = allocateOptimizerTempReg(&ra);
                    char reg[10];
                    sprintf(reg, "$t%d", regNum);
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, reg, curr->arg1, NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$a0", reg, NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "1", NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
                    
                    releaseOptimizerTempReg(&ra, regNum);
                } else {
                    // Print variable
                    int offset = getVarOffset(curr->arg1);
                    int regNum = allocateOptimizerTempReg(&ra);
                    char reg[10];
                    sprintf(reg, "$t%d", regNum);
                    
                    char offsetStr[20];
                    sprintf(offsetStr, "%d($sp)", offset);
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, reg, offsetStr, NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$a0", reg, NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "1", NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
                    
                    releaseOptimizerTempReg(&ra, regNum);
                }
                
                // Print newline
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "11", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", "10", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
                break;
            }
        }
        curr = curr->next;
    }
    
    // Program exit
    appendMIPS(&mipsList, createMIPS(MIPS_COMMENT, NULL, NULL, NULL, "Exit program"));
    appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$sp", "$sp", "400", NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "10", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
    
    // Print MIPS instructions to file
    printMIPS(&mipsList, output);
    
    // Cleanup
    freeMIPSList(&mipsList);
    fclose(output);
}
