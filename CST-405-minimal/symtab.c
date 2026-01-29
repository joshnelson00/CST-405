/* SYMBOL TABLE IMPLEMENTATION
 * Manages variable declarations and lookups
 * Essential for semantic analysis (checking if variables are declared)
 * Provides memory layout information for code generation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "ast.h"

/* Global symbol table instance */
GlobalSymbolTable globalSymTab;
/* Current symbol table pointer (for backward compatibility) */
SymbolTable* currentSymTab = NULL;

/* External declarations for line tracking */
extern int yyline;

/* Initialize global symbol table */
void initGlobalSymTab() {
    globalSymTab.func_count = 0;
    globalSymTab.current_local = NULL;
    globalSymTab.current_func_index = -1;
    printf("GLOBAL SYMBOL TABLE: Initialized\n");
}

/* Initialize an empty symbol table */
void initSymTab() {
    if (!currentSymTab) {
        currentSymTab = malloc(sizeof(SymbolTable));
    }
    currentSymTab->count = 0;       /* No variables yet */
    currentSymTab->nextOffset = 0;  /* Start at stack offset 0 */
    printf("SYMBOL TABLE: Initialized\n");
    printSymTab();
}

/* Add a new variable to the current symbol table */
int addVar(char* name, VarType type) {
    if (!currentSymTab) {
        fprintf(stderr, "❌ Error: No active symbol table\n");
        return -1;
    }
    
    /* Check for duplicate declaration */
    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Variable '%s' already declared\n", name);
        fprintf(stderr, "💡 Suggestion: Use a different variable name or\n");
        fprintf(stderr, "   check for spelling mistakes\n\n");
        return -1;  /* Duplicate - error */
    }

    /* Add new variable */
    if (currentSymTab->count < MAX_VARS) {
        currentSymTab->vars[currentSymTab->count].name = strdup(name);
        currentSymTab->vars[currentSymTab->count].type = type;
        currentSymTab->vars[currentSymTab->count].offset = currentSymTab->nextOffset;
        
        printf("SYMBOL TABLE: Added variable '%s' (%s) at offset %d\n", 
               name, 
               type == TYPE_FLOAT ? "float" : 
               type == TYPE_VOID ? "void" : "int",
               currentSymTab->nextOffset);
        
        currentSymTab->nextOffset += 4;  /* Allocate 4 bytes for each variable */
        return currentSymTab->vars[currentSymTab->count++].offset;
    }
    
    fprintf(stderr, "❌ Error: Symbol table full (max %d variables)\n", MAX_VARS);
    return -1;
}

/* Add variable to current local scope (alias for addVar) */
int addVarToLocal(char* name, VarType type) {
    return addVar(name, type);
}

/* Add function to global symbol table */
int addFunction(char* name, VarType return_type, ASTNode* ast_node) {
    /* Check for duplicate function */
    if (isFunctionDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Function '%s' already declared\n", name);
        return -1;
    }

    if (globalSymTab.func_count < MAX_FUNCS) {
        FunctionSymbol* func = &globalSymTab.funcs[globalSymTab.func_count];
        func->name = strdup(name);
        func->return_type = return_type;
        func->param_count = 0;
        func->local_symtab = malloc(sizeof(SymbolTable));
        func->local_symtab->count = 0;
        func->local_symtab->nextOffset = 0;
        func->ast_node = ast_node;
        
        printf("GLOBAL SYMBOL TABLE: Added function '%s' (%s)\n", 
               name,
               return_type == TYPE_FLOAT ? "float" : 
               return_type == TYPE_VOID ? "void" : "int");
        
        globalSymTab.func_count++;
        return globalSymTab.func_count - 1;
    }
    
    fprintf(stderr, "❌ Error: Function table full (max %d functions)\n", MAX_FUNCS);
    return -1;
}

/* Look up a variable's stack offset */
int getVarOffset(char* name) {
    if (!currentSymTab) {
        return -1;
    }
    
    /* Linear search through current symbol table */
    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) {
            printf("SYMBOL TABLE: Found variable '%s' at offset %d\n", name, currentSymTab->vars[i].offset);
            return currentSymTab->vars[i].offset;  /* Found it */
        }
    }

    fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
    fprintf(stderr, "   Variable '%s' used but not declared\n", name);
    fprintf(stderr, "💡 Suggestion: Declare variable before use:\n");
    fprintf(stderr, "   %s %s;\n", 
           name[0] >= 'a' && name[0] <= 'z' ? "int" : 
           name[0] >= 'A' && name[0] <= 'Z' ? "float" : "void", name);
    fprintf(stderr, "   Or check for spelling mistakes\n\n");
    return -1;  /* Variable not found - semantic error */
}

/* Get a variable's type */
VarType getVarType(char* name) {
    if (!currentSymTab) {
        return TYPE_INT;
    }
    
    /* Linear search through current symbol table */
    for (int i = 0; i < currentSymTab->count; i++) {
        if (strcmp(currentSymTab->vars[i].name, name) == 0) {
            return currentSymTab->vars[i].type;  /* Found it */
        }
    }
    return TYPE_INT;  /* Default to int if not found */
}

/* Check if a variable has been declared */
int isVarDeclared(char* name) {
    if (!currentSymTab) {
        return 0;
    }
    return getVarOffset(name) != -1;  /* True if found, false otherwise */
}

/* Enter function scope */
void enterFunction(char* name) {
    int func_index = -1;
    
    /* Find function in global table */
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0) {
            func_index = i;
            break;
        }
    }
    
    if (func_index != -1) {
        globalSymTab.current_func_index = func_index;
        globalSymTab.current_local = globalSymTab.funcs[func_index].local_symtab;
        currentSymTab = globalSymTab.current_local;
        printf("ENTER FUNCTION: '%s' (scope activated)\n", name);
    } else {
        fprintf(stderr, "❌ Error: Function '%s' not found in global table\n", name);
    }
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

/* Check if a function has been declared */
int isFunctionDeclared(char* name) {
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0) {
            return 1;  /* Found it */
        }
    }
    return 0;  /* Not found */
}

/* Get function return type */
VarType getFunctionReturnType(char* name) {
    for (int i = 0; i < globalSymTab.func_count; i++) {
        if (strcmp(globalSymTab.funcs[i].name, name) == 0) {
            return globalSymTab.funcs[i].return_type;
        }
    }
    return TYPE_VOID;  /* Default to void if not found */
}
/* Print current symbol table contents for debugging/tracing */
void printSymTab() {
    if (!currentSymTab) {
        printf("\n=== SYMBOL TABLE STATE ===\n");
        printf("(no active symbol table)\n");
        printf("==========================\n\n");
        return;
    }
    
    printf("\n=== SYMBOL TABLE STATE ===\n");
    printf("Count: %d, Next Offset: %d\n", currentSymTab->count, currentSymTab->nextOffset);
    if (currentSymTab->count == 0) {
        printf("(empty)\n");
    } else {
        printf("Variables:\n");
        for (int i = 0; i < currentSymTab->count; i++) {
            printf("  [%d] %s (%s) -> offset %d\n", 
                   i, currentSymTab->vars[i].name, 
                   currentSymTab->vars[i].type == TYPE_FLOAT ? "float" : 
                   currentSymTab->vars[i].type == TYPE_VOID ? "void" : "int",
                   currentSymTab->vars[i].offset);
        }
    }
    printf("==========================\n\n");
}

/* Print global symbol table contents */
void printGlobalSymTab() {
    printf("\n=== GLOBAL SYMBOL TABLE STATE ===\n");
    printf("Function Count: %d\n", globalSymTab.func_count);
    if (globalSymTab.func_count == 0) {
        printf("(no functions declared)\n");
    } else {
        printf("Functions:\n");
        for (int i = 0; i < globalSymTab.func_count; i++) {
            printf("  [%d] %s (%s) - %d params, %d local vars\n", 
                   i, globalSymTab.funcs[i].name,
                   globalSymTab.funcs[i].return_type == TYPE_FLOAT ? "float" : 
                   globalSymTab.funcs[i].return_type == TYPE_VOID ? "void" : "int",
                   globalSymTab.funcs[i].param_count,
                   globalSymTab.funcs[i].local_symtab->count);
        }
    }
    printf("================================\n\n");
}
