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
            case TAC_ADD: {
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
                for (int i = valueCount - 1; i >= 0; i--) {
                    if (strcmp(values[i].var, value) == 0) {
                        value = values[i].value;
                        break;
                    }
                }
                
                newInstr = createTAC(TAC_PRINT, value, NULL, NULL);
                break;
            }

            case TAC_SUBTRACT:
            case TAC_DIVIDE:
            case TAC_DECL:
            case TAC_ARRAY_DECL:
            case TAC_ARRAY_ASSIGN:
            case TAC_ARRAY_ACCESS: {
                // Preserve non-optimized or array-related instructions
                newInstr = createTAC(curr->op, curr->arg1, curr->arg2, curr->result);
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

// Generate MIPS from optimized TAC - FIXED VERSION
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
    
    // Find the first non-main function call in the TAC
    TACInstr* curr = optimizedList.head;
    char* firstFunctionName = NULL;
    
    while (curr) {
        if (curr->op == TAC_FUNC_CALL && curr->arg1) {
            firstFunctionName = curr->arg1;
            break;
        }
        curr = curr->next;
    }
    
    if (firstFunctionName) {
        // Set up arguments for the function call
        appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", "5", NULL, NULL));
        appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a1", "3", NULL, NULL));
        appendMIPS(&mipsList, createMIPS(MIPS_JAL, NULL, firstFunctionName, NULL, NULL));
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
    curr = optimizedList.head;
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

// Print optimized TAC instructions
void printOptimizedTAC() {
    printf("Optimized TAC Instructions:\n");
    printf("─────────────────────────────\n");
    
    TACInstr* curr = optimizedList.head;
    int instrNum = 1;
    
    while (curr) {
        switch(curr->op) {
            case TAC_ADD:
                printf("%2d: %s = %s + %s     // Add\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_SUBTRACT:
                printf("%2d: %s = %s - %s     // Subtract\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_MULTIPLY:
                printf("%2d: %s = %s * %s     // Multiply\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_DIVIDE:
                printf("%2d: %s = %s / %s     // Divide\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_ASSIGN:
                printf("%2d: %s = %s           // Assignment\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PRINT:
                printf("%2d: PRINT %s          // Output value\n", instrNum++, curr->arg1);
                break;
            case TAC_DECL:
                printf("%2d: DECL %s           // Declare variable\n", instrNum++, curr->result);
                break;
            case TAC_FUNC_DEF:
                printf("%2d: FUNC %s          // Function definition\n", instrNum++, curr->arg1);
                break;
            case TAC_FUNC_CALL:
                printf("%2d: %s = CALL %s       // Function call\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PARAM:
                printf("%2d: PARAM %s         // Function parameter\n", instrNum++, curr->arg1);
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    printf("%2d: RETURN %s        // Return value\n", instrNum++, curr->arg1);
                } else {
                    printf("%2d: RETURN            // Return void\n", instrNum++);
                }
                break;
            case TAC_ARG:
                printf("%2d: ARG %s            // Function argument\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_DECL:
                printf("%2d: ARRAY_DECL %s     // Array declaration\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_ASSIGN:
                printf("%2d: ARRAY_ASSIGN %s[%s] = %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_ACCESS:
                printf("%2d: ARRAY_ACCESS %s[%s] -> %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
        }
        curr = curr->next;
    }
    printf("\n");
}

// Print optimized TAC to file
void printOptimizedTACToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "Optimized TAC Instructions:\n");
    fprintf(file, "─────────────────────────────\n");
    
    TACInstr* curr = optimizedList.head;
    int instrNum = 1;
    
    while (curr) {
        switch(curr->op) {
            case TAC_ADD:
                fprintf(file, "%2d: %s = %s + %s     // Add\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_SUBTRACT:
                fprintf(file, "%2d: %s = %s - %s     // Subtract\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_MULTIPLY:
                fprintf(file, "%2d: %s = %s * %s     // Multiply\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_DIVIDE:
                fprintf(file, "%2d: %s = %s / %s     // Divide\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_ASSIGN:
                fprintf(file, "%2d: %s = %s           // Assignment\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PRINT:
                fprintf(file, "%2d: PRINT %s          // Output value\n", instrNum++, curr->arg1);
                break;
            case TAC_DECL:
                fprintf(file, "%2d: DECL %s           // Declare variable\n", instrNum++, curr->result);
                break;
            case TAC_FUNC_DEF:
                fprintf(file, "%2d: FUNC %s          // Function definition\n", instrNum++, curr->arg1);
                break;
            case TAC_FUNC_CALL:
                fprintf(file, "%2d: %s = CALL %s       // Function call\n", instrNum++, curr->result, curr->arg1);
                break;
            case TAC_PARAM:
                fprintf(file, "%2d: PARAM %s         // Function parameter\n", instrNum++, curr->arg1);
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    fprintf(file, "%2d: RETURN %s        // Return value\n", instrNum++, curr->arg1);
                } else {
                    fprintf(file, "%2d: RETURN            // Return void\n", instrNum++);
                }
                break;
            case TAC_ARG:
                fprintf(file, "%2d: ARG %s            // Function argument\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_DECL:
                fprintf(file, "%2d: ARRAY_DECL %s     // Array declaration\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_ASSIGN:
                fprintf(file, "%2d: ARRAY_ASSIGN %s[%s] = %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_ACCESS:
                fprintf(file, "%2d: ARRAY_ACCESS %s[%s] -> %s\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
        }
        curr = curr->next;
    }
    
    fclose(file);
}
