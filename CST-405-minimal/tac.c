
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tac.h"

TACList tacList;
extern TACList optimizedList;

/* Initialize TAC lists */
void initTAC() {
    tacList.head = tacList.tail = NULL;
    tacList.tempCount = 0;
    tacList.labelCount = 0;
    optimizedList.head = optimizedList.tail = NULL;
}

/* Generate a new temporary variable */
char* newTemp() {
    char* temp = malloc(10);
    sprintf(temp, "t%d", tacList.tempCount++);
    return temp;
}

/* Generate a new label */
char* newLabel() {
    char* label = malloc(10);
    sprintf(label, "L%d", tacList.labelCount++);
    return label;
}

/* Create a TAC instruction */
TACInstr* createTAC(TACOp op, char* arg1, char* arg2, char* result) {
    TACInstr* instr = malloc(sizeof(TACInstr));
    instr->op = op;
    instr->arg1 = arg1 ? strdup(arg1) : NULL;
    instr->arg2 = arg2 ? strdup(arg2) : NULL;
    instr->result = result ? strdup(result) : NULL;
    instr->next = NULL;
    return instr;
}

/* Append instruction to main TAC list */
void appendTAC(TACInstr* instr) {
    if (!tacList.head) {
        tacList.head = tacList.tail = instr;
    } else {
        tacList.tail->next = instr;
        tacList.tail = instr;
    }
}

/* Append instruction to optimized TAC list */
void appendOptimizedTAC(TACInstr* instr) {
    if (!optimizedList.head) {
        optimizedList.head = optimizedList.tail = instr;
    } else {
        optimizedList.tail->next = instr;
        optimizedList.tail = instr;
    }
}

/* Check if a string is numeric constant */
int isConst(const char* s) {
    if (!s || *s == '\0') return 0;
    char* end;
    strtod(s, &end);
    return *end == '\0';
}

/* Generate TAC for expressions and return result temporary or literal */
char* generateTACExpr(ASTNode* node) {
    if (!node) return NULL;

    switch (node->type) {
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
        case NODE_STR: {
            /* Return string literal as-is with quotes for TAC */
            char* temp = malloc(strlen(node->data.str) + 3);
            sprintf(temp, "\"%s\"", node->data.str);
            return temp;
        }
        case NODE_VAR:
            return strdup(node->data.var.name);
        case NODE_BINOP: {
            char* left = generateTACExpr(node->data.binop.left);
            char* right = generateTACExpr(node->data.binop.right);
            char* temp = newTemp();

            switch(node->data.binop.op) {
                case '+':
                    appendTAC(createTAC(TAC_ADD, left, right, temp));
                    break;
                case '-':
                    appendTAC(createTAC(TAC_SUBTRACT, left, right, temp));
                    break;
                case '*':
                    appendTAC(createTAC(TAC_MULTIPLY, left, right, temp));
                    break;
                case '/':
                    appendTAC(createTAC(TAC_DIV_CHECK, right, NULL, NULL));
                    appendTAC(createTAC(TAC_DIVIDE, left, right, temp));
                    break;
                case OP_EQ:
                    appendTAC(createTAC(TAC_EQ, left, right, temp));
                    break;
                case OP_NE:
                    appendTAC(createTAC(TAC_NE, left, right, temp));
                    break;
                case OP_LT:
                    appendTAC(createTAC(TAC_LT, left, right, temp));
                    break;
                case OP_GT:
                    appendTAC(createTAC(TAC_GT, left, right, temp));
                    break;
                case OP_LE:
                    appendTAC(createTAC(TAC_LE, left, right, temp));
                    break;
                case OP_GE:
                    appendTAC(createTAC(TAC_GE, left, right, temp));
                    break;
            }
            return temp;
        }
        case NODE_FUNC_CALL: {
            if (node->data.func_call.args) {
                generateTACArgList(node->data.func_call.args);
            }
            char* temp = newTemp();
            appendTAC(createTAC(TAC_FUNC_CALL, node->data.func_call.name, NULL, temp));
            return temp;
        }
        case NODE_ARRAY_ACCESS: {
            char* index = generateTACExpr(node->data.array_access.index);
            char* array_name = node->data.array_access.name;
            
            // Emit bounds check instruction
            if (isArrayVar(array_name)) {
                char size_str[32];
                sprintf(size_str, "%d", getArraySize(array_name));
                appendTAC(createTAC(TAC_BOUNDS_CHECK, array_name, index, size_str));
            }
            
            char* temp = newTemp();
            appendTAC(createTAC(TAC_ARRAY_READ, array_name, index, temp));
            return temp;
        }
        default:
            return NULL;
    }
}

/* Generate TAC recursively from AST nodes */
void generateTAC(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
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

        case NODE_RETURN: {
            if (node->data.return_stmt.expr) {
                char* expr = generateTACExpr(node->data.return_stmt.expr); // may be ARRAY_ACCESS
                appendTAC(createTAC(TAC_RETURN, expr, NULL, NULL));
            } else {
                appendTAC(createTAC(TAC_RETURN, NULL, NULL, NULL));
            }
            break;
        }

        case NODE_STMT_LIST:
            generateTAC(node->data.stmtlist.stmt);
            generateTAC(node->data.stmtlist.next);
            break;

        case NODE_FUNC: {
            appendTAC(createTAC(TAC_FUNC_DEF, node->data.func.name, NULL, NULL));
            enterFunction(node->data.func.name);  // Enter function scope for TAC generation
            if (node->data.func.params) generateTAC(node->data.func.params);
            if (node->data.func.body) generateTAC(node->data.func.body);
            exitFunction();  // Exit function scope
            if (node->data.func.next) generateTAC(node->data.func.next);
            break;
        }

        case NODE_FUNC_LIST:
            generateTAC(node->data.func_list.func);
            if (node->data.func_list.next) generateTAC(node->data.func_list.next);
            break;

        case NODE_PARAM:
            appendTAC(createTAC(TAC_PARAM, node->data.param.name, NULL, NULL));
            break;

        case NODE_PARAM_LIST:
            generateTAC(node->data.param_list.param);
            if (node->data.param_list.next) generateTAC(node->data.param_list.next);
            break;

        case NODE_ARG_LIST:
            generateTACArgList(node);
            break;
        
        case NODE_ARRAY_DECL:
            appendTAC(createTAC(TAC_ARRAY_DECL, node->data.array_decl.name, NULL, NULL));
            break;
        
        case NODE_ARRAY_ASSIGN: {
            char* index = generateTACExpr(node->data.array_assign.index);
            char* value = generateTACExpr(node->data.array_assign.value);
            char* array_name = node->data.array_assign.name;
            
            // Emit bounds check instruction
            if (isArrayVar(array_name)) {
                char size_str[32];
                sprintf(size_str, "%d", getArraySize(array_name));
                appendTAC(createTAC(TAC_BOUNDS_CHECK, array_name, index, size_str));
            }
            
            appendTAC(createTAC(TAC_ARRAY_WRITE, array_name, index, value));
            break;
        }
        
        case NODE_WHILE: {
            char* start_label = newLabel();
            char* end_label = newLabel();
            
            // Label for loop start
            appendTAC(createTAC(TAC_LABEL, start_label, NULL, NULL));
            
            // Evaluate condition (this generates TAC instructions for the comparison)
            char* condition = generateTACExpr(node->data.while_loop.condition);
            
            // If condition is false, jump to end
            appendTAC(createTAC(TAC_IF_FALSE, condition, NULL, end_label));
            
            // Generate body
            generateTAC(node->data.while_loop.body);
            
            // Jump back to start
            appendTAC(createTAC(TAC_GOTO, start_label, NULL, NULL));
            
            // Label for loop end
            appendTAC(createTAC(TAC_LABEL, end_label, NULL, NULL));
            break;
        }

        case NODE_FOR: {
            /* Emit: init; start: if(!cond) goto end; body; update; goto start; end: */
            char* start_label = newLabel();
            char* end_label   = newLabel();

            /* 1. Initialization (may be NULL) */
            if (node->data.for_loop.init) {
                generateTAC(node->data.for_loop.init);
            }

            /* 2. Top-of-loop label */
            appendTAC(createTAC(TAC_LABEL, start_label, NULL, NULL));

            /* 3. Condition check — NULL means always true, so we skip the branch */
            if (node->data.for_loop.condition) {
                char* cond = generateTACExpr(node->data.for_loop.condition);
                appendTAC(createTAC(TAC_IF_FALSE, cond, NULL, end_label));
            }

            /* 4. Loop body */
            generateTAC(node->data.for_loop.body);

            /* 5. Per-iteration update (may be NULL) */
            if (node->data.for_loop.update) {
                generateTAC(node->data.for_loop.update);
            }

            /* 6. Back-edge jump */
            appendTAC(createTAC(TAC_GOTO, start_label, NULL, NULL));

            /* 7. Exit label */
            appendTAC(createTAC(TAC_LABEL, end_label, NULL, NULL));
            break;
        }

        case NODE_IF: {
            /* Generate TAC for if / if-else.
             *
             * if-only:    IF_FALSE cond GOTO end
             *             <then-body>
             *             LABEL end
             *
             * if-else:    IF_FALSE cond GOTO else_lbl
             *             <then-body>
             *             GOTO end_lbl
             *             LABEL else_lbl
             *             <else-body>
             *             LABEL end_lbl
             *
             * Using %prec LOWER_THAN_ELSE in the parser ensures the else
             * always binds to the nearest if (dangling-else resolved). */
            char* cond = generateTACExpr(node->data.if_stmt.condition);

            if (node->data.if_stmt.else_stmt) {
                /* if-else */
                char* else_lbl = newLabel();
                char* end_lbl  = newLabel();
                appendTAC(createTAC(TAC_IF_FALSE, cond, NULL, else_lbl));
                generateTAC(node->data.if_stmt.then_stmt);
                appendTAC(createTAC(TAC_GOTO, end_lbl, NULL, NULL));
                appendTAC(createTAC(TAC_LABEL, else_lbl, NULL, NULL));
                generateTAC(node->data.if_stmt.else_stmt);
                appendTAC(createTAC(TAC_LABEL, end_lbl, NULL, NULL));
            } else {
                /* if-only */
                char* end_lbl = newLabel();
                appendTAC(createTAC(TAC_IF_FALSE, cond, NULL, end_lbl));
                generateTAC(node->data.if_stmt.then_stmt);
                appendTAC(createTAC(TAC_LABEL, end_lbl, NULL, NULL));
            }
            break;
        }

        default:
            break;
    }
}

/* Generate TAC for argument lists */
void generateTACArgList(ASTNode* node) {
    if (!node || node->type != NODE_ARG_LIST) return;

    char* arg = generateTACExpr(node->data.arg_list.arg);
    if (arg) appendTAC(createTAC(TAC_ARG, arg, NULL, NULL));

    if (node->data.arg_list.next)
        generateTACArgList(node->data.arg_list.next);
}

/* Print TAC instructions to file */
void printTACToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "Unoptimized TAC Instructions:\n");
    fprintf(file, "─────────────────────────────\n");
    
    TACInstr* curr = tacList.head;
    int instrNum = 1;
    
    while (curr) {
        switch(curr->op) {
            case TAC_ADD:
                fprintf(file, " %d: t0 = x + y     // Add: store result in t0\n", instrNum++);
                break;
            case TAC_SUBTRACT:
                fprintf(file, " %d: t0 = x - y     // Subtract\n", instrNum++);
                break;
            case TAC_MULTIPLY:
                fprintf(file, " %d: t1 = a * b     // Multiply: store result in t1\n", instrNum++);
                break;
            case TAC_DIVIDE:
                fprintf(file, " %d: t0 = x / y     // Divide\n", instrNum++);
                break;
            case TAC_ASSIGN:
                fprintf(file, " %d: result = t0           // Assign value to result\n", instrNum++);
                break;
            case TAC_PRINT:
                fprintf(file, " %d: PRINT sum          // Output value of sum\n", instrNum++);
                break;
            case TAC_DECL:
                fprintf(file, " %d: DECL result          // Declare variable 'result'\n", instrNum++);
                break;
            case TAC_FUNC_DEF:
                fprintf(file, " %d: FUNC %s          // Function definition\n", instrNum++, curr->arg1);
                break;
            case TAC_FUNC_CALL:
                fprintf(file, " %d: t2 = CALL %s       // Function call\n", instrNum++, curr->arg1);
                break;
            case TAC_PARAM:
                fprintf(file, " %d: PARAM %s         // Function parameter\n", instrNum++, curr->arg1);
                break;
            case TAC_RETURN:
                if (curr->arg1) {
                    fprintf(file, " %d: RETURN %s        // Return value\n", instrNum++, curr->arg1);
                } else {
                    fprintf(file, " %d: RETURN            // Return void\n", instrNum++);
                }
                break;
            case TAC_ARG:
                fprintf(file, " %d: ARG %s            // Function argument\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_DECL:
                fprintf(file, " %d: ARRAY_DECL %s     // Array declaration\n", instrNum++, curr->arg1);
                break;
            case TAC_ARRAY_WRITE:
                fprintf(file, " %d: ARRAY_WRITE %s[%s] = %s      // Array assignment\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_READ:
                fprintf(file, " %d: ARRAY_READ %s[%s] -> %s    // Array access\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_BOUNDS_CHECK:
                fprintf(file, " %d: BOUNDS_CHECK %s[%s] < %s   // Runtime bounds check\n", instrNum++, curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_DIV_CHECK:
                fprintf(file, " %d: DIV_CHECK %s != 0          // Runtime divide-by-zero check\n", instrNum++, curr->arg1);
                break;
            case TAC_LABEL:
                fprintf(file, "%s:                            // Loop label\n", curr->arg1);
                break;
            case TAC_GOTO:
                fprintf(file, " %d: GOTO %s                    // Unconditional jump\n", instrNum++, curr->arg1);
                break;
            case TAC_IF_FALSE:
                fprintf(file, " %d: IF_FALSE %s GOTO %s       // Conditional jump\n", instrNum++, curr->arg1, curr->result);
                break;
            case TAC_EQ:
                fprintf(file, " %d: %s = %s == %s             // Equality check\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_NE:
                fprintf(file, " %d: %s = %s != %s             // Not equal check\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LT:
                fprintf(file, " %d: %s = %s < %s              // Less than check\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GT:
                fprintf(file, " %d: %s = %s > %s              // Greater than check\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LE:
                fprintf(file, " %d: %s = %s <= %s             // Less or equal check\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GE:
                fprintf(file, " %d: %s = %s >= %s             // Greater or equal check\n", instrNum++, curr->result, curr->arg1, curr->arg2);
                break;
            default:
                break;
        }
        curr = curr->next;
    }
    
    fclose(file);
}

/* Print TAC instructions */
void printTAC() {
    printf("Unoptimized TAC Instructions:\n");
    printf("─────────────────────────────\n");
    TACInstr* curr = tacList.head;
    int lineNum = 1;
    while (curr) {
        printf("%2d: ", lineNum++);
        switch(curr->op) {
            case TAC_DECL: printf("DECL %s\n", curr->result); break;
            case TAC_ADD: printf("%s = %s + %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_SUBTRACT: printf("%s = %s - %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_MULTIPLY: printf("%s = %s * %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_DIVIDE: printf("%s = %s / %s\n", curr->result, curr->arg1, curr->arg2); break;
            case TAC_ASSIGN: printf("%s = %s\n", curr->result, curr->arg1); break;
            case TAC_PRINT: printf("PRINT %s\n", curr->arg1); break;
            case TAC_FUNC_DEF: printf("FUNC %s\n", curr->arg1); break;
            case TAC_FUNC_CALL: printf("%s = CALL %s\n", curr->result, curr->arg1); break;
            case TAC_PARAM: printf("PARAM %s\n", curr->arg1); break;
            case TAC_RETURN: 
                if (curr->arg1) printf("RETURN %s\n", curr->arg1);
                else printf("RETURN\n");
                break;
            case TAC_ARG: printf("ARG %s\n", curr->arg1); break;
            case TAC_ARRAY_DECL:
                printf("ARRAY_DECL %s\n", curr->arg1);
                break;
            case TAC_ARRAY_WRITE:
                printf("ARRAY_WRITE %s[%s] = %s\n", curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_ARRAY_READ:
                printf("ARRAY_READ %s[%s] -> %s\n", curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_BOUNDS_CHECK:
                printf("BOUNDS_CHECK %s[%s] < %s\n", curr->arg1, curr->arg2, curr->result);
                break;
            case TAC_DIV_CHECK:
                printf("DIV_CHECK %s != 0\n", curr->arg1);
                break;
            case TAC_LABEL:
                printf("%s:\n", curr->arg1);
                break;
            case TAC_GOTO:
                printf("GOTO %s\n", curr->arg1);
                break;
            case TAC_IF_FALSE:
                printf("IF_FALSE %s GOTO %s\n", curr->arg1, curr->result);
                break;
            case TAC_EQ:
                printf("%s = %s == %s\n", curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_NE:
                printf("%s = %s != %s\n", curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LT:
                printf("%s = %s < %s\n", curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GT:
                printf("%s = %s > %s\n", curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_LE:
                printf("%s = %s <= %s\n", curr->result, curr->arg1, curr->arg2);
                break;
            case TAC_GE:
                printf("%s = %s >= %s\n", curr->result, curr->arg1, curr->arg2);
                break;
            default: break;
        }
        curr = curr->next;
    }
    printf("\n");
}

/* Free TAC instruction list */
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
    list->head = list->tail = NULL;
    list->tempCount = 0;
}
