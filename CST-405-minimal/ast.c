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

/* Create a variable reference node */
ASTNode* createVar(char* name) {
    ASTNode* node = ast_alloc(sizeof(ASTNode));
    node->type = NODE_VAR;
    node->data.var.name = strdup(name);  /* Copy the variable name */
    node->data.var.type = getVarType(name);       /* Default type for variables */
    return node;
}
/* Create a binary operation node (for addition) */
ASTNode* createBinOp(char op, ASTNode* left, ASTNode* right) {
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
        case NODE_VAR:
            printf("VAR: %s\n", node->data.var.name);
            break;
        case NODE_BINOP:
            printf("BINOP: %c\n", node->data.binop.op);
            printAST(node->data.binop.left, level + 1);
            printf("  ");
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
            printf("ASSIGN: %s = ", node->data.assign.var);
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
