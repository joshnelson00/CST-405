#ifndef AST_H
#define AST_H

#include "symtab.h"  /* Include for VarType definition */

/* ABSTRACT SYNTAX TREE (AST)
 * The AST is an intermediate representation of the program structure
 * It represents the hierarchical syntax of the source code
 * Each node represents a construct in the language
 */

/* NODE TYPES - Different kinds of AST nodes in our language */
typedef enum {
    NODE_NUM,       /* Numeric literal (e.g., 42) */
    NODE_FLT,       /* Numeric float (e.g., 43.46) */
    NODE_VAR,       /* Variable reference (e.g., x) */
    NODE_BINOP,     /* Binary operation (e.g., x + y) */
    NODE_DECL,      /* Variable declaration (e.g., int x) */
    NODE_ASSIGN,    /* Assignment statement (e.g., x = 10) */
    NODE_PRINT,     /* Print statement (e.g., print(x)) */
    NODE_STMT_LIST  /* List of statements (program structure) */
} NodeType;

/* AST NODE STRUCTURE
 * Uses a union to efficiently store different node data
 * Only the relevant fields for each node type are used
 */
typedef struct ASTNode {
    NodeType type;  /* Identifies what kind of node this is */
    
    /* Union allows same memory to store different data types */
    union {
        /* Literal number value (NODE_NUM) */
        int num;

        float flt;
        
        /* Variable or declaration name (NODE_VAR, NODE_DECL) */
        struct {
            char* name;              /* Variable name */
            VarType type;            /* Variable type (for declarations) */
        } var;
        /* int value; */ /* For potential future use in declarations with assignment */

        
        /* Binary operation structure (NODE_BINOP) */
        struct {
            char op;                    /* Operator character ('+') */
            struct ASTNode* left;       /* Left operand */
            struct ASTNode* right;      /* Right operand */
        } binop;
        
        /* Assignment structure (NODE_ASSIGN) */
        struct {
            char* var;                  /* Variable being assigned to */
            struct ASTNode* value;      /* Expression being assigned */
        } assign;
        
        /* Print expression (NODE_PRINT) */
        struct ASTNode* expr;
        
        /* Statement list structure (NODE_STMT_LIST) */
        struct {
            struct ASTNode* stmt;       /* Current statement */
            struct ASTNode* next;       /* Rest of the list */
        } stmtlist;
    } data;
} ASTNode;

/* AST CONSTRUCTION FUNCTIONS
 * These functions are called by the parser to build the tree
 */
void init_ast_memory();                                         /* Initialize AST memory pool */
ASTNode* createNum(int value);                                   /* Create number node */
ASTNode* createFlt(double value);                                   /* Create float node */ 
ASTNode* createVar(char* name);                                  /* Create variable node */
ASTNode* createBinOp(char op, ASTNode* left, ASTNode* right);   /* Create binary op node */
ASTNode* createDecl(char* name, VarType type);                              /* Create declaration node */
ASTNode* createAssign(char* var, ASTNode* value);               /* Create assignment node */
ASTNode* createPrint(ASTNode* expr);                            /* Create print node */
ASTNode* createStmtList(ASTNode* stmt1, ASTNode* stmt2);        /* Create statement list */

/* AST DISPLAY FUNCTION */
void printAST(ASTNode* node, int level);                        /* Pretty-print the AST */

#endif
