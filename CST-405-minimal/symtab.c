/* SYMBOL TABLE IMPLEMENTATION
 * Manages variable declarations and lookups
 * Essential for semantic analysis (checking if variables are declared)
 * Provides memory layout information for code generation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
/* External declarations for line tracking */
extern int yyline;
/* Global symbol table instance */
SymbolTable symtab;
/* Initialize an empty symbol table */
void initSymTab() {
    symtab.count = 0;       /* No variables yet */
    symtab.nextOffset = 0;  /* Start at stack offset 0 */
    printf("SYMBOL TABLE: Initialized\n");
    printSymTab();
}
/* Add a new variable to the symbol table */
int addVar(char* name, VarType type) {
    /* Check for duplicate declaration */
    if (isVarDeclared(name)) {
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Variable '%s' already declared\n", name);
        fprintf(stderr, "💡 Suggestion: Use a different variable name or\n");
        fprintf(stderr, "   remove the first declaration of '%s'\n\n", name);
        return -1;  /* Error: variable already exists */
    }
    /* Add new symbol entry */
    symtab.vars[symtab.count].name = strdup(name);
    symtab.vars[symtab.count].offset = symtab.nextOffset;
    symtab.vars[symtab.count].type = type;
    /* Advance offset by 4 bytes (size of int/float in MIPS) */
    symtab.nextOffset += 4;
    symtab.count++;
    printf("SYMBOL TABLE: Added variable '%s' (%s) at offset %d\n", 
           name, type == TYPE_FLOAT ? "float" : "int", symtab.vars[symtab.count - 1].offset);
    printSymTab();
    /* Return the offset for this variable */
    return symtab.vars[symtab.count - 1].offset;
}
/* Look up a variable's stack offset */
int getVarOffset(char* name) {
    /* Linear search through symbol table */
    for (int i = 0; i < symtab.count; i++) {
        if (strcmp(symtab.vars[i].name, name) == 0) {
            printf("SYMBOL TABLE: Found variable '%s' at offset %d\n", name, symtab.vars[i].offset);
            return symtab.vars[i].offset;  /* Found it */
        }
    }

    fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
    fprintf(stderr, "   Variable '%s' used but not declared\n", name);
    fprintf(stderr, "💡 Suggestion: Declare variable before use:\n");
    fprintf(stderr, "   %s %s;\n", 
           name[0] >= 'a' && name[0] <= 'z' ? "int" : "float", name);
    fprintf(stderr, "   Or check for spelling mistakes\n\n");
    return -1;  /* Variable not found - semantic error */
}
/* Get a variable's type */
VarType getVarType(char* name) {
    /* Linear search through symbol table */
    for (int i = 0; i < symtab.count; i++) {
        if (strcmp(symtab.vars[i].name, name) == 0) {
            return symtab.vars[i].type;  /* Found it */
        }
    }
    return TYPE_INT;  /* Default to int if not found */
}
/* Check if a variable has been declared */
int isVarDeclared(char* name) {
    return getVarOffset(name) != -1;  /* True if found, false otherwise */
}
/* Print current symbol table contents for debugging/tracing */
void printSymTab() {
    printf("\n=== SYMBOL TABLE STATE ===\n");
    printf("Count: %d, Next Offset: %d\n", symtab.count, symtab.nextOffset);
    if (symtab.count == 0) {
        printf("(empty)\n");
    } else {
        printf("Variables:\n");
        for (int i = 0; i < symtab.count; i++) {
            printf("  [%d] %s (%s) -> offset %d\n", 
                   i, symtab.vars[i].name, 
                   symtab.vars[i].type == TYPE_FLOAT ? "float" : "int",
                   symtab.vars[i].offset);
        }
    }
    printf("==========================\n\n");
}
