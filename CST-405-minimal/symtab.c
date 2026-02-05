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
SymbolTable symtab = { .verbose = 1 };

// Hash function (djb2)
static unsigned int hash(const char* str) {
    unsigned int h = 5381;
    int c;
    while ((c = *str++)) {
        h = ((h << 5) + h) + c; // h * 33 + c
    }
    return h;
}

void setSymTabVerbose(int enabled) {
    symtab.verbose = enabled ? 1 : 0;
}
/* Initialize an empty symbol table */
void initSymTab() {
    memset(symtab.buckets, 0, sizeof(symtab.buckets));
    symtab.count = 0;       /* No variables yet */
    symtab.nextOffset = 0;  /* Start at stack offset 0 */
    symtab.lookups = 0;
    symtab.collisions = 0;
    if (symtab.verbose) {
        printf("SYMBOL TABLE: Initialized\n");
        printSymTab();
    }
}
/* Add a new variable to the symbol table */
int addVar(char* name, VarType type) {
    unsigned int h = hash(name) % HASH_SIZE;
    SymbolNode* node = symtab.buckets[h];
    // Check for duplicate
    while (node) {
        if (strcmp(node->name, name) == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' already declared\n", name);
            fprintf(stderr, "💡 Suggestion: Use a different variable name or\n");
            fprintf(stderr, "   remove the first declaration of '%s'\n\n", name);
            return -1;
        }
        node = node->next;
    }
    // Add new node
    SymbolNode* newNode = malloc(sizeof(SymbolNode));
    newNode->name = strdup(name);
    newNode->offset = symtab.nextOffset;
    newNode->type = type;
    newNode->next = symtab.buckets[h];
    if (symtab.buckets[h]) symtab.collisions++;  // Collision if bucket not empty
    symtab.buckets[h] = newNode;
    symtab.nextOffset += 4;
    symtab.count++;
    if (symtab.verbose) {
        printf("SYMBOL TABLE: Added variable '%s' (%s) at offset %d\n", 
               name, type == TYPE_FLOAT ? "float" : "int", newNode->offset);
        printSymTab();
    }
    return newNode->offset;
}
/* Look up a variable's stack offset */
int getVarOffset(char* name) {
    unsigned int h = hash(name) % HASH_SIZE;
    SymbolNode* node = symtab.buckets[h];
    symtab.lookups++;
    while (node) {
        if (strcmp(node->name, name) == 0) {
            if (symtab.verbose) {
                printf("SYMBOL TABLE: Found variable '%s' at offset %d\n", name, node->offset);
            }
            return node->offset;  /* Found it */
        }
        node = node->next;
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
    unsigned int h = hash(name) % HASH_SIZE;
    SymbolNode* node = symtab.buckets[h];
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return node->type;
        }
        node = node->next;
    }
    return TYPE_INT;  /* Default to int if not found */
}
/* Check if a variable has been declared */
int isVarDeclared(char* name) {
    unsigned int h = hash(name) % HASH_SIZE;
    SymbolNode* node = symtab.buckets[h];
    while (node) {
        if (strcmp(node->name, name) == 0) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}
/* Print current symbol table contents for debugging/tracing */
void printSymTab() {
    if (!symtab.verbose) {
        return;
    }
    printf("\n=== SYMBOL TABLE STATE ===\n");
    printf("Count: %d, Next Offset: %d, Lookups: %d, Collisions: %d\n", symtab.count, symtab.nextOffset, symtab.lookups, symtab.collisions);
    if (symtab.count == 0) {
        printf("(empty)\n");
    } else {
        printf("Variables:\n");
        for (int i = 0; i < HASH_SIZE; i++) {
            SymbolNode* node = symtab.buckets[i];
            while (node) {
                printf("  [%d] %s (%s) -> offset %d\n", 
                       i, node->name, 
                       node->type == TYPE_FLOAT ? "float" : "int",
                       node->offset);
                node = node->next;
            }
        }
    }
    printf("==========================\n\n");
}
