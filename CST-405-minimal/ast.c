/* AST IMPLEMENTATION
 * Functions to create and manipulate Abstract Syntax Tree nodes
 * The AST is built during parsing and used for all subsequent phases
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "stringpool.h"

// Memory pool structure
#define POOL_SIZE 4096  // Allocate in 4KB chunks
#define MAX_POOLS 100

typedef struct MemPool {
    char* memory;
    size_t used;
    size_t size;
    struct MemPool* next;
} MemPool;

typedef struct {
    MemPool* current;
    MemPool* head;
    int total_allocations;
    int pool_count;
    size_t total_memory;
} ASTMemoryManager;

static ASTMemoryManager ast_mem = {0};

// Initialize memory pool
void init_ast_memory() {
    ast_mem.head = malloc(sizeof(MemPool));
    ast_mem.head->memory = malloc(POOL_SIZE);
    ast_mem.head->used = 0;
    ast_mem.head->size = POOL_SIZE;
    ast_mem.head->next = NULL;
    ast_mem.current = ast_mem.head;
    ast_mem.pool_count = 1;
    ast_mem.total_memory = POOL_SIZE;
}

// Allocate from pool
static void* ast_alloc(size_t size) {
    // Lazy init if caller didn't initialize
    if (!ast_mem.current) {
        init_ast_memory();
    }

    // Align to 8 bytes for better performance
    size = (size + 7) & ~7;

    if (ast_mem.current->used + size > ast_mem.current->size) {
        if (ast_mem.pool_count >= MAX_POOLS) {
            return NULL;
        }

        // Need new pool
        MemPool* new_pool = malloc(sizeof(MemPool));
        new_pool->memory = malloc(POOL_SIZE);
        new_pool->used = 0;
        new_pool->size = POOL_SIZE;
        new_pool->next = NULL;

        ast_mem.current->next = new_pool;
        ast_mem.current = new_pool;
        ast_mem.pool_count++;
        ast_mem.total_memory += POOL_SIZE;
    }

    void* ptr = ast_mem.current->memory + ast_mem.current->used;
    ast_mem.current->used += size;
    ast_mem.total_allocations++;

    return ptr;
}

/* Create a number literal node */
ASTNode* createNum(int value) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_NUM;
    node->data.num = value;  /* Store the integer value */
    return node;
}

ASTNode* createFlt(double value) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_FLT;
    node->data.flt = (float)value;
    return node;
}

/* Create a variable reference node */
ASTNode* createVar(char* name) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_VAR;
    node->data.var.name = intern_string(name);  /* Intern the variable name */
    node->data.var.type = getVarType(name);       /* Default type for variables */
    return node;
}
/* Create a binary operation node (for addition) */
ASTNode* createBinOp(char op, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_BINOP;
    node->data.binop.op = op;        /* Store operator (+) */
    node->data.binop.left = left;    /* Left subtree */
    node->data.binop.right = right;  /* Right subtree */
    return node;
}

/* Create a variable declaration node */
ASTNode* createDecl(char* name, VarType type) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_DECL;
    node->data.var.name = intern_string(name);  /* Intern variable name */
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
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_ASSIGN;
    node->data.assign.var = intern_string(var);  /* Intern variable name */
    node->data.assign.value = value;      /* Expression tree */
    return node;
}

/* Create a print statement node */
ASTNode* createPrint(ASTNode* expr) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = NODE_PRINT;
    node->data.expr = expr;  /* Expression to print */
    return node;
}

/* Create a statement list node (links statements together) */
ASTNode* createStmtList(ASTNode* stmt1, ASTNode* stmt2) {
    ASTNode* node = malloc(sizeof(ASTNode));
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
            printAST(node->data.binop.right, level + 1);
            break;
        case NODE_DECL:
            printf("DECL: %s (%s)\n", node->data.var.name, 
                   node->data.var.type == TYPE_FLOAT ? "float" : "int");
            break;
        case NODE_ASSIGN:
            printf("ASSIGN: %s\n", node->data.assign.var);
            printAST(node->data.assign.value, level + 1);
            break;
        case NODE_PRINT:
            printf("PRINT\n");
            printAST(node->data.expr, level + 1);
            break;
        case NODE_STMT_LIST:
            /* Print statements in sequence at same level */
            printAST(node->data.stmtlist.stmt, level);
            printAST(node->data.stmtlist.next, level);
            break;
    }
}
