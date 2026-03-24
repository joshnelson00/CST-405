#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "ast.h"

/* Global symbol table instance */
GlobalSymbolTable globalSymTab;
SymbolTable* currentSymTab = NULL;  // current active scope
static StructType structTypes[MAX_STRUCTS];
static int structTypeCount = 0;

/* Global error counter for semantic errors */
int semantic_error_count = 0;
int struct_feature_used = 0;

extern int yyline;

/* DJB2 Hash Function - O(1) lookup optimization */
unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    
    return hash % HASH_SIZE;
}

/* Initialize global symbol table */
void initGlobalSymTab() {
    globalSymTab.func_count = 0;
    globalSymTab.current_local = NULL;
    globalSymTab.current_func_index = -1;
    structTypeCount = 0;
    printf("GLOBAL SYMBOL TABLE: Initialized\n");
}

StructType* lookupStruct(const char* name) {
    if (!name) return NULL;
    for (int i = 0; i < structTypeCount; i++) {
        if (strcmp(structTypes[i].name, name) == 0) {
            return &structTypes[i];
        }
    }
    return NULL;
}

int getStructFieldOffset(const StructType* st, const char* field_name) {
    if (!st || !field_name) return -1;
    for (int i = 0; i < st->numFields; i++) {
        if (strcmp(st->fields[i].name, field_name) == 0) {
            return st->fields[i].offset;
        }
    }
    return -1;
}

int registerStruct(StructType* st) {
    if (!st || !st->name) return -1;
    if (lookupStruct(st->name)) {
        return -1;
    }
    if (structTypeCount >= MAX_STRUCTS) {
        fprintf(stderr, "❌ Error: Struct table full (max %d structs)\n", MAX_STRUCTS);
        semantic_error_count++;
        return -1;
    }

    StructType* dst = &structTypes[structTypeCount++];
    dst->name = strdup(st->name);
    dst->numFields = st->numFields;
    dst->totalSize = st->numFields * 4;

    for (int i = 0; i < st->numFields; i++) {
        dst->fields[i].name = strdup(st->fields[i].name);
        dst->fields[i].offset = i * 4;
    }

    struct_feature_used = 1;
    printf("STRUCT TABLE: Registered struct '%s' (%d fields, %d bytes)\n",
           dst->name, dst->numFields, dst->totalSize);
    return 0;
}

/* Initialize a new local symbol table */
void initSymTab() {
    if (!currentSymTab) currentSymTab = malloc(sizeof(SymbolTable));
    currentSymTab->count = 0;
    currentSymTab->nextOffset = 0;
    // Initialize hash table to NULL
    for (int i = 0; i < HASH_SIZE; i++) {
        currentSymTab->hash_table[i] = NULL;
    }
    printf("SYMBOL TABLE: Initialized with hash table (O(1) lookup)\n");
    printSymTab();
}

/* Add variable to current symbol table */
int addVar(char* name, VarType type) {
    if (!currentSymTab) {
        fprintf(stderr, "❌ Error: No active symbol table\n");
        return -1;
    }

    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Variable '%s' is already declared in this scope\n", name);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Remove the duplicate declaration if it was unintentional\n");
        fprintf(stderr, "   • Use a different name (e.g., '%s2' or '%s_alt')\n", name, name);
        fprintf(stderr, "   • If you intended to reassign, just use: %s = <value>;\n\n", name);
        semantic_error_count++;
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        semantic_error_count++;
        return -1;
    }

    // Allocate new symbol
    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
    entry->structType = NULL;
    entry->isArray = 0;
    entry->arraySize = 0;
    entry->offset = currentSymTab->nextOffset;
    entry->next = NULL;
    currentSymTab->nextOffset += 4; // assume 4 bytes per variable
    currentSymTab->count++;

    // Add to hash table for O(1) lookup
    unsigned int h = hash(name);
    entry->next = currentSymTab->hash_table[h];
    currentSymTab->hash_table[h] = entry;

    printf("SYMBOL TABLE: Added '%s' (%s) at offset %d [hash=%u]\n", 
           name,
           type == TYPE_FLOAT ? "float" : type == TYPE_VOID ? "void" : "int",
           entry->offset, h);

    return entry->offset;
}

/* Add array to current symbol table */
int addArray(char* name, VarType type, int size) {
    if (!currentSymTab) {
        fprintf(stderr, "❌ Error: No active symbol table\n");
        semantic_error_count++;
        return -1;
    }

    if (size <= 0) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have %s size (%d)\n", name,
                size < 0 ? "negative" : "zero", size);
        fprintf(stderr, "💡 Suggestion: Array size must be a positive integer\n\n");
        semantic_error_count++;
        return -1;
    }

    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' is already declared in this scope\n", name);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Remove the duplicate declaration\n");
        fprintf(stderr, "   • Use a different name (e.g., '%s2')\n\n", name);
        semantic_error_count++;
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        semantic_error_count++;
        return -1;
    }

    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
    entry->structType = NULL;
    entry->isArray = 1;
    entry->arraySize = size;
    entry->offset = currentSymTab->nextOffset;
    entry->next = NULL;
    currentSymTab->nextOffset += 4 * size; // allocate space for array
    currentSymTab->count++;

    // Add to hash table for O(1) lookup
    unsigned int h = hash(name);
    entry->next = currentSymTab->hash_table[h];
    currentSymTab->hash_table[h] = entry;

    printf("SYMBOL TABLE: Added array '%s[%d]' (%s) at offset %d [hash=%u]\n", 
           name, size,
           type == TYPE_FLOAT ? "float" : type == TYPE_VOID ? "void" : "int",
           entry->offset, h);

    return entry->offset;
}

/* Add parameter to current symbol table (function params) */
int addParam(char* name, VarType type) {
    return addVar(name, type);  // same as variable but can track param_count later
}

/* Recursively count parameters in a nested param list AST */
static int countParams(ASTNode* node) {
    if (!node) return 0;
    if (node->type == NODE_PARAM) return 1;
    if (node->type == NODE_PARAM_LIST) {
        return countParams(node->data.param_list.param) + 
               countParams(node->data.param_list.next);
    }
    return 0;
}

/* Add function to global table */
int addFunction(char* name, VarType return_type, ASTNode* ast_node) {
    // Check if function was already prepared (via prepareFunctionScope)
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0) {
            // Function exists, just update AST and count parameters
            globalSymTab.funcs[i].ast_node = ast_node;
            
            // Count parameters from AST using recursive traversal
            int param_count = 0;
            if (ast_node && ast_node->data.func.params) {
                param_count = countParams(ast_node->data.func.params);
            }
            globalSymTab.funcs[i].param_count = param_count;
            
            printf("GLOBAL SYMBOL TABLE: Updated function '%s' with AST (%d params)\n", 
                   name, param_count);
            return i;
        }
    }
    
    // Function doesn't exist yet, create it (old behavior for compatibility)
    if (globalSymTab.func_count >= MAX_FUNCS) {
        fprintf(stderr, "❌ Error: Function table full (max %d functions)\n", MAX_FUNCS);
        return -1;
    }

    FunctionSymbol* func = &globalSymTab.funcs[globalSymTab.func_count];
    func->name = strdup(name);
    func->return_type = return_type;
    
    // Count parameters from AST using recursive traversal
    int param_count = 0;
    if (ast_node && ast_node->data.func.params) {
        param_count = countParams(ast_node->data.func.params);
    }
    func->param_count = param_count;
    
    func->local_symtab = malloc(sizeof(SymbolTable));
    func->local_symtab->count = 0;
    func->local_symtab->nextOffset = 0;
    // Initialize hash table for function's local scope
    for (int i = 0; i < HASH_SIZE; i++) {
        func->local_symtab->hash_table[i] = NULL;
    }
    func->ast_node = ast_node;

    globalSymTab.func_count++;
    printf("GLOBAL SYMBOL TABLE: Added function '%s' (%s) with %d params\n",
           name,
           return_type == TYPE_FLOAT ? "float" : return_type == TYPE_VOID ? "void" : "int",
           param_count);

    return globalSymTab.func_count - 1;
}

/* Prepare function scope - creates function entry and activates its scope */
void prepareFunctionScope(char* name, VarType return_type) {
    if (isFunctionDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Function '%s' is already defined\n", name);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Remove the duplicate function definition\n");
        fprintf(stderr, "   • Rename the function (e.g., '%s2' or '%s_v2')\n", name, name);
        fprintf(stderr, "   • If you intended to overload, this language does not support function overloading\n\n");
        semantic_error_count++;
        /* Enter the existing function's scope for error recovery */
        /* This allows the parser to continue processing the duplicate body */
        enterFunction(name);
        return;
    }

    if (globalSymTab.func_count >= MAX_FUNCS) {
        fprintf(stderr, "❌ Error: Function table full (max %d functions)\n", MAX_FUNCS);
        return;
    }

    FunctionSymbol* func = &globalSymTab.funcs[globalSymTab.func_count];
    func->name = strdup(name);
    func->return_type = return_type;
    func->param_count = 0;
    func->local_symtab = malloc(sizeof(SymbolTable));
    func->local_symtab->count = 0;
    func->local_symtab->nextOffset = 0;
    // Initialize hash table for function's local scope
    for (int i = 0; i < HASH_SIZE; i++) {
        func->local_symtab->hash_table[i] = NULL;
    }
    func->ast_node = NULL;  // Will be set later

    globalSymTab.current_func_index = globalSymTab.func_count;
    globalSymTab.current_local = func->local_symtab;
    currentSymTab = func->local_symtab;
    
    globalSymTab.func_count++;
    printf("PREPARE FUNCTION SCOPE: '%s' (%s) - scope activated\n",
           name,
           return_type == TYPE_FLOAT ? "float" : return_type == TYPE_VOID ? "void" : "int");
}

/* Enter function scope */
void enterFunction(char* name) {
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0) {
            globalSymTab.current_func_index = i;
            globalSymTab.current_local = globalSymTab.funcs[i].local_symtab;
            currentSymTab = globalSymTab.current_local;
            printf("ENTER FUNCTION: '%s' (scope activated)\n", name);
            return;
        }
    }
    fprintf(stderr, "❌ Error: Function '%s' not found\n", name);
}

/* Exit function scope */
void exitFunction() {
    if (globalSymTab.current_func_index >= 0) {
        printf("EXIT FUNCTION: '%s' (scope deactivated)\n",
               globalSymTab.funcs[globalSymTab.current_func_index].name);
    }
    globalSymTab.current_func_index = -1;
    globalSymTab.current_local = NULL;
    currentSymTab = NULL;
}

/* Look up variable offset using hash table O(1) lookup */
int getVarOffset(char* name) {
    if (!currentSymTab) return -1;

    unsigned int h = hash(name);
    Symbol* node = currentSymTab->hash_table[h];
    
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->offset;
        }
        node = node->next;
    }

    return -1; // not found
}

/* Look up variable type using hash table O(1) lookup */
VarType getVarType(char* name) {
    if (!currentSymTab) return TYPE_INT;

    unsigned int h = hash(name);
    Symbol* node = currentSymTab->hash_table[h];
    
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->type;
        }
        node = node->next;
    }

    return TYPE_INT; // default
}

/* Check if variable declared (returns true/false only) */
int isVarDeclared(char* name) {
    return getVarOffset(name) != -1;
}

/* Check if variable is an array using hash table O(1) lookup */
int isArray(char* name) {
    if (!currentSymTab) return 0;

    unsigned int h = hash(name);
    Symbol* node = currentSymTab->hash_table[h];
    
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->isArray;
        }
        node = node->next;
    }
    return 0;
}

/* Get size of array using hash table O(1) lookup */
int getArraySize(char* name) {
    if (!currentSymTab) return 0;

    unsigned int h = hash(name);
    Symbol* node = currentSymTab->hash_table[h];
    
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->arraySize;
        }
        node = node->next;
    }
    return 0;
}

/* Check if function declared */
int isFunctionDeclared(char* name) {
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0) return 1;
    }
    return 0;
}

/* Validate function call argument count */
int validateFunctionCall(char* func_name, int arg_count) {
    /* Skip validation for built-in functions */
    if (strcmp(func_name, "print") == 0) return 1;

    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, func_name) == 0) {
            if (globalSymTab.funcs[i].param_count != arg_count) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Function '%s' expects %d argument%s but got %d\n",
                       func_name, 
                       globalSymTab.funcs[i].param_count,
                       globalSymTab.funcs[i].param_count == 1 ? "" : "s",
                       arg_count);
                fprintf(stderr, "💡 Suggestions:\n");
                if (arg_count < globalSymTab.funcs[i].param_count) {
                    fprintf(stderr, "   • Add %d more argument%s to the function call\n",
                            globalSymTab.funcs[i].param_count - arg_count,
                            (globalSymTab.funcs[i].param_count - arg_count) == 1 ? "" : "s");
                } else {
                    fprintf(stderr, "   • Remove %d argument%s from the function call\n",
                            arg_count - globalSymTab.funcs[i].param_count,
                            (arg_count - globalSymTab.funcs[i].param_count) == 1 ? "" : "s");
                }
                fprintf(stderr, "   • Check the function definition for the correct signature\n\n");
                semantic_error_count++;
                return 0;  // Validation failed
            }
            return 1;  // Validation passed
        }
    }
    /* Function not found - check for similar names */
    fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
    fprintf(stderr, "   Function '%s' is not declared\n", func_name);
    fprintf(stderr, "💡 Suggestions:\n");
    fprintf(stderr, "   • Check for typos in the function name\n");
    fprintf(stderr, "   • Make sure the function is defined before it is called\n");
    if (globalSymTab.func_count > 0) {
        fprintf(stderr, "   • Available functions: ");
        for (int i = 0; i < globalSymTab.func_count; i++) {
            fprintf(stderr, "%s%s", globalSymTab.funcs[i].name,
                    i < globalSymTab.func_count - 1 ? ", " : "");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
    semantic_error_count++;
    return 0;
}

/* Get function return type */
VarType getFunctionReturnType(char* name) {
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0)
            return globalSymTab.funcs[i].return_type;
    }
    return TYPE_VOID;
}

/* Debug print current symbol table */
void printSymTab() {
    printf("\n=== SYMBOL TABLE ===\n");
    if (!currentSymTab) {
        printf("(no active table)\n");
        return;
    }
    printf("Count: %d, NextOffset: %d\n", currentSymTab->count, currentSymTab->nextOffset);
    for (int i = 0; i < currentSymTab->count; i++) {
        if (currentSymTab->vars[i].type == TYPE_STRUCT && currentSymTab->vars[i].structType) {
            printf("  %s (struct %s) -> offset %d\n",
                   currentSymTab->vars[i].name,
                   currentSymTab->vars[i].structType->name,
                   currentSymTab->vars[i].offset);
        } else {
            printf("  %s (%s) -> offset %d\n",
                   currentSymTab->vars[i].name,
                   currentSymTab->vars[i].type == TYPE_FLOAT ? "float" :
                   currentSymTab->vars[i].type == TYPE_VOID ? "void" : "int",
                   currentSymTab->vars[i].offset);
        }
    }
    printf("==================\n");
}

/* Debug print global symbol table */
void printGlobalSymTab() {
    printf("\n=== GLOBAL SYMBOL TABLE ===\n");
    printf("Functions: %d\n", globalSymTab.func_count);
    for (int i = 0; i < globalSymTab.func_count; i++) {
        FunctionSymbol* f = &globalSymTab.funcs[i];
        printf("  %s (%s) - %d params, %d locals\n",
               f->name,
               f->return_type == TYPE_FLOAT ? "float" : f->return_type == TYPE_VOID ? "void" : "int",
               f->param_count,
               f->local_symtab->count);
    }
    printf("==========================\n");
}

int addArrayVar(char* name, VarType type, int size) {
    if (!currentSymTab) {
        fprintf(stderr, "❌ Error: No active symbol table\n");
        semantic_error_count++;
        return -1;
    }

    if (size < 0) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have negative size (%d)\n", name, size);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Array size must be a positive integer\n");
        fprintf(stderr, "   • Example: %s %s[%d];\n\n",
                type == TYPE_FLOAT ? "float" : "int", name, -size);
        semantic_error_count++;
        return -1;
    }

    if (size == 0) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have zero size\n", name);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Array size must be at least 1\n");
        fprintf(stderr, "   • Example: %s %s[10];\n\n",
                type == TYPE_FLOAT ? "float" : "int", name);
        semantic_error_count++;
        return -1;
    }

    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' is already declared in this scope\n", name);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Remove the duplicate declaration if it was unintentional\n");
        fprintf(stderr, "   • Use a different name (e.g., '%s2' or '%s_alt')\n\n", name, name);
        semantic_error_count++;
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        semantic_error_count++;
        return -1;
    }

    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
    entry->structType = NULL;
    entry->isArray = 1;
    entry->arraySize = size;
    entry->offset = currentSymTab->nextOffset;
    entry->next = NULL;
    currentSymTab->nextOffset += 4 * size; // allocate space for the entire array
    currentSymTab->count++;

    // Add to hash table for O(1) lookup
    unsigned int h = hash(name);
    entry->next = currentSymTab->hash_table[h];
    currentSymTab->hash_table[h] = entry;

    printf("SYMBOL TABLE: Added array '%s' (%s[%d]) at offset %d [hash=%u]\n", 
           name,
           type == TYPE_FLOAT ? "float" : type == TYPE_VOID ? "void" : "int",
           size,
           entry->offset, h);

    return entry->offset;
}

int addStructVar(char* name, const char* struct_name) {
    if (!currentSymTab) {
        fprintf(stderr, "❌ Error: No active symbol table\n");
        return -1;
    }

    StructType* st = lookupStruct(struct_name);
    if (!st) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Unknown struct type '%s'\n", struct_name);
        fprintf(stderr, "💡 Suggestion: define it first with: struct %s { ... };\n\n", struct_name);
        semantic_error_count++;
        return -1;
    }

    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Variable '%s' is already declared in this scope\n", name);
        fprintf(stderr, "💡 Suggestion: use a different variable name\n\n");
        semantic_error_count++;
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        semantic_error_count++;
        return -1;
    }

    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = TYPE_STRUCT;
    entry->structType = st;
    entry->isArray = 0;
    entry->arraySize = 0;
    entry->offset = currentSymTab->nextOffset;
    entry->next = NULL;
    currentSymTab->nextOffset += st->totalSize;
    currentSymTab->count++;

    unsigned int h = hash(name);
    entry->next = currentSymTab->hash_table[h];
    currentSymTab->hash_table[h] = entry;

    struct_feature_used = 1;
    printf("SYMBOL TABLE: Added struct var '%s' (struct %s) at offset %d [size=%d]\n",
           name, struct_name, entry->offset, st->totalSize);
    return entry->offset;
}

StructType* getVarStructType(char* name) {
    if (!currentSymTab || !name) return NULL;

    unsigned int h = hash(name);
    Symbol* node = currentSymTab->hash_table[h];
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->structType;
        }
        node = node->next;
    }
    return NULL;
}

int isArrayVar(char* name) {
    if (!currentSymTab) return 0;

    unsigned int h = hash(name);
    Symbol* node = currentSymTab->hash_table[h];
    
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->isArray;
        }
        node = node->next;
    }
    return 0;
}

/* Add function parameters to the current scope's symbol table
 * This is called after prepareFunctionScope() to register params
 * so that undeclared variable checks work correctly within function bodies.
 */
void addParamsToScope(ASTNode* node) {
    if (!node) return;
    if (node->type == NODE_PARAM) {
        /* Pre-register param count so recursive calls inside the body can
         * pass the argument-count check before addFunction() runs. */
        if (globalSymTab.current_func_index >= 0)
            globalSymTab.funcs[globalSymTab.current_func_index].param_count++;
        if (node->data.param.is_array) {
            /* Array parameters have unknown size at compile time.
             * Use a large placeholder size to avoid false bounds warnings. */
            addArrayVar(node->data.param.name, node->data.param.type, 9999);
        } else {
            addVar(node->data.param.name, node->data.param.type);
        }
    } else if (node->type == NODE_PARAM_LIST) {
        addParamsToScope(node->data.param_list.param);
        addParamsToScope(node->data.param_list.next);
    }
}