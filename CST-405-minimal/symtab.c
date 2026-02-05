#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "ast.h"

/* Global symbol table instance */
GlobalSymbolTable globalSymTab;
SymbolTable* currentSymTab = NULL;  // current active scope

extern int yyline;

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
    printf("SYMBOL TABLE: Initialized\n");
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

    Symbol* entry = &currentSymTab->vars[currentSymTab->count];
    entry->name = strdup(name);
    entry->type = type;
    entry->is_array = 0;
    entry->array_size = 0;
    entry->offset = currentSymTab->nextOffset;
    currentSymTab->nextOffset += 4; // assume 4 bytes per variable
    currentSymTab->count++;

    printf("SYMBOL TABLE: Added '%s' (%s) at offset %d\n", 
           name,
           type == TYPE_FLOAT ? "float" : type == TYPE_VOID ? "void" : "int",
           entry->offset);

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
    entry->is_array = 1;
    entry->array_size = size;
    entry->offset = currentSymTab->nextOffset;
    currentSymTab->nextOffset += 4 * size; // allocate space for array
    currentSymTab->count++;

    printf("SYMBOL TABLE: Added array '%s[%d]' (%s) at offset %d\n", 
           name, size,
           type == TYPE_FLOAT ? "float" : type == TYPE_VOID ? "void" : "int",
           entry->offset);

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

/* Look up variable offset (local then global params if needed) */
int getVarOffset(char* name) {
    if (!currentSymTab) return -1;

    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) return currentSymTab->vars[i].offset;
    }

    return -1; // not found
}

/* Look up variable type (local then global) */
VarType getVarType(char* name) {
    if (!currentSymTab) return TYPE_INT;

    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) return currentSymTab->vars[i].type;
    }

    return TYPE_INT; // default
}

/* Check if variable declared (returns true/false only) */
int isVarDeclared(char* name) {
    return getVarOffset(name) != -1;
}

/* Check if variable is an array */
int isArray(char* name) {
    if (!currentSymTab) return 0;

    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) 
            return currentSymTab->vars[i].is_array;
    }
    return 0;
}

/* Get size of array */
int getArraySize(char* name) {
    if (!currentSymTab) return 0;

    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) 
            return currentSymTab->vars[i].array_size;
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
    currentSymTab->nextOffset += 4 * size; // allocate space for the entire array
    currentSymTab->count++;

    printf("SYMBOL TABLE: Added array '%s' (%s[%d]) at offset %d\n", 
           name,
           type == TYPE_FLOAT ? "float" : type == TYPE_VOID ? "void" : "int",
           size,
           entry->offset);

    return entry->offset;
}

int isArrayVar(char* name) {
    if (!currentSymTab) return 0;

    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) {
            return currentSymTab->vars[i].isArray;
        }
    }
    return 0;
}

int getArraySize(char* name) {
    if (!currentSymTab) return -1;

    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0 && currentSymTab->vars[i].isArray) {
            return currentSymTab->vars[i].arraySize;
        }
    }
    return -1;
}