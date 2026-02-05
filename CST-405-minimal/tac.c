#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tac.h"

TACList tacList;
TACList optimizedList;

// Peephole optimization stats
typedef struct {
    int eliminated_temps;
    int constant_folded;
    int dead_code_removed;
} OptimizationStats;

static OptimizationStats opt_stats = {0};

void initTAC() {
    tacList.head = NULL;
    tacList.tail = NULL;
    tacList.tempCount = 0;
    optimizedList.head = NULL;
    optimizedList.tail = NULL;
}

char* newTemp() {
    char* temp = malloc(10);
    sprintf(temp, "t%d", tacList.tempCount++);
    return temp;
}

TACInstr* createTAC(TACOp op, char* arg1, char* arg2, char* result) {
    TACInstr* instr = malloc(sizeof(TACInstr));
    instr->op = op;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    instr->result = result ? strdup(result) : NULL;
    instr->next = NULL;
    return instr;
}

void appendTAC(TACInstr* instr) {
    if (!tacList.head) {
        tacList.head = tacList.tail = instr;
    } else {
        tacList.tail->next = instr;
        tacList.tail = instr;
    }
}

void appendOptimizedTAC(TACInstr* instr) {
    if (!optimizedList.head) {
        optimizedList.head = optimizedList.tail = instr;
    } else {
        optimizedList.tail->next = instr;
        optimizedList.tail = instr;
    }
}

int isConst(const char* s) {
    if (!s || *s == '\0') return 0;

    char* end;
    strtod(s, &end);
    return *end == '\0';
}

static void removeNOPs() {
    TACInstr* current = tacList.head;
    TACInstr* prev = NULL;

    while (current) {
        if (current->op == TAC_NOP) {
            TACInstr* toDelete = current;
            if (prev) {
                prev->next = current->next;
                current = current->next;
            } else {
                tacList.head = current->next;
                current = current->next;
            }

            if (toDelete == tacList.tail) {
                tacList.tail = prev;
            }

            opt_stats.dead_code_removed++;
            // Note: In production, should free toDelete
        } else {
            prev = current;
            current = current->next;
        }
    }

    if (!tacList.head) {
        tacList.tail = NULL;
    }
}

static void peepholeOptimizeTAC() {
    TACInstr* current = tacList.head;

    while (current && current->next) {
        // Pattern 1: Constant folding for temp additions
        // t0 = 5; t1 = 10; t2 = t0 + t1; => t2 = 15;
        if (current->op == TAC_ASSIGN &&
            current->next->op == TAC_ASSIGN &&
            isConst(current->arg1) &&
            isConst(current->next->arg1)) {

            TACInstr* third = current->next->next;
            if (third && third->op == TAC_ADD &&
                third->arg1 && third->arg2 &&
                current->result && current->next->result &&
                strcmp(third->arg1, current->result) == 0 &&
                strcmp(third->arg2, current->next->result) == 0) {

                int val1 = atoi(current->arg1);
                int val2 = atoi(current->next->arg1);
                char buffer[32];
                sprintf(buffer, "%d", val1 + val2);

                // Replace with single assignment
                third->op = TAC_ASSIGN;
                if (third->arg1) free(third->arg1);
                third->arg1 = strdup(buffer);
                if (third->arg2) {
                    free(third->arg2);
                    third->arg2 = NULL;
                }

                // Remove redundant instructions
                current->op = TAC_NOP;  // Mark for removal
                current->next->op = TAC_NOP;

                opt_stats.constant_folded++;
            }
        }

        // Pattern 2: Copy propagation
        // t0 = x; y = t0; => y = x;
        if (current->op == TAC_ASSIGN &&
            current->next->op == TAC_ASSIGN &&
            current->result && current->next->arg1 &&
            strcmp(current->next->arg1, current->result) == 0) {

            free(current->next->arg1);
            current->next->arg1 = current->arg1 ? strdup(current->arg1) : NULL;
            current->op = TAC_NOP;
            opt_stats.eliminated_temps++;
        }

        current = current->next;
    }

    // Remove NOP instructions
    removeNOPs();
}

char* generateTACExpr(ASTNode* node) {
    if (!node) return NULL;
    
    switch(node->type) {
        case NODE_NUM: {
            char* temp = malloc(20);
            sprintf(temp, "%d", node->data.num);
            return temp;
        }
        
        case NODE_FLT: {
            char* temp = malloc(20);
            sprintf(temp, "%f", node->data.flt);
            return temp;
        }
        
        case NODE_VAR:
            return strdup(node->data.var.name);
        
        case NODE_BINOP: {
            char* left = generateTACExpr(node->data.binop.left);
            char* right = generateTACExpr(node->data.binop.right);
            char* temp = newTemp();
            
            if (node->data.binop.op == '+') {
                appendTAC(createTAC(TAC_ADD, left, right, temp));
            } else if (node->data.binop.op == '-') {
                appendTAC(createTAC(TAC_SUBTRACT, left, right, temp));
            } else if (node->data.binop.op == '*') {
                appendTAC(createTAC(TAC_MULTIPLY, left, right, temp));
            } else if (node->data.binop.op == '/') {
                appendTAC(createTAC(TAC_DIVIDE, left, right, temp));
            }
            return temp;
        }
        
        default:
            return NULL;
    }
}

void generateTAC(ASTNode* node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_DECL:
            appendTAC(createTAC(TAC_DECL, NULL, NULL, node->data.var.name));
            break;
            
        case NODE_ASSIGN: {
            char* expr = generateTACExpr(node->data.assign.value);
            appendTAC(createTAC(TAC_ASSIGN, expr, NULL, node->data.assign.var));
            break;
        }
        
        case NODE_PRINT: {
            char* expr = generateTACExpr(node->data.expr);
            appendTAC(createTAC(TAC_PRINT, expr, NULL, NULL));
            break;
        }
        
        case NODE_STMT_LIST:
            generateTAC(node->data.stmtlist.stmt);
            generateTAC(node->data.stmtlist.next);
            break;
            
        default:
            break;
    }
}

void printTAC() {
    printf("Unoptimized TAC Instructions:\n");
    printf("─────────────────────────────\n");
    TACInstr* curr = tacList.head;
    int lineNum = 1;
    while (curr) {
        printf("%2d: ", lineNum++);
        switch(curr->op) {
            case TAC_DECL:
                printf("DECL %s", curr->result);
                printf("          // Declare variable '%s'\n", curr->result);
                break;
            case TAC_ADD:
                printf("%s = %s + %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Add: store result in %s\n", curr->result);
                break;
            case TAC_SUBTRACT:
                printf("%s = %s - %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Subtract: store result in %s\n", curr->result);
                break;
            case TAC_MULTIPLY:
                printf("%s = %s * %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Multiply: store result in %s\n", curr->result);
                break;
            case TAC_DIVIDE:
                printf("%s = %s / %s", curr->result, curr->arg1, curr->arg2);
                printf("     // Divide: store result in %s\n", curr->result);
                break;
            case TAC_ASSIGN:
                printf("%s = %s", curr->result, curr->arg1);
                printf("           // Assign value to %s\n", curr->result);
                break;
            case TAC_PRINT:
                printf("PRINT %s", curr->arg1);
                printf("          // Output value of %s\n", curr->arg1);
                break;
            case TAC_NOP:
                printf("NOP\n");
                break;
            default:
                break;
        }
        curr = curr->next;
    }
}

// Simple optimization: constant folding and copy propagation
void optimizeTAC() {
    opt_stats.eliminated_temps = 0;
    opt_stats.constant_folded = 0;
    opt_stats.dead_code_removed = 0;

    // Peephole pass works in-place on tacList
    peepholeOptimizeTAC();

    // Reset optimized list before rebuilding
    optimizedList.head = NULL;
    optimizedList.tail = NULL;

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
            case TAC_DECL:
                newInstr = createTAC(TAC_DECL, NULL, NULL, curr->result);
                break;
                
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
                double result = atof(left) + atof(right);
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
            case TAC_NOP:
                // Ignore (shouldn't be present after peephole removal)
                break;
        }
        
        if (newInstr) {
            appendOptimizedTAC(newInstr);
        }
        
        curr = curr->next;
    }
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
            case TAC_NOP:
                printf("NOP\n");
                break;
            default:
                break;
        }
        curr = curr->next;
    }
}
