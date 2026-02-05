#ifndef SYMTAB_H
#define SYMTAB_H
#ifndef HASH_SIZE
#define HASH_SIZE 211  // Prime number for better distribution
#endif
#ifndef MAX_VARS
#define MAX_VARS 1000  // Increased capacity
#endif
/* SYMBOL TABLE
 * Tracks all declared variables during compilation
 * Maps variable names to their memory locations (stack offsets)
 * Used for semantic checking and code generation
 */
/* VARIABLE TYPES */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT
} VarType;
/* SYMBOL ENTRY - Information about each variable */
typedef struct SymbolNode {
    char* name;
    int offset;
    VarType type;
    struct SymbolNode* next;  // For chaining
} SymbolNode;

typedef struct {
    SymbolNode* buckets[HASH_SIZE];
    int count;
    int nextOffset;
    // Performance counters
    int lookups;
    int collisions;
    int verbose;
} SymbolTable;

/* SYMBOL TABLE OPERATIONS */
void initSymTab();               /* Initialize empty symbol table */
int addVar(char* name, VarType type);          /* Add new variable with type, returns offset or -1 if duplicate */
int getVarOffset(char* name);    /* Get stack offset for variable, -1 if not found */
VarType getVarType(char* name);   /* Get type for variable, returns TYPE_INT if not found */
VarType getVarTypeByName(const char* name);
int isVarDeclared(char* name);   /* Check if variable exists (1=yes, 0=no) */
void printSymTab();              /* Print current symbol table contents for tracing */
void setSymTabVerbose(int enabled);
#endif
