/* MINIMAL C COMPILER - EDUCATIONAL VERSION
 * Demonstrates all phases of compilation with a simple language
 * Supports: int variables, addition, assignment, print
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen.h"
#include "tac.h"
#include "symtab.h"
#include "semantic.h"
#include "benchmark.h"
#include "stringpool.h"

extern SymbolTable symtab;

void test_symbol_table_performance() {
    setSymTabVerbose(0);
    initSymTab();
    srand(12345);

    // Test with 1000 variables
    BenchmarkResult* insert_bench = start_benchmark();
    for(int i = 0; i < 1000; i++) {
        char varname[20];
        sprintf(varname, "var_%d", i);
        addVar(varname, TYPE_INT);
    }
    end_benchmark(insert_bench, "Symbol Table Insert");
    free(insert_bench);

    // Test lookups
    symtab.lookups = 0;
    BenchmarkResult* lookup_bench = start_benchmark();
    for(int i = 0; i < 10000; i++) {
        char varname[20];
        sprintf(varname, "var_%d", rand() % 1000);
        getVarOffset(varname);
    }
    end_benchmark(lookup_bench, "Symbol Table Lookup");
    free(lookup_bench);

    printf("Entries: %d\n", symtab.count);
    printf("Collisions: %d\n", symtab.collisions);
    printf("Lookups: %d\n", symtab.lookups);
    printf("Load factor: %.3f\n", (double)symtab.count / HASH_SIZE);
}

extern int yyparse();
extern FILE* yyin;
extern ASTNode* root;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input.c> <output.s>\n", argv[0]);
        printf("Example: ./minicompiler test.c output.s\n");
        return 1;
    }
    
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", argv[1]);
        return 1;
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          MINIMAL C COMPILER - EDUCATIONAL VERSION          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    /* PHASE 0: Various Initializations */
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ PHASE 0: VARIOUS INITIALIZATIONS                         │\n");
    printf("└──────────────────────────────────────────────────────────┘\n");

    initSymTab();  /* Initialize symbol table */

    /* PHASE 1: Lexical and Syntax Analysis */
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ PHASE 1: LEXICAL & SYNTAX ANALYSIS                       │\n");
    printf("├──────────────────────────────────────────────────────────┤\n");
    printf("│ • Reading source file: %s\n", argv[1]);                   
    printf("│ • Tokenizing input (scanner.l)\n");
    printf("│ • Parsing grammar rules (parser.y)\n");
    printf("│ • Building Abstract Syntax Tree\n");
    printf("└──────────────────────────────────────────────────────────┘\n");
    
    if (yyparse() == 0) {
        printf("✓ Parse successful - program is syntactically correct!\n\n");
        
        /* PHASE 2: AST Display */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 2: ABSTRACT SYNTAX TREE (AST)                      │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Tree structure representing the program hierarchy:       │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        printAST(root, 0);
        printf("\n");

         /* PHASE 3: Semantic Analysis */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 3: SEMANTIC ANALYSIS                               │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Checking semantic correctness:                           │\n");
        printf("│ • Variables declared before use                          │\n");
        printf("│ • No duplicate declarations                              │\n");
        printf("│ • Type consistency (for future extensions)               │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        initSemantic();
        if (!analyzeProgram(root)) {
            printf("\n✗ Compilation failed due to semantic errors!\n");
            fclose(yyin);
            return 1;
        }
        printf("\n");

        /* PHASE 4: Intermediate Code */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 4: INTERMEDIATE CODE GENERATION                    │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Three-Address Code (TAC) - simplified instructions:      │\n");
        printf("│ • Each instruction has at most 3 operands                │\n");
        printf("│ • Temporary variables (t0, t1, ...) for expressions      │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        initTAC();
        generateTAC(root);
        printTAC();
        printf("\n");
        
        /* PHASE 5: Optimization */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 5: CODE OPTIMIZATION                               │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Applying optimizations:                                  │\n");
        printf("│ • Constant folding (evaluate compile-time expressions)   │\n");
        printf("│ • Copy propagation (replace variables with values)       │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        optimizeTAC();
        printOptimizedTAC();
        printf("\n");
        
        /* PHASE 6: Code Generation */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 6: MIPS CODE GENERATION                            │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Translating to MIPS assembly:                            │\n");
        printf("│ • Variables stored on stack                              │\n");
        printf("│ • Using $t0-$t7 for temporary values                     │\n");
        printf("│ • System calls for print operations                      │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        generateMIPS(root, argv[2]);
        printf("✓ MIPS assembly code generated to: %s\n", argv[2]);
        printf("\n");
        
        printf("╔════════════════════════════════════════════════════════════╗\n");
        printf("║                  COMPILATION SUCCESSFUL!                   ║\n");
        printf("║         Run the output file in a MIPS simulator            ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        printf("\n❌ Compilation Failed!\n");
        printf("┌──────────────────────────────────────────────────┐\n");
        printf("│                  ERROR SUMMARY                   │\n");
        printf("├──────────────────────────────────────────────────┤\n");
        printf("│ Your code has syntax or semantic errors          │\n");
        printf("│ Check the messages above for details             │\n");
        printf("│                                                  │\n");
        printf("│ 💡 Most Common Errors:                           │\n");
        printf("│   • Missing semicolon ';'                        │\n");
        printf("│   • Undeclared variables                         │\n");
        printf("│   • Invalid print syntax                         │\n");
        printf("│   • Type mismatches                              │\n");
        printf("│   • Unmatched parentheses                        │\n");
        printf("└──────────────────────────────────────────────────┘\n");
        printf("\n🔧 Quick fixes to try:\n");
        printf("   1. Add semicolons after each statement\n");
        printf("   2. Declare all variables before use\n");
        printf("   3. Use print(expression) with semicolon\n");
        printf("   4. Check parentheses matching\n");
        printf("   5. Verify variable names are spelled correctly\n");
        return 1;
    }
    
    fclose(yyin);
    printf("\nRunning symbol table performance test...\n");
    test_symbol_table_performance();
    return 0;
}
