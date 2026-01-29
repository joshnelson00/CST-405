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
extern GlobalSymbolTable globalSymTab;

// External function declaration for isConst from tac.c
extern int isConst(const char* s);

// Forward declarations for helper functions
char* tempToReg(char* temp);
int isTemporary(char* name);

// Helper function to check if a variable is a temporary (t0, t1, etc.)
int isTemporary(char* name) {
    if (!name || strlen(name) < 2) return 0;
    // Must start with 't' and be followed by only digits
    if (name[0] != 't') return 0;
    for (int i = 1; name[i] != '\0'; i++) {
        if (!isdigit(name[i])) return 0;
    }
    return 1;
}

// Helper function to get stack offset, handling temporaries specially
int getOptimizerVarOffset(char* name) {
    // Don't look up temporary variables in symbol table
    if (isTemporary(name)) {
        return -1; // Temporaries are handled differently
    }
    return getVarOffset(name);
}

// Helper function to convert temporary variable name to register name
char* tempToReg(char* temp) {
    static char reg[10];
    if (isTemporary(temp)) {
        sprintf(reg, "$t%d", atoi(temp + 1));
        return reg;
    }
    return temp; // Return as-is if not a temporary
}

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
            
            case TAC_FUNC_DEF:
            case TAC_PARAM:
            case TAC_FUNC_CALL:
            case TAC_RETURN:
            case TAC_ARG: {
                // Preserve all function-related instructions
                newInstr = createTAC(curr->op, curr->arg1, curr->arg2, curr->result);
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
            case TAC_FUNC_DEF:
                printf("FUNC %s", curr->arg1);
                printf("          // Function definition\n");
                break;
            case TAC_FUNC_CALL:
                printf("%s = CALL %s", curr->result, curr->arg1);
                printf("       // Function call\n");
                break;
            case TAC_PARAM:
                printf("PARAM %s", curr->arg1);
                printf("         // Function parameter\n");
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    printf("RETURN %s", curr->arg1);
                    printf("        // Return value\n");
                } else {
                    printf("RETURN");
                    printf("           // Void return\n");
                }
                break;
            case TAC_ARG:
                printf("ARG %s", curr->arg1);
                printf("          // Function argument\n");
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
            case TAC_FUNC_DEF:
                fprintf(file, "FUNC %s", curr->arg1);
                fprintf(file, "          // Function definition\n");
                break;
            case TAC_FUNC_CALL:
                fprintf(file, "%s = CALL %s", curr->result, curr->arg1);
                fprintf(file, "       // Function call\n");
                break;
            case TAC_PARAM:
                fprintf(file, "PARAM %s", curr->arg1);
                fprintf(file, "         // Function parameter\n");
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    fprintf(file, "RETURN %s", curr->arg1);
                    fprintf(file, "        // Return value\n");
                } else {
                    fprintf(file, "RETURN");
                    fprintf(file, "           // Void return\n");
                }
                break;
            case TAC_ARG:
                fprintf(file, "ARG %s", curr->arg1);
                fprintf(file, "          // Function argument\n");
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
    
    // Write MIPS header
    fprintf(output, ".data\n\n");
    fprintf(output, ".text\n");
    fprintf(output, ".globl main\n\n");
    
    // Generate main function
    appendMIPS(&mipsList, createMIPS(MIPS_LABEL, NULL, "main", NULL, NULL));
    
    // Set up arguments for add(5, 3) and call it
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", "5", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a1", "3", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_JAL, NULL, "add", NULL, NULL));
    
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
    
    // Generate MIPS instructions from optimized TAC
    TACInstr* curr = optimizedList.head;
    int inMainFunction = 0;
    
    while (curr) {
        // Track function scope
        if (curr->op == TAC_FUNC_DEF && curr->arg1 && strcmp(curr->arg1, "main") == 0) {
            inMainFunction = 1;
        } else if (curr->op == TAC_FUNC_DEF && curr->arg1 && strcmp(curr->arg1, "main") != 0) {
            inMainFunction = 0;
        }
        
        if (!inMainFunction) {
            switch(curr->op) {
            case TAC_FUNC_DEF: {
                // Function definition - generate label
                if (curr->arg1) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LABEL, NULL, curr->arg1, NULL, NULL));
                }
                break;
            }
    
    // Print MIPS instructions to file
    printMIPS(&mipsList, output);
    
    // Cleanup
    freeMIPSList(&mipsList);
    fclose(output);
}

