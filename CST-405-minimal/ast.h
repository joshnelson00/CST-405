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
    NODE_ARRAY_DECL, /* Array declaration (e.g., int arr[10]) */
    NODE_ARRAY_ACCESS, /* Array access (e.g., arr[5]) */
    NODE_ASSIGN,    /* Assignment statement (e.g., x = 10) */
    NODE_ARRAY_ASSIGN, /* Array assignment (e.g., arr[i] = 10) */
    NODE_PRINT,     /* Print statement (e.g., print(x)) */
    NODE_RETURN,    /* Return statement (e.g., return x) */
    NODE_FUNC,      /* Function definition (e.g., int f() {}) */
    NODE_PARAM,     /* Function parameter (e.g., int x) */
    NODE_PARAM_LIST, /* Parameter list (e.g., int x, float y) */
    NODE_FUNC_CALL, /* Function call (e.g., f(x, y)) */
    NODE_ARG_LIST, /* Argument list (e.g., x, y) */
    NODE_STMT_LIST,  /* List of statements (program structure) */
    NODE_FUNC_LIST  /* List of functions (program structure) */
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
        
        /* Array declaration (NODE_ARRAY_DECL) */
        struct {
            char* name;              /* Array name */
            VarType type;            /* Element type */
            int size;                /* Array size */
        } array_decl;
        
        /* Array access (NODE_ARRAY_ACCESS) */
        struct {
            char* name;              /* Array name */
            struct ASTNode* index;   /* Index expression */
        } array_access;
        
        /* Array assignment (NODE_ARRAY_ASSIGN) */
        struct {
            char* name;              /* Array name */
            struct ASTNode* index;   /* Index expression */
            struct ASTNode* value;   /* Value to assign */
        } array_assign;

        
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
        
        /* Return expression (NODE_RETURN) */
        struct {
            struct ASTNode* expr;       /* Expression to return (NULL for void) */
        } return_stmt;
        
        /* Function definition (NODE_FUNC) */
        struct {
            char* name;                  /* Function name */
            VarType return_type;         /* Return type */
            struct ASTNode* params;      /* Parameter list (NULL if none) */
            struct ASTNode* body;        /* Function body (statement list) */
            struct ASTNode* next;        /* Next function in program */
        } func;
        
        /* Function list (NODE_FUNC_LIST) - Wrapper for multiple functions */
        struct {
            struct ASTNode* func;       /* Current function */
            struct ASTNode* next;       /* Next function in list */
        } func_list;
        
        /* Function parameter (NODE_PARAM) */
        struct {
            char* name;                  /* Parameter name */
            VarType type;                /* Parameter type */
            int is_array;                /* 1 if array parameter */
        } param;
        
        /* Parameter list (NODE_PARAM_LIST) */
        struct {
            struct ASTNode* param;      /* Current parameter */
            struct ASTNode* next;        /* Next parameter (NULL if last) */
        } param_list;
        
        /* Statement list structure (NODE_STMT_LIST) */
        struct {
            struct ASTNode* stmt;       /* Current statement */
            struct ASTNode* next;       /* Rest of the list */
        } stmtlist;
        
        /* Function call structure (NODE_FUNC_CALL) */
        struct {
            char* name;                  /* Function name */
            struct ASTNode* args;        /* Argument list (NULL if no args) */
        } func_call;
        
        /* Argument list structure (NODE_ARG_LIST) */
        struct {
            struct ASTNode* arg;        /* Current argument */
            struct ASTNode* next;        /* Next argument (NULL if last) */
        } arg_list;
    } data;
} ASTNode;

/* AST CONSTRUCTION FUNCTIONS
 * These functions are called by the parser to build the tree
 */
ASTNode* createNum(int value);                                   /* Create number node */
ASTNode* createFlt(double value);                                   /* Create float node */ 
ASTNode* createVar(char* name);                                  /* Create variable node */
ASTNode* createBinOp(char op, ASTNode* left, ASTNode* right);   /* Create binary op node */
ASTNode* createDecl(char* name, VarType type);                              /* Create declaration node */
ASTNode* createArrayDecl(char* name, VarType type, int size);   /* Create array declaration node */
ASTNode* createArrayAccess(char* name, ASTNode* index);         /* Create array access node */
ASTNode* createArrayAssign(char* name, ASTNode* index, ASTNode* value); /* Create array assignment node */
ASTNode* createAssign(char* var, ASTNode* value);               /* Create assignment node */
ASTNode* createPrint(ASTNode* expr);                            /* Create print node */
ASTNode* createReturn(ASTNode* expr);                           /* Create return node */
ASTNode* createFunc(char* name, VarType return_type, ASTNode* params, ASTNode* body);  /* Create function node */
ASTNode* createParam(char* name, VarType type);                  /* Create parameter node */
ASTNode* createArrayParam(char* name, VarType type);             /* Create array parameter node */
ASTNode* createParamList(ASTNode* param, ASTNode* next);         /* Create parameter list */
ASTNode* createFuncCall(char* name, ASTNode* args);              /* Create function call node */
ASTNode* createArgList(ASTNode* arg, ASTNode* next);             /* Create argument list */
ASTNode* createStmtList(ASTNode* stmt1, ASTNode* stmt2);        /* Create statement list */
ASTNode* createFuncList(ASTNode* func1, ASTNode* func2);         /* Create function list */    

/* AST DISPLAY FUNCTION */
void printAST(ASTNode* node, int level);                        /* Pretty-print the AST */

#endif
