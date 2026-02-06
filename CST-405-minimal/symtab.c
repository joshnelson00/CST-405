#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "ast.h"

/* Global symbol table instance */
GlobalSymbolTable globalSymTab;
SymbolTable* currentSymTab = NULL;  // current active scope

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
    printf("GLOBAL SYMBOL TABLE: Initialized\n");
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
        fprintf(stderr, "   Variable '%s' already declared\n", name);
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        return -1;
    }

    // Allocate new symbol
    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
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
        return -1;
    }

    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Variable '%s' already declared\n", name);
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        return -1;
    }

    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
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

/* Add function to global table */
int addFunction(char* name, VarType return_type, ASTNode* ast_node) {
    if (isFunctionDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Function '%s' already declared\n", name);
        return -1;
    }

    if (globalSymTab.func_count >= MAX_FUNCS) {
        fprintf(stderr, "❌ Error: Function table full (max %d functions)\n", MAX_FUNCS);
        return -1;
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
    func->ast_node = ast_node;

    globalSymTab.func_count++;
    printf("GLOBAL SYMBOL TABLE: Added function '%s' (%s)\n",
           name,
           return_type == TYPE_FLOAT ? "float" : return_type == TYPE_VOID ? "void" : "int");

    return globalSymTab.func_count - 1;
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
        printf("  %s (%s) -> offset %d\n",
               currentSymTab->vars[i].name,
               currentSymTab->vars[i].type == TYPE_FLOAT ? "float" :
               currentSymTab->vars[i].type == TYPE_VOID ? "void" : "int",
               currentSymTab->vars[i].offset);
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
        return -1;
    }

    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Variable '%s' already declared\n", name);
        return -1;
    }

    if (currentSymTab->count >= MAX_VARS) {
        fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
        return -1;
    }

    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
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