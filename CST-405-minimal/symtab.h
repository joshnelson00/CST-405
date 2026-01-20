#ifndef SYMTAB_H
#define SYMTAB_H
/* SYMBOL TABLE
 * Tracks all declared variables during compilation
 * Maps variable names to their memory locations (stack offsets)
 * Used for semantic checking and code generation
 */
#define MAX_VARS 100  /* Maximum number of variables supported */
/* VARIABLE TYPES */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT
} VarType;
/* SYMBOL ENTRY - Information about each variable */
typedef struct {
    char* name;     /* Variable identifier */
    int offset;     /* Stack offset in bytes (for MIPS stack frame) */
    VarType type;   /* Variable type (int or float) */
} Symbol;
/* SYMBOL TABLE STRUCTURE */
typedef struct {
    Symbol vars[MAX_VARS];  /* Array of all variables */
    VarType type;           /* Variable type */
    int count;              /* Number of variables declared */
    int nextOffset;         /* Next available stack offset */
} SymbolTable;
/* SYMBOL TABLE OPERATIONS */
void initSymTab();               /* Initialize empty symbol table */
int addVar(char* name, VarType type);          /* Add new variable with type, returns offset or -1 if duplicate */
int getVarOffset(char* name);    /* Get stack offset for variable, -1 if not found */
VarType getVarType(char* name);   /* Get type for variable, returns TYPE_INT if not found */
VarType getVarTypeByName(const char* name);
int isVarDeclared(char* name);   /* Check if variable exists (1=yes, 0=no) */
void printSymTab();              /* Print current symbol table contents for tracing */
#endif
