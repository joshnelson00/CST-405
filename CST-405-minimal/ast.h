#ifndef AST_H
#define AST_H

#include "symtab.h"  /* Include for VarType definition */

/* MEMORY POOL FOR AST NODES - Performance optimization */
#define POOL_SIZE 4096  /* 4KB chunks */

typedef struct MemoryPool {
    char memory[POOL_SIZE];
    size_t used;
    struct MemoryPool* next;
} MemoryPool;

/* ABSTRACT SYNTAX TREE (AST)
 * The AST is an intermediate representation of the program structure
 * It represents the hierarchical syntax of the source code
 * Each node represents a construct in the language
 */

/* NODE TYPES - Different kinds of AST nodes in our language */
typedef enum {
    NODE_NUM,       /* Numeric literal (e.g., 42) */
    NODE_FLT,       /* Numeric float (e.g., 43.46) */
    NODE_STR,       /* String literal (e.g., "hello\n") */
    NODE_VAR,       /* Variable reference (e.g., x) */
    NODE_BINOP,     /* Binary operation (e.g., x + y) */
    NODE_DECL,      /* Variable declaration (e.g., int x) */
    NODE_ARRAY_DECL, /* Array declaration (e.g., int arr[10]) */
    NODE_ARRAY_ACCESS, /* Array access (e.g., arr[5]) */
    NODE_ASSIGN,    /* Assignment statement (e.g., x = 10) */
    NODE_ARRAY_ASSIGN, /* Array assignment (e.g., arr[i] = 10) */
    NODE_PRINT,     /* Print statement (e.g., print(x)) */
    NODE_WRITE,     /* Write statement (e.g., write(x)) */
    NODE_RETURN,    /* Return statement (e.g., return x) */
    NODE_FUNC,      /* Function definition (e.g., int f() {}) */
    NODE_PARAM,     /* Function parameter (e.g., int x) */
    NODE_PARAM_LIST, /* Parameter list (e.g., int x, float y) */
    NODE_FUNC_CALL, /* Function call (e.g., f(x, y)) */
    NODE_ARG_LIST, /* Argument list (e.g., x, y) */
    NODE_STMT_LIST,  /* List of statements (program structure) */
    NODE_FUNC_LIST,  /* List of functions (program structure) */
    NODE_WHILE,     /* While loop (e.g., while (cond) { ... }) */
    NODE_FOR,       /* For loop  (e.g., for (i=0; i<n; i=i+1) { ... }) */
    NODE_IF,        /* If statement (e.g., if (cond) { ... } else { ... }) */
    NODE_SWITCH,    /* Switch statement */
    NODE_CASE,      /* Case/default clause */
    NODE_BREAK,     /* Break statement */
    NODE_STRUCT_DEF,    /* Struct type definition */
    NODE_FIELD_DECL,    /* Struct field declaration */
    NODE_MEMBER_ACCESS, /* Struct member read (e.g., p.x) */
    NODE_MEMBER_ASSIGN, /* Struct member write (e.g., p.x = 3) */
    NODE_ADDR_OF        /* Address-of expression (e.g., &p) */
} NodeType;

/* Comparison operators for NODE_BINOP */
#define OP_EQ 1000  /* == */
#define OP_NE 1001  /* != */
#define OP_LT 1002  /* < */
#define OP_GT 1003  /* > */
#define OP_LE 1004  /* <= */
#define OP_GE 1005  /* >= */
#define OP_AND 1006 /* && */
#define OP_OR  1007 /* || */
#define OP_NOT 1008 /* ! (unary, uses left operand) */

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
        
        /* String literal value (NODE_STR) */
        char* str;
        
        /* Variable or declaration name (NODE_VAR, NODE_DECL) */
        struct {
            char* name;              /* Variable name */
            VarType type;            /* Variable type (for declarations) */
            char* struct_name;       /* Struct type name when type == TYPE_STRUCT */
        } var;

        /* Struct field declaration (NODE_FIELD_DECL) */
        struct {
            char* name;              /* Field name */
            struct ASTNode* next;    /* Next field declaration */
        } field_decl;
        
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
            int op;                     /* Operator ('+', '-', '*', '/', or OP_EQ, OP_NE, etc.) */
            struct ASTNode* left;       /* Left operand */
            struct ASTNode* right;      /* Right operand */
        } binop;
        
        /* Assignment structure (NODE_ASSIGN) */
        struct {
            char* var;                  /* Variable being assigned to */
            struct ASTNode* value;      /* Expression being assigned */
        } assign;

        /* Struct member access (NODE_MEMBER_ACCESS) */
        struct {
            struct ASTNode* base;       /* Base expression (typically a variable) */
            char* field;                /* Field name */
        } member_access;

        /* Struct member assignment (NODE_MEMBER_ASSIGN) */
        struct {
            struct ASTNode* base;       /* Base expression */
            char* field;                /* Field name */
            struct ASTNode* value;      /* Assigned expression */
        } member_assign;
        
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
            char* struct_name;           /* Struct type name for struct params */
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
        
        /* While loop structure (NODE_WHILE) */
        struct {
            struct ASTNode* condition;  /* Loop condition */
            struct ASTNode* body;       /* Loop body (statement list) */
        } while_loop;

        /* For loop structure (NODE_FOR) */
        struct {
            struct ASTNode* init;       /* Initialization statement (NULL if omitted) */
            struct ASTNode* condition;  /* Loop condition (NULL = always true) */
            struct ASTNode* update;     /* Per-iteration update (NULL if omitted) */
            struct ASTNode* body;       /* Loop body; should not be NULL */
        } for_loop;

        /* If statement (NODE_IF) */
        struct {
            struct ASTNode* condition;  /* boolean test expression */
            struct ASTNode* then_stmt;  /* executed when condition is true */
            struct ASTNode* else_stmt;  /* executed when false (NULL = no else) */
        } if_stmt;

        /* Switch statement (NODE_SWITCH) */
        struct {
            struct ASTNode* expr;       /* controlling expression */
            struct ASTNode* cases;      /* head of linked case list */
        } switch_stmt;

        /* Case/default clause (NODE_CASE) */
        struct {
            int value;                  /* case constant (ignored for default) */
            int is_default;             /* 1 = default clause */
            struct ASTNode* body;       /* clause body (can be NULL for fall-through) */
            struct ASTNode* next;       /* next case/default clause */
        } case_stmt;
    } data;
} ASTNode;

/* AST CONSTRUCTION FUNCTIONS
 * These functions are called by the parser to build the tree
 */
ASTNode* createNum(int value);                                   /* Create number node */
ASTNode* createFlt(double value);                                   /* Create float node */
ASTNode* createStr(char* str);                                   /* Create string node */
ASTNode* createVar(char* name);                                  /* Create variable node */
ASTNode* createBinOp(int op, ASTNode* left, ASTNode* right);   /* Create binary op node */
ASTNode* createDecl(char* name, VarType type);                              /* Create declaration node */
ASTNode* createStructDecl(char* name, const char* struct_name);             /* Create struct declaration node */
ASTNode* createArrayDecl(char* name, VarType type, int size);   /* Create array declaration node */
ASTNode* createArrayAccess(char* name, ASTNode* index);         /* Create array access node */
ASTNode* createArrayAssign(char* name, ASTNode* index, ASTNode* value); /* Create array assignment node */
ASTNode* createAssign(char* var, ASTNode* value);               /* Create assignment node */
ASTNode* createPrint(ASTNode* expr);                            /* Create print node */
ASTNode* createWrite(ASTNode* expr);                            /* Create write node */
ASTNode* createReturn(ASTNode* expr);                           /* Create return node */
ASTNode* createFunc(char* name, VarType return_type, ASTNode* params, ASTNode* body);  /* Create function node */
ASTNode* createParam(char* name, VarType type);                  /* Create parameter node */
ASTNode* createStructParam(char* name, const char* struct_name, int is_pointer); /* Create struct param node */
ASTNode* createArrayParam(char* name, VarType type);             /* Create array parameter node */
ASTNode* createParamList(ASTNode* param, ASTNode* next);         /* Create parameter list */
ASTNode* createFuncCall(char* name, ASTNode* args);              /* Create function call node */
ASTNode* createArgList(ASTNode* arg, ASTNode* next);             /* Create argument list */
ASTNode* createStmtList(ASTNode* stmt1, ASTNode* stmt2);        /* Create statement list */
ASTNode* createFuncList(ASTNode* func1, ASTNode* func2);         /* Create function list */
ASTNode* createWhile(ASTNode* condition, ASTNode* body);         /* Create while loop node */
ASTNode* createFor(ASTNode* init, ASTNode* condition, ASTNode* update, ASTNode* body); /* Create for loop node */
ASTNode* createIf(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt);          /* Create if-stmt node */
ASTNode* createSwitch(ASTNode* expr, ASTNode* cases);                                    /* Create switch node */
ASTNode* createCase(int value, int is_default, ASTNode* body);                           /* Create case/default node */
ASTNode* appendCase(ASTNode* list, ASTNode* clause);                                     /* Append case node to list tail */
ASTNode* createBreak(void);                                                               /* Create break node */
ASTNode* createStructDef(char* name, ASTNode* fields);                                   /* Create struct definition node */
ASTNode* createFieldDecl(char* name);                                                    /* Create field declaration node */
ASTNode* appendField(ASTNode* list, ASTNode* field);                                     /* Append field to list tail */
ASTNode* createMemberAccess(ASTNode* base, char* field);                                 /* Create member access node */
ASTNode* createMemberAssign(ASTNode* base, char* field, ASTNode* value);                 /* Create member assignment node */
ASTNode* createAddrOf(ASTNode* expr);                                                     /* Create address-of node */

/* AST DISPLAY FUNCTION */
void printAST(ASTNode* node, int level);                        /* Pretty-print the AST */

/* MEMORY POOL FUNCTIONS */
void* ast_alloc(size_t size);                                   /* Allocate from pool */
void free_all_pools();                                          /* Free all memory pools */

#endif
