#ifndef SYMTAB_H
#define SYMTAB_H
/* SYMBOL TABLE
 * Tracks all declared variables during compilation
 * Maps variable names to their memory locations (stack offsets)
 * Used for semantic checking and code generation
 */

/* Forward declaration for ASTNode */
typedef struct ASTNode ASTNode;

/* Global error counter for semantic errors detected during compilation */
extern int semantic_error_count;

#define MAX_VARS 100  /* Maximum number of variables supported */
#define MAX_FUNCS 50  /* Maximum number of functions supported */
#define MAX_STRUCTS 32
#define MAX_STRUCT_FIELDS 32
#define HASH_SIZE 211 /* Prime number for better hash distribution */

typedef struct FieldInfo {
    char* name;
    int offset;
} FieldInfo;

typedef struct StructType {
    char* name;
    FieldInfo fields[MAX_STRUCT_FIELDS];
    int numFields;
    int totalSize;
} StructType;

/* VARIABLE TYPES */
typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_FLOAT,
    TYPE_VOID,
    TYPE_STRUCT,
    TYPE_STRUCT_PTR
} VarType;
/* SYMBOL ENTRY - Information about each variable */
typedef struct Symbol {
    char* name;     /* Variable identifier */
    int offset;     /* Stack offset in bytes (for MIPS stack frame) */
    VarType type;   /* Variable type (int or float) */
    StructType* structType; /* Struct metadata when type == TYPE_STRUCT */
    int isArray;   /* Flag indicating if variable is an array */
    int arraySize; /* Size of the array if isArray is true */
    struct Symbol* next; /* Next symbol in hash chain (for collision resolution) */
} Symbol;

/* SYMBOL TABLE STRUCTURE */
typedef struct {
    Symbol vars[MAX_VARS];  /* Array of all variables (for backward compatibility) */
    Symbol* hash_table[HASH_SIZE]; /* Hash table for O(1) lookup */
    int count;              /* Number of variables declared */
    int nextOffset;         /* Next available stack offset */
} SymbolTable;

/* FUNCTION SYMBOL ENTRY - Information about each function */
typedef struct {
    char* name;             /* Function identifier */
    VarType return_type;    /* Return type */
    int param_count;        /* Number of parameters */
    SymbolTable* local_symtab; /* Pointer to local symbol table */
    ASTNode* ast_node;      /* Pointer to function AST */
} FunctionSymbol;
/* GLOBAL SYMBOL TABLE - Contains functions and global variables */
typedef struct {
    FunctionSymbol funcs[MAX_FUNCS];  /* Array of all functions */
    SymbolTable* current_local;      /* Current local symbol table */
    int func_count;                  /* Number of functions declared */
    int current_func_index;           /* Index of current function being processed */
} GlobalSymbolTable;
/* SYMBOL TABLE OPERATIONS */
void initSymTab();               /* Initialize empty symbol table */
void initGlobalSymTab();         /* Initialize global symbol table */
int addVar(char* name, VarType type);          /* Add new variable with type, returns offset or -1 if duplicate */
int addVarToLocal(char* name, VarType type);   /* Add variable to current local scope */
int addFunction(char* name, VarType return_type, ASTNode* ast_node); /* Add function to global table */
int getVarOffset(char* name);    /* Get stack offset for variable, -1 if not found */
VarType getVarType(char* name);   /* Get type for variable, returns TYPE_INT if not found */
VarType getVarTypeByName(const char* name);
int isVarDeclared(char* name);   /* Check if variable exists (1=yes, 0=no) */
void prepareFunctionScope(char* name, VarType return_type); /* Prepare function scope for parsing */
void enterFunction(char* name);  /* Enter function scope */
void exitFunction();             /* Exit function scope */
int isFunctionDeclared(char* name); /* Check if function exists */
VarType getFunctionReturnType(char* name); /* Get function return type */
void printSymTab();              /* Print current symbol table contents for tracing */
void printGlobalSymTab();        /* Print global symbol table */
int addArrayVar(char* name, VarType type, int size); /* Add array variable */
int isArrayVar(char* name); /* Check if variable is an array */
int getArraySize(char* name); /* Get size of array variable */
int validateFunctionCall(char* func_name, int arg_count); /* Validate function call argument count */
void addParamsToScope(ASTNode* params); /* Add function parameters to current scope */

/* Struct type operations */
int registerStruct(StructType* st);
StructType* lookupStruct(const char* name);
int getStructFieldOffset(const StructType* st, const char* field_name);
int addStructVar(char* name, const char* struct_name);
int addStructPtrVar(char* name, const char* struct_name);
StructType* getVarStructType(char* name);
int isStructPointerVar(char* name);

/* Tracks whether the parsed source used struct features (Session 1 support). */
extern int struct_feature_used;
#endif
