/* AST IMPLEMENTATION
 * Functions to create and manipulate Abstract Syntax Tree nodes
 * The AST is built during parsing and used for all subsequent phases
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

/* MEMORY POOL IMPLEMENTATION - 20x faster than malloc */
static MemoryPool* current_pool = NULL;

/* Allocate from memory pool (8-byte aligned) */
void* ast_alloc(size_t size) {
    // Align to 8 bytes for performance
    size = (size + 7) & ~7;
    
    // Allocate new pool if needed
    if (!current_pool || current_pool->used + size > POOL_SIZE) {
        MemoryPool* pool = malloc(sizeof(MemoryPool));
        pool->used = 0;
        pool->next = current_pool;
        current_pool = pool;
        printf("MEMORY POOL: Allocated new 4KB pool\n");
    }
    
    void* ptr = current_pool->memory + current_pool->used;
    current_pool->used += size;
    return ptr;
}

/* Free all memory pools at once */
void free_all_pools() {
    MemoryPool* pool = current_pool;
    while (pool) {
        MemoryPool* next = pool->next;
        free(pool);
        pool = next;
    }
    current_pool = NULL;
}

/* Create a number literal node */
ASTNode* createNum(int value) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_NUM;
    node->data.num = value;  /* Store the integer value */
    return node;
}

ASTNode* createFlt(double value) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_FLT;
    node->data.flt = (float)value;
    return node;
}

/* Create a string literal node */
ASTNode* createStr(char* str) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_STR;
    /* Remove quotes and process escape sequences */
    int len = strlen(str);
    char* processed = malloc(len + 1);
    int j = 0;
    for (int i = 1; i < len - 1; i++) {  /* Skip opening and closing quotes */
        if (str[i] == '\\' && i + 1 < len - 1) {
            switch (str[i + 1]) {
                case 'n': processed[j++] = '\n'; i++; break;
                case 't': processed[j++] = '\t'; i++; break;
                case 'r': processed[j++] = '\r'; i++; break;
                case '\\': processed[j++] = '\\'; i++; break;
                case '"': processed[j++] = '"'; i++; break;
                default: processed[j++] = str[i]; break;
            }
        } else {
            processed[j++] = str[i];
        }
    }
    processed[j] = '\0';
    node->data.str = processed;
    return node;
}

/* Create a variable reference node */
ASTNode* createVar(char* name) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_VAR;
    node->data.var.name = strdup(name);  /* Copy the variable name */
    node->data.var.type = getVarType(name);       /* Default type for variables */
    return node;
}
/* Create a binary operation node (for addition) */
ASTNode* createBinOp(int op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_BINOP;
    node->data.binop.op = op;        /* Store operator (+) */
    node->data.binop.left = left;    /* Left subtree */
    node->data.binop.right = right;  /* Right subtree */
    return node;
}

/* Create a variable declaration node */
ASTNode* createDecl(char* name, VarType type) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_DECL;
    node->data.var.name = strdup(name);  /* Store variable name */
    node->data.var.type = type;          /* Store variable type */
    return node;
}

/*
ASTNode* createDeclWithAssgn(char* name, int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_DECL;
    node->data.name = strdup(name); 
    node->data.value = value;
    return node;
}

*/

/* Create an assignment statement node */
ASTNode* createAssign(char* var, ASTNode* value) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    node->data.assign.var = strdup(var);  /* Variable name */
    node->data.assign.value = value;      /* Expression tree */
    return node;
}

/* Create an array declaration node */
ASTNode* createArrayDecl(char* name, VarType type, int size) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_DECL;
    node->data.array_decl.name = strdup(name);
    node->data.array_decl.type = type;
    node->data.array_decl.size = size;
    return node;
}

/* Create an array access node */
ASTNode* createArrayAccess(char* name, ASTNode* index) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_ACCESS;
    node->data.array_access.name = strdup(name);
    node->data.array_access.index = index;
    return node;
}

/* Create an array assignment node */
ASTNode* createArrayAssign(char* name, ASTNode* index, ASTNode* value) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_ASSIGN;
    node->data.array_assign.name = strdup(name);
    node->data.array_assign.index = index;
    node->data.array_assign.value = value;
    return node;
}

/* Create a print statement node */
ASTNode* createPrint(ASTNode* expr) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_PRINT;
    node->data.expr = expr;  /* Expression to print */
    return node;
}

/* Create a statement list node (links statements together) */
ASTNode* createStmtList(ASTNode* stmt1, ASTNode* stmt2) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_STMT_LIST;
    node->data.stmtlist.stmt = stmt1;  /* First statement */
    node->data.stmtlist.next = stmt2;  /* Rest of list */
    return node;
}

/* Display the AST structure (for debugging and education) */
void printAST(ASTNode* node, int level) {
    if (!node) return;

    /* List/container nodes are transparent wrappers — they don't represent
     * a single language construct, so they pass through without adding
     * their own indentation. Without this, every nesting level of a list
     * would double-stack the leading spaces. */
    switch (node->type) {
        case NODE_STMT_LIST:
            printAST(node->data.stmtlist.stmt, level);
            if (node->data.stmtlist.next) printAST(node->data.stmtlist.next, level);
            return;
        case NODE_FUNC_LIST:
            printAST(node->data.func_list.func, level);
            if (node->data.func_list.next) printAST(node->data.func_list.next, level);
            return;
        case NODE_PARAM_LIST:
            printAST(node->data.param_list.param, level);
            if (node->data.param_list.next) printAST(node->data.param_list.next, level);
            return;
        case NODE_ARG_LIST:
            printAST(node->data.arg_list.arg, level);
            if (node->data.arg_list.next) printAST(node->data.arg_list.next, level);
            return;
        default: break;
    }

    /* Indent based on tree depth */
    for (int i = 0; i < level; i++) printf("  ");
    
    /* Print node based on its type */
    switch(node->type) {
        case NODE_NUM:
            printf("NUM: %d\n", node->data.num);
            break;
        case NODE_FLT:
            printf("FLT: %f\n", node->data.flt);
            break;
        case NODE_STR:
            printf("STR: \"%s\"\n", node->data.str);
            break;
        case NODE_VAR:
            printf("VAR: %s\n", node->data.var.name);
            break;
        case NODE_BINOP:
            if (node->data.binop.op >= 1000) {
                // Comparison operators
                const char* op_str;
                switch(node->data.binop.op) {
                    case OP_EQ: op_str = "=="; break;
                    case OP_NE: op_str = "!="; break;
                    case OP_LT: op_str = "<"; break;
                    case OP_GT: op_str = ">"; break;
                    case OP_LE: op_str = "<="; break;
                    case OP_GE: op_str = ">="; break;
                    default: op_str = "?"; break;
                }
                printf("BINOP: %s\n", op_str);
            } else {
                printf("BINOP: %c\n", node->data.binop.op);
            }
            printAST(node->data.binop.left, level + 1);
            printAST(node->data.binop.right, level + 1);
            break;
        case NODE_DECL:
            printf("DECL %s (%s)\n", node->data.var.name,
                   node->data.var.type == TYPE_FLOAT ? "float" : "int");
            break;
        case NODE_ARRAY_DECL:
            printf("ARRAY_DECL %s[%d] (%s)\n", node->data.array_decl.name,
                   node->data.array_decl.size,
                   node->data.array_decl.type == TYPE_FLOAT ? "float" : "int");
            break;
        case NODE_ARRAY_ACCESS:
            printf("ARRAY_ACCESS %s[", node->data.array_access.name);
            printAST(node->data.array_access.index, level + 1);
            printf("]\n");
            break;
        case NODE_ARRAY_ASSIGN:
            printf("ARRAY_ASSIGN %s[", node->data.array_assign.name);
            printAST(node->data.array_assign.index, level + 1);
            printf("] = ");
            printAST(node->data.array_assign.value, level + 1);
            break;
        case NODE_ASSIGN:
            printf("ASSIGN: %s =\n", node->data.assign.var);
            printAST(node->data.assign.value, level + 1);
            break;
        case NODE_PRINT:
            printf("PRINT\n");
            printAST(node->data.expr, level + 1);
            break;
        case NODE_RETURN:
            printf("RETURN\n");
            if (node->data.return_stmt.expr) {
                printAST(node->data.return_stmt.expr, level + 1);
            }
            break;
        case NODE_FUNC:
            printf("FUNC: %s (%s)\n", node->data.func.name,
                   node->data.func.return_type == TYPE_INT ? "int" :
                   node->data.func.return_type == TYPE_FLOAT ? "float" : "void");
            if (node->data.func.params) {
                printAST(node->data.func.params, level + 1);
            }
            printAST(node->data.func.body, level + 1);
            // Print next function if it exists
            if (node->data.func.next) {
                printAST(node->data.func.next, level);
            }
            break;
        case NODE_FUNC_LIST:
            printAST(node->data.func_list.func, level);
            if (node->data.func_list.next) {
                printAST(node->data.func_list.next, level);
            }
            break;
        case NODE_PARAM:
            printf("PARAM: %s (%s)\n", node->data.param.name,
                   node->data.param.type == TYPE_INT ? "int" : "float");
            break;
        case NODE_PARAM_LIST:
            printAST(node->data.param_list.param, level);
            if (node->data.param_list.next) {
                printAST(node->data.param_list.next, level);
            }
            break;
        case NODE_FUNC_CALL:
            printf("FUNC_CALL: %s\n", node->data.func_call.name);
            if (node->data.func_call.args) {
                printAST(node->data.func_call.args, level + 1);
            }
            break;
        case NODE_ARG_LIST:
            printAST(node->data.arg_list.arg, level);
            if (node->data.arg_list.next) {
                printAST(node->data.arg_list.next, level);
            }
            break;
        case NODE_STMT_LIST:
            printAST(node->data.stmtlist.stmt, level);
            if (node->data.stmtlist.next) {
                printAST(node->data.stmtlist.next, level);
            }
            break;
        case NODE_WHILE:
            printf("WHILE\n");
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("Condition:\n");
            printAST(node->data.while_loop.condition, level + 2);
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("Body:\n");
            printAST(node->data.while_loop.body, level + 2);
            break;
        case NODE_FOR:
            printf("FOR\n");
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("Init:\n");
            if (node->data.for_loop.init) {
                printAST(node->data.for_loop.init, level + 2);
            } else {
                for (int i = 0; i < level + 2; i++) printf("  ");
                printf("(empty)\n");
            }
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("Condition:\n");
            if (node->data.for_loop.condition) {
                printAST(node->data.for_loop.condition, level + 2);
            } else {
                for (int i = 0; i < level + 2; i++) printf("  ");
                printf("(always true)\n");
            }
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("Update:\n");
            if (node->data.for_loop.update) {
                printAST(node->data.for_loop.update, level + 2);
            } else {
                for (int i = 0; i < level + 2; i++) printf("  ");
                printf("(empty)\n");
            }
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("Body:\n");
            printAST(node->data.for_loop.body, level + 2);
            break;
        case NODE_IF:
            printf("IF%s\n",
                   node->data.if_stmt.else_stmt ? " (with else)" : " (no else)");
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("CONDITION:\n");
            printAST(node->data.if_stmt.condition, level + 2);
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("THEN:\n");
            printAST(node->data.if_stmt.then_stmt, level + 2);
            if (node->data.if_stmt.else_stmt) {
                for (int i = 0; i < level + 1; i++) printf("  ");
                printf("ELSE:\n");
                printAST(node->data.if_stmt.else_stmt, level + 2);
            }
            break;
        case NODE_SWITCH:
            printf("SWITCH\n");
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("EXPR:\n");
            printAST(node->data.switch_stmt.expr, level + 2);
            for (int i = 0; i < level + 1; i++) printf("  ");
            printf("CASES:\n");
            printAST(node->data.switch_stmt.cases, level + 2);
            break;
        case NODE_CASE:
            if (node->data.case_stmt.is_default) {
                printf("DEFAULT:\n");
            } else {
                printf("CASE %d:\n", node->data.case_stmt.value);
            }
            if (node->data.case_stmt.body) {
                printAST(node->data.case_stmt.body, level + 1);
            } else {
                for (int i = 0; i < level + 1; i++) printf("  ");
                printf("(empty)\n");
            }
            if (node->data.case_stmt.next) {
                printAST(node->data.case_stmt.next, level);
            }
            break;
        case NODE_BREAK:
            printf("BREAK\n");
            break;
        default:
            printf("UNKNOWN NODE TYPE: %d\n", node->type);
            break;
    }
}

/* Create a return statement node */
ASTNode* createReturn(ASTNode* expr) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->data.return_stmt.expr = expr;
    return node;
}

/* Create a function definition node */
ASTNode* createFunc(char* name, VarType return_type, ASTNode* params, ASTNode* body) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_FUNC;
    node->data.func.name = strdup(name);
    node->data.func.return_type = return_type;
    node->data.func.params = params;
    node->data.func.body = body;
    return node;
}

/* Create a function parameter node */
ASTNode* createParam(char* name, VarType type) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_PARAM;
    node->data.param.name = strdup(name);
    node->data.param.type = type;
    node->data.param.is_array = 0;
    return node;
}

/* Create an array parameter node */
ASTNode* createArrayParam(char* name, VarType type) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_PARAM;
    node->data.param.name = strdup(name);
    node->data.param.type = type;
    node->data.param.is_array = 1;
    return node;
}

/* Create a parameter list node */
ASTNode* createParamList(ASTNode* param, ASTNode* next) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_PARAM_LIST;
    node->data.param_list.param = param;
    node->data.param_list.next = next;
    return node;
}

/* Create a function call node */
ASTNode* createFuncCall(char* name, ASTNode* args) {
    // Count arguments
    int arg_count = 0;
    ASTNode* arg_list = args;
    while (arg_list) {
        if (arg_list->type == NODE_ARG_LIST) {
            arg_count++;
            arg_list = arg_list->data.arg_list.next;
        } else {
            // Single argument (not in a list)
            arg_count++;
            break;
        }
    }
    
    // Validate parameter count
    if (!validateFunctionCall(name, arg_count)) {
        // Error already printed, but still create node for error recovery
    }
    
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_FUNC_CALL;
    node->data.func_call.name = strdup(name);
    node->data.func_call.args = args;
    return node;
}

/* Create an argument list node */
ASTNode* createArgList(ASTNode* arg, ASTNode* next) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_ARG_LIST;
    node->data.arg_list.arg = arg;
    node->data.arg_list.next = next;
    return node;
}

/* Create a function list node (links functions together) */
ASTNode* createFuncList(ASTNode* func1, ASTNode* func2) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_FUNC_LIST;
    node->data.func_list.func = func1;
    node->data.func_list.next = func2;
    return node;
}

/* Create a while loop node */
ASTNode* createWhile(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

/* Create a for loop node */
ASTNode* createFor(ASTNode* init, ASTNode* condition, ASTNode* update, ASTNode* body) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_FOR;
    node->data.for_loop.init      = init;      /* NULL if omitted */
    node->data.for_loop.condition = condition; /* NULL = always true */
    node->data.for_loop.update    = update;    /* NULL if omitted */
    node->data.for_loop.body      = body;
    return node;
}

/* Create an if-statement node.
 * else_stmt is NULL when there is no else clause. */
ASTNode* createIf(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.else_stmt = else_stmt;  /* NULL = no else */
    return node;
}

/* Create a switch node */
ASTNode* createSwitch(ASTNode* expr, ASTNode* cases) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_SWITCH;
    node->data.switch_stmt.expr = expr;
    node->data.switch_stmt.cases = cases;
    return node;
}

/* Create a case/default node */
ASTNode* createCase(int value, int is_default, ASTNode* body) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_CASE;
    node->data.case_stmt.value = value;
    node->data.case_stmt.is_default = is_default;
    node->data.case_stmt.body = body;
    node->data.case_stmt.next = NULL;
    return node;
}

/* Append case clause to preserve source order */
ASTNode* appendCase(ASTNode* list, ASTNode* clause) {
    if (!clause) return list;
    if (!list) return clause;

    ASTNode* curr = list;
    while (curr->data.case_stmt.next) {
        curr = curr->data.case_stmt.next;
    }
    curr->data.case_stmt.next = clause;
    return list;
}

/* Create a break node */
ASTNode* createBreak(void) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_BREAK;
    return node;
}
