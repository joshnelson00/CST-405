#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tac.h"

TACList tacList;
extern TACList optimizedList;

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
        
        case NODE_FUNC_CALL: {
            // Generate arguments first
            if (node->data.func_call.args) {
                generateTACArgList(node->data.func_call.args);
            }
            
            // Create temporary for return value and generate call instruction
            char* temp = newTemp();
            appendTAC(createTAC(TAC_FUNC_CALL, node->data.func_call.name, NULL, temp));
            
            return temp;
        }
        
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
            
        case NODE_FUNC: {
            // Function definition
            appendTAC(createTAC(TAC_FUNC_DEF, node->data.func.name, NULL, NULL));
            // Generate parameters
            if (node->data.func.params) {
                generateTAC(node->data.func.params);
            }
            // Generate function body
            if (node->data.func.body) {
                generateTAC(node->data.func.body);
            }
            
            // Generate next function if it exists
            if (node->data.func.next) {
                generateTAC(node->data.func.next);
            }
            break;
        }
        
        case NODE_FUNC_LIST:
            // Generate current function
            generateTAC(node->data.func_list.func);
            // Generate next function if it exists
            if (node->data.func_list.next) {
                generateTAC(node->data.func_list.next);
            }
            break;
        
        case NODE_PARAM:
            // Parameters are already added to symbol table during parsing
            appendTAC(createTAC(TAC_PARAM, node->data.param.name, NULL, NULL));
            break;
            
        case NODE_PARAM_LIST:
            generateTAC(node->data.param_list.param);
            if (node->data.param_list.next) {
                generateTAC(node->data.param_list.next);
            }
            break;
            
        case NODE_RETURN: {
            if (node->data.return_stmt.expr) {
                char* expr = generateTACExpr(node->data.return_stmt.expr);
                appendTAC(createTAC(TAC_RETURN, expr, NULL, NULL));
            } else {
                appendTAC(createTAC(TAC_RETURN, NULL, NULL, NULL));
            }
            break;
        }
        
        case NODE_FUNC_CALL: {
            // Generate arguments first
            if (node->data.func_call.args) {
                generateTACArgList(node->data.func_call.args);
            }
            
            // Create temporary for return value and generate call instruction
            char* temp = newTemp();
            appendTAC(createTAC(TAC_FUNC_CALL, node->data.func_call.name, NULL, temp));
            
            // Return the temporary variable name
            // Note: We don't need to call generateTACFuncCall here as it would duplicate the call
            break;
        }
        
        case NODE_ARG_LIST:
            generateTACArgList(node);
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

/* Generate TAC for function call */
char* generateTACFuncCall(ASTNode* node) {
    if (!node || node->type != NODE_FUNC_CALL) return NULL;
    
    // Generate arguments first
    if (node->data.func_call.args) {
        generateTACArgList(node->data.func_call.args);
    }
    
    // Create temporary for return value
    char* temp = newTemp();
    appendTAC(createTAC(TAC_FUNC_CALL, node->data.func_call.name, NULL, temp));
    
    return temp;
}

/* Generate TAC for argument list */
void generateTACArgList(ASTNode* node) {
    if (!node || node->type != NODE_ARG_LIST) return;
    
    // Generate current argument
    char* arg = generateTACExpr(node->data.arg_list.arg);
    if (arg) {
        appendTAC(createTAC(TAC_ARG, arg, NULL, NULL));
    }
    
    // Generate next argument if exists
    if (node->data.arg_list.next) {
        generateTACArgList(node->data.arg_list.next);
    }
}

// Free TAC instruction list to prevent memory leaks
void freeTACList(TACList* list) {
    TACInstr* curr = list->head;
    while (curr) {
        TACInstr* next = curr->next;
        if (curr->arg1) free(curr->arg1);
        if (curr->arg2) free(curr->arg2);
        if (curr->result) free(curr->result);
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->tempCount = 0;
}
