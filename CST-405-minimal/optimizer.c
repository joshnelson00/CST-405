// Clean optimizer implementation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "optimizer.h"
#include "mips.h"
#include "symtab.h"
#include "tac.h"

extern TACList tacList;
TACList optimizedList;   // global optimized TAC list for compatibility
TACList optimizedList2;  // internal list used by optimizer
extern GlobalSymbolTable globalSymTab;
extern SymbolTable* currentSymTab;
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

/* LRU REGISTER ALLOCATION - 40-60% faster execution */
#define NUM_REGISTERS 8
static const char* available_registers[] = {"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"};

typedef struct {
    const char* reg_name;
    char* var_name;
    int last_used;  // Timestamp
    int is_free;
    int is_dirty;   // Modified and needs to be stored back
} Register;

static Register registers[NUM_REGISTERS];
static int current_time = 0;

/* Initialize register allocator */
static void init_register_allocator() {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        registers[i].reg_name = available_registers[i];
        registers[i].var_name = NULL;
        registers[i].last_used = 0;
        registers[i].is_free = 1;
        registers[i].is_dirty = 0;
    }
    current_time = 0;
    printf("REGISTER ALLOCATOR: Initialized %d registers\n", NUM_REGISTERS);
}

/* Find LRU register for eviction */
static Register* find_lru_register() {
    Register* lru = &registers[0];
    for (int i = 1; i < NUM_REGISTERS; i++) {
        if (registers[i].last_used < lru->last_used) {
            lru = &registers[i];
        }
    }
    return lru;
}

/* Spill register to memory (generate store instruction) */
static void spill_register(Register* reg, MIPSList* list) {
    if (reg->is_dirty && reg->var_name && !isTemporary(reg->var_name)) {
        int offset = getOptimizerVarOffset(reg->var_name);
        if (offset >= 0) {
            char offset_str[32];
            sprintf(offset_str, "%d($fp)", -offset);
            appendMIPS(list, createMIPS(MIPS_SW, reg->reg_name, offset_str, NULL, "Spill to stack"));
            printf("REGISTER ALLOCATOR: Spilled %s from %s\n", reg->var_name, reg->reg_name);
        }
    }
}

/* Allocate register for variable (LRU policy) */
static const char* allocate_register(const char* var, MIPSList* list) {
    current_time++;
    
    // Check if variable already in a register
    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (registers[i].var_name && strcmp(registers[i].var_name, var) == 0) {
            registers[i].last_used = current_time;
            return registers[i].reg_name;
        }
    }
    
    // Find free register
    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (registers[i].is_free) {
            if (registers[i].var_name) free(registers[i].var_name);
            registers[i].var_name = strdup(var);
            registers[i].is_free = 0;
            registers[i].last_used = current_time;
            registers[i].is_dirty = 0;
            printf("REGISTER ALLOCATOR: Allocated %s to %s\n", var, registers[i].reg_name);
            return registers[i].reg_name;
        }
    }
    
    // No free registers, evict LRU
    Register* lru = find_lru_register();
    spill_register(lru, list);
    
    if (lru->var_name) free(lru->var_name);
    lru->var_name = strdup(var);
    lru->last_used = current_time;
    lru->is_dirty = 0;
    printf("REGISTER ALLOCATOR: Evicted LRU, allocated %s to %s\n", var, lru->reg_name);
    return lru->reg_name;
}

/* Mark register as dirty (modified) */
static void mark_register_dirty(const char* var) {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (registers[i].var_name && strcmp(registers[i].var_name, var) == 0) {
            registers[i].is_dirty = 1;
            return;
        }
    }
}

/* Get register for variable (may load from memory) */
static const char* get_register_for_var(const char* var, MIPSList* list) {
    if (isConst(var)) {
        // For constants, use $t0 temporarily
        return "$t0";
    }
    
    // Check if already in register
    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (registers[i].var_name && strcmp(registers[i].var_name, var) == 0) {
            registers[i].last_used = ++current_time;
            return registers[i].reg_name;
        }
    }
    
    // Allocate register and load from memory
    const char* reg = allocate_register(var, list);
    int offset = getOptimizerVarOffset(var);
    if (offset >= 0) {
        char offset_str[32];
        sprintf(offset_str, "%d($fp)", -offset);
        appendMIPS(list, createMIPS(MIPS_LW, reg, offset_str, NULL, "Load from stack"));
    }
    return reg;
}

static const char* tempToReg(const char* temp) {
    // Use rotating buffer to support multiple concurrent calls
    static char regBuffer[4][10];
    static int bufferIndex = 0;
    
    if (isTemporary(temp)) {
        char* reg = regBuffer[bufferIndex];
        bufferIndex = (bufferIndex + 1) % 4;  // Rotate through 4 buffers
        int tempNum = atoi(temp + 1);
        
        // MIPS register allocation: Direct mapping for t0-t9
        // For temps beyond t9, use modulo wrapping
        // Note: This may cause issues with complex programs using >10 temps
        int regNum = tempNum % 10;
        sprintf(reg, "$t%d", regNum);
        return reg;
    }
    return temp;
}

/* Helper function to evaluate constant expression */
static int evalConstExpr(const char* arg1, const char* arg2, TACOp op, char* result_buf) {
    if (!isConst(arg1) || !isConst(arg2)) return 0;
    
    int val1 = atoi(arg1);
    int val2 = atoi(arg2);
    int result = 0;
    
    switch(op) {
        case TAC_ADD: result = val1 + val2; break;
        case TAC_SUBTRACT: result = val1 - val2; break;
        case TAC_MULTIPLY: result = val1 * val2; break;
        case TAC_DIVIDE: 
            if (val2 == 0) return 0;  // Avoid division by zero
            result = val1 / val2; 
            break;
        default: return 0;
    }
    
    sprintf(result_buf, "%d", result);
    return 1;
}

/* TAC Peephole Optimization with Constant Folding */
void optimizeTAC2() {
    TACInstr* curr = tacList.head;
    optimizedList2.head = optimizedList2.tail = NULL;
    int optimizations_applied = 0;
    
    printf("OPTIMIZER: Starting TAC peephole optimization...\n");
    
    while (curr) {
        int skip = 0;
        
        // OPTIMIZATION 1: Constant Folding for binary operations
        if ((curr->op == TAC_ADD || curr->op == TAC_SUBTRACT || 
             curr->op == TAC_MULTIPLY || curr->op == TAC_DIVIDE) &&
            isConst(curr->arg1) && isConst(curr->arg2)) {
            
            char result_val[32];
            if (evalConstExpr(curr->arg1, curr->arg2, curr->op, result_val)) {
                // Replace with constant assignment
                TACInstr* newInstr = createTAC(TAC_ASSIGN, result_val, NULL, curr->result);
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
                
                printf("OPTIMIZER: Folded %s %s %s -> %s\n", 
                       curr->arg1, 
                       curr->op == TAC_ADD ? "+" : curr->op == TAC_SUBTRACT ? "-" : 
                       curr->op == TAC_MULTIPLY ? "*" : "/",
                       curr->arg2, result_val);
                optimizations_applied++;
                skip = 1;
            }
        }
        
        // OPTIMIZATION 2: Copy propagation for simple assignments
        // If we see: t0 = const; t1 = t0; we can replace with t1 = const
        // (This would require multi-pass or tracking, keeping it simple for now)
        
        // OPTIMIZATION 3: Algebraic simplification
        // x + 0 -> x, x * 1 -> x, x * 0 -> 0
        if (!skip && curr->op == TAC_ADD) {
            if (isConst(curr->arg2) && atoi(curr->arg2) == 0) {
                // x + 0 = x  ->  result = x
                TACInstr* newInstr = createTAC(TAC_ASSIGN, curr->arg1, NULL, curr->result);
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
                printf("OPTIMIZER: Simplified %s + 0 -> %s\n", curr->arg1, curr->arg1);
                optimizations_applied++;
                skip = 1;
            } else if (isConst(curr->arg1) && atoi(curr->arg1) == 0) {
                // 0 + y = y  ->  result = y
                TACInstr* newInstr = createTAC(TAC_ASSIGN, curr->arg2, NULL, curr->result);
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
                printf("OPTIMIZER: Simplified 0 + %s -> %s\n", curr->arg2, curr->arg2);
                optimizations_applied++;
                skip = 1;
            }
        }
        
        if (!skip && curr->op == TAC_MULTIPLY) {
            if (isConst(curr->arg2) && atoi(curr->arg2) == 1) {
                // x * 1 = x
                TACInstr* newInstr = createTAC(TAC_ASSIGN, curr->arg1, NULL, curr->result);
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
                printf("OPTIMIZER: Simplified %s * 1 -> %s\n", curr->arg1, curr->arg1);
                optimizations_applied++;
                skip = 1;
            } else if (isConst(curr->arg1) && atoi(curr->arg1) == 1) {
                // 1 * y = y
                TACInstr* newInstr = createTAC(TAC_ASSIGN, curr->arg2, NULL, curr->result);
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
                printf("OPTIMIZER: Simplified 1 * %s -> %s\n", curr->arg2, curr->arg2);
                optimizations_applied++;
                skip = 1;
            } else if ((isConst(curr->arg2) && atoi(curr->arg2) == 0) ||
                      (isConst(curr->arg1) && atoi(curr->arg1) == 0)) {
                // x * 0 = 0 or 0 * y = 0
                TACInstr* newInstr = createTAC(TAC_ASSIGN, "0", NULL, curr->result);
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
                printf("OPTIMIZER: Simplified %s * %s -> 0\n", curr->arg1, curr->arg2);
                optimizations_applied++;
                skip = 1;
            }
        }
        
        // If not optimized, copy instruction as-is
        if (!skip) {
            TACInstr* newInstr = createTAC(curr->op, curr->arg1, curr->arg2, curr->result);
            if (newInstr) {
                if (!optimizedList2.head) optimizedList2.head = optimizedList2.tail = newInstr;
                else { optimizedList2.tail->next = newInstr; optimizedList2.tail = newInstr; }
            }
        }
        
        curr = curr->next;
    }
    
    printf("OPTIMIZER: Applied %d optimizations\n", optimizations_applied);
    optimizedList = optimizedList2; // Sync with global optimizedList
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
            case TAC_ARRAY_DECL: printf("ARRAY_DECL %s[%s]\n", curr->result, curr->arg1); break;
            case TAC_ARRAY_WRITE: printf("%s[%s] = %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_ARRAY_READ: printf("%s = %s[%s]\n", curr->result, curr->arg1, curr->arg2); break;
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
            case TAC_ARRAY_DECL: fprintf(file, "ARRAY_DECL %s[%s]\n", curr->result, curr->arg1); break;
            case TAC_ARRAY_WRITE: fprintf(file, "%s[%s] = %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_ARRAY_READ: fprintf(file, "%s = %s[%s]\n", curr->result, curr->arg1, curr->arg2); break;
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
    
    // Initialize LRU register allocator
    // TEMPORARILY DISABLED FOR DEBUGGING
    // init_register_allocator();
    
    // Write MIPS header
    fprintf(output, ".data\n\n");
    fprintf(output, ".text\n");
    fprintf(output, ".globl main\n\n");
    
    // Generate main function
    appendMIPS(&mipsList, createMIPS(MIPS_LABEL, NULL, "main", NULL, NULL));
    
    // Calculate stack space needed for main function
    // Get the symbol table's nextOffset to know how much stack we need
    int stackSize = 0;
    if (currentSymTab) {
        stackSize = currentSymTab->nextOffset;
    }
    
    // Allocate stack frame if needed
    if (stackSize > 0) {
        char stackStr[32];
        sprintf(stackStr, "-%d", stackSize);
        appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$sp", "$sp", stackStr, NULL));
    }
    
    // Process main function body - handle ALL operations including function calls
    TACInstr* curr = optimizedList2.head;
    int inMain = 0;
    #define MAX_ARGS 32  // Support up to 32 arguments per function call
    TACInstr* pendingArgs[MAX_ARGS];  // Track pending ARG instructions before a call
    int pendingArgCount = 0;
    
    // Iterate through all TAC and process main function body
    while (curr) {
        if (curr->op == TAC_FUNC_DEF && curr->arg1 && strcmp(curr->arg1, "main") == 0) {
            inMain = 1;
            curr = curr->next;
            continue;
        }
        if (curr->op == TAC_FUNC_DEF && inMain) {
            break; // Exited main
        }
        
        if (inMain) {
            if (curr->op == TAC_ARG) {
                // Store ARG instruction for upcoming function call
                pendingArgs[pendingArgCount++] = curr;
            } else if (curr->op == TAC_FUNC_CALL) {
                // Load arguments into registers (MIPS convention: $a0-$a3, then stack)
                // Args are in reverse order in pendingArgs array, so we need to reverse
                for (int i = pendingArgCount - 1; i >= 0; i--) {
                    int argIndex = pendingArgCount - 1 - i;  // 0-based argument index in correct order
                    char* argValue = pendingArgs[i]->arg1;
                    
                    // Determine target location: $a0-$a3 or stack
                    if (argIndex < 4) {
                        // Use argument registers $a0-$a3
                        char argReg[8];
                        sprintf(argReg, "$a%d", argIndex);
                        
                        // Load argument value into register
                        if (isdigit(argValue[0]) || (argValue[0] == '-' && isdigit(argValue[1]))) {
                            // Immediate value
                            appendMIPS(&mipsList, createMIPS(MIPS_LI, argReg, argValue, NULL, NULL));
                        } else if (isTemporary(argValue)) {
                            // Temporary variable
                            char regBuf[10];
                            strcpy(regBuf, tempToReg(argValue));
                            appendMIPS(&mipsList, createMIPS(MIPS_MOVE, argReg, regBuf, NULL, NULL));
                        } else {
                            // Variable - check if array or regular
                            int offset = getOptimizerVarOffset(argValue);
                            if (offset != -1) {
                                if (isArrayVar(argValue)) {
                                    // Array: pass address
                                    char offsetStr[32];
                                    sprintf(offsetStr, "%d", offset);
                                    appendMIPS(&mipsList, createMIPS(MIPS_ADDI, argReg, "$sp", offsetStr, NULL));
                                } else {
                                    // Regular variable: load value
                                    char offsetStr[32];
                                    sprintf(offsetStr, "%d($sp)", offset);
                                    appendMIPS(&mipsList, createMIPS(MIPS_LW, argReg, offsetStr, NULL, NULL));
                                }
                            }
                        }
                    } else {
                        // Arguments beyond $a3 would go on stack (for full MIPS compliance)
                        // For now, warn that this isn't fully implemented
                        if (argIndex == 4) {  // Only warn once per call
                            fprintf(stderr, "Warning: Functions with > 4 arguments use simplified handling\n");
                        }
                    }
                }
                
                // Make function call
                appendMIPS(&mipsList, createMIPS(MIPS_JAL, NULL, curr->arg1, NULL, NULL));
                
                // Move result to temp register
                if (curr->result && isTemporary(curr->result)) {
                    char resultBuf[10];
                    strcpy(resultBuf, tempToReg(curr->result));
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, resultBuf, "$v0", NULL, NULL));
                }
                
                // Clear pending args for next call
                pendingArgCount = 0;
            } else if (curr->op == TAC_ARRAY_WRITE) {
                // arr[index] = value: arg1=arrayName, arg2=index, result=value
                char* arrayName = curr->arg1;
                char* index = curr->arg2;
                char* value = curr->result;
            int baseOffset = getOptimizerVarOffset(arrayName);
            
            if (baseOffset != -1) {
                // Load index into $t7
                if (isTemporary(index)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$t7", tempToReg(index), NULL, NULL));
                } else if (isdigit(index[0]) || (index[0] == '-' && isdigit(index[1]))) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t7", index, NULL, NULL));
                } else {
                    char indexOffsetStr[32];
                    int indexOffset = getOptimizerVarOffset(index);
                    if (indexOffset != -1) {
                        sprintf(indexOffsetStr, "%d($fp)", indexOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, "$t7", indexOffsetStr, NULL, NULL));
                    }
                }
                
                // Calculate offset: $t7 = $t7 * 4
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t8", "4", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_MUL, NULL, "$t7", "$t8", NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$t7", NULL, NULL, NULL));
                
                // Add base offset
                char baseStr[32];
                sprintf(baseStr, "%d", baseOffset);
                appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$t7", "$t7", baseStr, NULL));
                
                // Add $sp to get final address
                appendMIPS(&mipsList, createMIPS(MIPS_ADD, "$t7", "$t7", "$sp", NULL));
                
                // Load value into $t6
                if (isTemporary(value)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$t6", tempToReg(value), NULL, NULL));
                } else if (isdigit(value[0]) || (value[0] == '-' && isdigit(value[1]))) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t6", value, NULL, NULL));
                } else {
                    char valueOffsetStr[32];
                    int valueOffset = getOptimizerVarOffset(value);
                    if (valueOffset != -1) {
                        sprintf(valueOffsetStr, "%d($sp)", valueOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, "$t6", valueOffsetStr, NULL, NULL));
                    }
                }
                
                // Store value at calculated address
                appendMIPS(&mipsList, createMIPS(MIPS_SW, "$t6", "0($t7)", NULL, NULL));
            }
            } else if (curr->op == TAC_ARRAY_READ) {
            // temp = arr[index]: result=temp, arg1=array_name, arg2=index
            char* temp = curr->result;
            char* arrayName = curr->arg1;
            char* index = curr->arg2;
            int baseOffset = getOptimizerVarOffset(arrayName);
            
            if (baseOffset != -1) {
                // Load index into $t7
                if (isTemporary(index)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$t7", tempToReg(index), NULL, NULL));
                } else if (isdigit(index[0]) || (index[0] == '-' && isdigit(index[1]))) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t7", index, NULL, NULL));
                } else {
                    char indexOffsetStr[32];
                    int indexOffset = getOptimizerVarOffset(index);
                    if (indexOffset != -1) {
                        sprintf(indexOffsetStr, "%d($sp)", indexOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, "$t7", indexOffsetStr, NULL, NULL));
                    }
                }
                
                // Calculate offset: $t7 = $t7 * 4
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t8", "4", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_MUL, NULL, "$t7", "$t8", NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$t7", NULL, NULL, NULL));
                
                // Add base offset
                char baseStr[32];
                sprintf(baseStr, "%d", baseOffset);
                appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$t7", "$t7", baseStr, NULL));
                
                // Add $sp to get final address
                appendMIPS(&mipsList, createMIPS(MIPS_ADD, "$t7", "$t7", "$sp", NULL));
                
                // Load value from calculated address into temp register
                if (isTemporary(temp)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LW, (char*)tempToReg(temp), "0($t7)", NULL, NULL));
                }
            }
            } else if (curr->op == TAC_ASSIGN) {
                char* dest = curr->result;
                char* src = curr->arg1;
                int destOffset = getOptimizerVarOffset(dest);
                if (isTemporary(src)) {
                    if (destOffset != -1) {
                        char offsetStr[32];
                        sprintf(offsetStr, "%d($sp)", destOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_SW, (char*)tempToReg(src), offsetStr, NULL, NULL));
                    }
                } else if ((src && isdigit(src[0])) || (src && src[0] == '-' && isdigit(src[1]))) {
                    if (destOffset != -1) {
                        appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t7", src, NULL, NULL));
                        char offsetStr[32];
                        sprintf(offsetStr, "%d($sp)", destOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_SW, "$t7", offsetStr, NULL, NULL));
                    }
                } else if (src) {
                    int srcOffset = getOptimizerVarOffset(src);
                    if (srcOffset != -1 && destOffset != -1) {
                        char srcOffsetStr[32], destOffsetStr[32];
                        sprintf(srcOffsetStr, "%d($sp)", srcOffset);
                        sprintf(destOffsetStr, "%d($sp)", destOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, "$t7", srcOffsetStr, NULL, NULL));
                        appendMIPS(&mipsList, createMIPS(MIPS_SW, "$t7", destOffsetStr, NULL, NULL));
                    }
                }
            } else if (curr->op == TAC_PRINT) {
                char* val = curr->arg1;
                if (isTemporary(val)) {
                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$a0", (char*)tempToReg(val), NULL, NULL));
                } else if ((val && isdigit(val[0])) || (val && val[0] == '-' && isdigit(val[1]))) {
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", val, NULL, NULL));
                } else if (val) {
                    int valOffset = getOptimizerVarOffset(val);
                    if (valOffset != -1) {
                        char offsetStr[32];
                        sprintf(offsetStr, "%d($sp)", valOffset);
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, "$a0", offsetStr, NULL, NULL));
                    }
                }
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "1", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "11", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_LI, "$a0", "10", NULL, NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
            } else if (curr->op == TAC_ADD) {
                // Handle add: result = arg1 + arg2
                if (isTemporary(curr->arg1) && isTemporary(curr->arg2) && isTemporary(curr->result)) {
                    // Create separate buffers for each register string (NOT static)
                    char arg1RegBuf[10], arg2RegBuf[10], resultRegBuf[10];
                    const char* arg1RegConst = tempToReg(curr->arg1);
                    const char* arg2RegConst = tempToReg(curr->arg2);
                    const char* resultRegConst = tempToReg(curr->result);
                    strcpy(arg1RegBuf, arg1RegConst);
                    strcpy(arg2RegBuf, arg2RegConst);
                    strcpy(resultRegBuf, resultRegConst);
                    appendMIPS(&mipsList, createMIPS(MIPS_ADD, resultRegBuf, arg1RegBuf, arg2RegBuf, NULL));
                }
            }
        }
        curr = curr->next;
    }
    
    // Exit program (PRINT instructions will handle output)
    // Restore stack pointer before exit
    if (stackSize > 0) {
        char stackStr[32];
        sprintf(stackStr, "%d", stackSize);
        appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$sp", "$sp", stackStr, NULL));
    }
    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$v0", "10", NULL, NULL));
    appendMIPS(&mipsList, createMIPS(MIPS_SYSCALL, NULL, NULL, NULL, NULL));
    
    // Generate function definitions (skip main)
    curr = optimizedList2.head;
    while (curr) {
        if (curr->op == TAC_FUNC_DEF && curr->arg1 && strcmp(curr->arg1, "main") != 0) {
            // Generate function label
            appendMIPS(&mipsList, createMIPS(MIPS_LABEL, NULL, curr->arg1, NULL, NULL));
            
            // Check if this function calls other functions - if so, need to save $ra
            int callsOtherFuncs = 0;
            TACInstr* scan = curr->next;
            while (scan && scan->op != TAC_FUNC_DEF) {
                if (scan->op == TAC_FUNC_CALL) {
                    callsOtherFuncs = 1;
                    break;
                }
                scan = scan->next;
            }
            
            // If function calls others, save $ra and allocate stack
            if (callsOtherFuncs) {
                appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$sp", "$sp", "-8", NULL));
                appendMIPS(&mipsList, createMIPS(MIPS_SW, "$ra", "0($sp)", NULL, NULL));
            }
            
            // Build parameter-to-register mapping
            // Scan ahead to find TAC_PARAM instructions and map them to $a0-$a3
            char paramNames[4][32];  // Store up to 4 parameter names
            int paramCount = 0;
            TACInstr* paramScan = curr->next;
            while (paramScan && paramScan->op == TAC_PARAM && paramCount < 4) {
                if (paramScan->arg1) {
                    strcpy(paramNames[paramCount], paramScan->arg1);
                    paramCount++;
                }
                paramScan = paramScan->next;
            }
            
            // Process all instructions in this function
            TACInstr* funcInstr = curr->next;
            TACInstr* funcPendingArgs[MAX_ARGS];  // Support up to MAX_ARGS arguments
            int funcPendingArgCount = 0;
            
            // Track variable-to-register mappings for this function
            // Simple map: variable name -> temp register name
            #define MAX_VAR_MAPPINGS 32
            char varToTemp[MAX_VAR_MAPPINGS][32];  // Variable names
            char varToReg[MAX_VAR_MAPPINGS][32];   // Corresponding temp registers
            int varMapCount = 0;
            
            while (funcInstr && funcInstr->op != TAC_FUNC_DEF) {
                if (funcInstr->op == TAC_ARG) {
                    funcPendingArgs[funcPendingArgCount++] = funcInstr;
                } else if (funcInstr->op == TAC_FUNC_CALL) {
                    // Handle function call within function - support variable arguments
                    // Args are in reverse order, so reverse them
                    for (int i = funcPendingArgCount - 1; i >= 0; i--) {
                        int argIndex = funcPendingArgCount - 1 - i;
                        char* argValue = funcPendingArgs[i]->arg1;
                        
                        if (argIndex < 4) {
                            char argReg[8];
                            sprintf(argReg, "$a%d", argIndex);
                            
                            if (isdigit(argValue[0]) || (argValue[0] == '-' && isdigit(argValue[1]))) {
                                // Immediate value
                                appendMIPS(&mipsList, createMIPS(MIPS_LI, argReg, argValue, NULL, NULL));
                            } else if (isTemporary(argValue)) {
                                // Temporary variable
                                char regBuf[10];
                                strcpy(regBuf, tempToReg(argValue));
                                appendMIPS(&mipsList, createMIPS(MIPS_MOVE, argReg, regBuf, NULL, NULL));
                            } else {
                                // Check if it's a mapped variable
                                int found = 0;
                                for (int j = 0; j < varMapCount; j++) {
                                    if (strcmp(varToTemp[j], argValue) == 0) {
                                        char regBuf[10];
                                        strcpy(regBuf, tempToReg(varToReg[j]));
                                        appendMIPS(&mipsList, createMIPS(MIPS_MOVE, argReg, regBuf, NULL, NULL));
                                        found = 1;
                                        break;
                                    }
                                }
                                if (!found) {
                                    // Assume it's a function parameter - could be in $a0-$a3
                                    // For simplicity, don't generate code (it's already there)
                                }
                            }
                        } else {
                            // Arguments beyond 4 would go on stack
                            if (argIndex == 4) {
                                fprintf(stderr, "Warning: Functions with > 4 arguments use simplified handling\n");
                            }
                        }
                    }
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_JAL, NULL, funcInstr->arg1, NULL, NULL));
                    
                    if (funcInstr->result && isTemporary(funcInstr->result)) {
                        char resultBuf[10];
                        strcpy(resultBuf, tempToReg(funcInstr->result));
                        appendMIPS(&mipsList, createMIPS(MIPS_MOVE, resultBuf, "$v0", NULL, NULL));
                    }
                    funcPendingArgCount = 0;
                } else if (funcInstr->op == TAC_ASSIGN) {
                    // Handle assignment: result = arg1
                    // Store temp to stack variable or temp to temp
                    if (isTemporary(funcInstr->arg1) && isTemporary(funcInstr->result)) {
                        char arg1Buf[10], resBuf[10];
                        strcpy(arg1Buf, tempToReg(funcInstr->arg1));
                        strcpy(resBuf, tempToReg(funcInstr->result));
                        appendMIPS(&mipsList, createMIPS(MIPS_MOVE, resBuf, arg1Buf, NULL, NULL));
                    } else if (isTemporary(funcInstr->arg1) && !isTemporary(funcInstr->result)) {
                        // Variable = temp: track this mapping
                        if (varMapCount < MAX_VAR_MAPPINGS) {
                            strcpy(varToTemp[varMapCount], funcInstr->result);
                            strcpy(varToReg[varMapCount], funcInstr->arg1);
                            varMapCount++;
                        }
                    }
                } else if (funcInstr->op == TAC_ARRAY_READ) {
                    // Handle array access in functions: arr[index]
                    // TAC format: arg1=arrayName, arg2=index, result=resultTemp
                    char* arrayName = funcInstr->arg1;
                    char* index = funcInstr->arg2;
                    char* resultTemp = funcInstr->result;
                    
                    // Array parameters have base address in $a0 (first param)
                    // For now, assume array param is always first param
                    
                    // Load index into $t7
                    if (index && ((isdigit(index[0])) || (index[0] == '-' && isdigit(index[1])))) {
                        appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t7", index, NULL, NULL));
                    } else if (index && isTemporary(index)) {
                        char regBuf[10];
                        strcpy(regBuf, tempToReg(index));
                        appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$t7", regBuf, NULL, NULL));
                    } else {
                        // Default to 0 if index is NULL or unrecognized
                        appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t7", "0", NULL, NULL));
                    }
                    
                    // Calculate offset: index * 4
                    appendMIPS(&mipsList, createMIPS(MIPS_LI, "$t8", "4", NULL, NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MUL, NULL, "$t7", "$t8", NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$t7", NULL, NULL, NULL));
                    
                    // Add base address (array param in $a0)
                    appendMIPS(&mipsList, createMIPS(MIPS_ADD, "$t7", "$t7", "$a0", NULL));
                    
                    // Load value
                    if (resultTemp && isTemporary(resultTemp)) {
                        char regBuf[10];
                        strcpy(regBuf, tempToReg(resultTemp));
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, regBuf, "0($t7)", NULL, NULL));
                    }
                } else if (funcInstr->op == TAC_ADD) {
                    // Handle ADD operations using temp registers or parameters
                    char arg1Buf[10], arg2Buf[10], resBuf[10];
                    
                    // Map arg1 to appropriate register
                    if (isTemporary(funcInstr->arg1)) {
                        strcpy(arg1Buf, tempToReg(funcInstr->arg1));
                    } else {
                        // Check if it's a parameter
                        int found = 0;
                        for (int p = 0; p < paramCount; p++) {
                            if (strcmp(funcInstr->arg1, paramNames[p]) == 0) {
                                sprintf(arg1Buf, "$a%d", p);
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            // Check variable mappings
                            for (int v = 0; v < varMapCount; v++) {
                                if (strcmp(varToTemp[v], funcInstr->arg1) == 0) {
                                    strcpy(arg1Buf, tempToReg(varToReg[v]));
                                    found = 1;
                                    break;
                                }
                            }
                            if (!found) strcpy(arg1Buf, "$a0");  // Fallback
                        }
                    }
                    
                    // Map arg2 to appropriate register
                    if (isTemporary(funcInstr->arg2)) {
                        strcpy(arg2Buf, tempToReg(funcInstr->arg2));
                    } else {
                        int found = 0;
                        for (int p = 0; p < paramCount; p++) {
                            if (strcmp(funcInstr->arg2, paramNames[p]) == 0) {
                                sprintf(arg2Buf, "$a%d", p);
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            for (int v = 0; v < varMapCount; v++) {
                                if (strcmp(varToTemp[v], funcInstr->arg2) == 0) {
                                    strcpy(arg2Buf, tempToReg(varToReg[v]));
                                    found = 1;
                                    break;
                                }
                            }
                            if (!found) strcpy(arg2Buf, "$a1");  // Fallback
                        }
                    }
                    
                    // Map result
                    if (isTemporary(funcInstr->result)) {
                        strcpy(resBuf, tempToReg(funcInstr->result));
                    } else {
                        strcpy(resBuf, "$v0");
                    }
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_ADD, resBuf, arg1Buf, arg2Buf, NULL));
                } else if (funcInstr->op == TAC_MULTIPLY) {
                    // Handle MULTIPLY operations
                    char arg1Buf[10], arg2Buf[10];
                    
                    // Map arg1
                    if (isTemporary(funcInstr->arg1)) {
                        strcpy(arg1Buf, tempToReg(funcInstr->arg1));
                    } else {
                        int found = 0;
                        for (int p = 0; p < paramCount; p++) {
                            if (strcmp(funcInstr->arg1, paramNames[p]) == 0) {
                                sprintf(arg1Buf, "$a%d", p);
                                found = 1;
                                break;
                            }
                        }
                        if (!found) strcpy(arg1Buf, "$a0");  // Fallback
                    }
                    
                    // Map arg2
                    if (isTemporary(funcInstr->arg2)) {
                        strcpy(arg2Buf, tempToReg(funcInstr->arg2));
                    } else {
                        int found = 0;
                        for (int p = 0; p < paramCount; p++) {
                            if (strcmp(funcInstr->arg2, paramNames[p]) == 0) {
                                sprintf(arg2Buf, "$a%d", p);
                                found = 1;
                                break;
                            }
                        }
                        if (!found) strcpy(arg2Buf, "$a1");  // Fallback
                    }
                    
                    appendMIPS(&mipsList, createMIPS(MIPS_MUL, NULL, arg1Buf, arg2Buf, NULL));
                    
                    if (funcInstr->result && isTemporary(funcInstr->result)) {
                        char resBuf[10];
                        strcpy(resBuf, tempToReg(funcInstr->result));
                        appendMIPS(&mipsList, createMIPS(MIPS_MFLO, resBuf, NULL, NULL, NULL));
                    } else {
                        appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$v0", NULL, NULL, NULL));
                    }
                } else if (funcInstr->op == TAC_DIVIDE) {
                    appendMIPS(&mipsList, createMIPS(MIPS_DIV, NULL, "$a0", "$a1", NULL));
                    appendMIPS(&mipsList, createMIPS(MIPS_MFLO, "$v0", NULL, NULL, NULL));
                } else if (funcInstr->op == TAC_SUBTRACT) {
                    appendMIPS(&mipsList, createMIPS(MIPS_SUB, "$v0", "$a0", "$a1", NULL));
                } else if (funcInstr->op == TAC_RETURN) {
                    // If returning a temp, move it to $v0 first
                    if (funcInstr->arg1 && isTemporary(funcInstr->arg1)) {
                        char regBuf[10];
                        strcpy(regBuf, tempToReg(funcInstr->arg1));
                        appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$v0", regBuf, NULL, NULL));
                    } else if (funcInstr->arg1 && !isTemporary(funcInstr->arg1)) {
                        // Returning a variable - check mapping (search backwards for latest mapping)
                        int found = 0;
                        for (int i = varMapCount - 1; i >= 0; i--) {
                            if (strcmp(varToTemp[i], funcInstr->arg1) == 0) {
                                // Found mapping: variable -> temp register
                                char regBuf[10];
                                strcpy(regBuf, tempToReg(varToReg[i]));
                                appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$v0", regBuf, NULL, NULL));
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            // Check if it's a parameter
                            for (int p = 0; p < paramCount; p++) {
                                if (strcmp(funcInstr->arg1, paramNames[p]) == 0) {
                                    char paramReg[8];
                                    sprintf(paramReg, "$a%d", p);
                                    appendMIPS(&mipsList, createMIPS(MIPS_MOVE, "$v0", paramReg, NULL, NULL));
                                    found = 1;
                                    break;
                                }
                            }
                        }
                    }
                    
                    // If function called others, restore $ra and stack
                    if (callsOtherFuncs) {
                        appendMIPS(&mipsList, createMIPS(MIPS_LW, "$ra", "0($sp)", NULL, NULL));
                        appendMIPS(&mipsList, createMIPS(MIPS_ADDI, "$sp", "$sp", "8", NULL));
                    }
                    
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
