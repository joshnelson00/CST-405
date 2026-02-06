/* MINIMAL C COMPILER - EDUCATIONAL VERSION
 * Demonstrates all phases of compilation with a simple language
 * Supports: int variables, addition, assignment, print
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "codegen.h"
#include "optimizer.h"
#include "tac.h"
#include "benchmark.h"

// External declarations for TAC lists
extern TACList tacList;
extern TACList optimizedList;

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
    
    // Start total compilation timer
    BenchmarkResult* bench_total = start_benchmark();
    
    /* PHASE 0: Various Initializations */
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ PHASE 0: VARIOUS INITIALIZATIONS                         │\n");
    printf("└──────────────────────────────────────────────────────────┘\n");

    BenchmarkResult* bench_init = start_benchmark();
    initGlobalSymTab();  /* Initialize global symbol table */
    initSymTab();        /* Initialize symbol table */
    end_benchmark(bench_init, "Phase 0: Initialization");
    free(bench_init);

    /* PHASE 1: Lexical and Syntax Analysis */
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ PHASE 1: LEXICAL & SYNTAX ANALYSIS                       │\n");
    printf("├──────────────────────────────────────────────────────────┤\n");
    printf("│ • Reading source file: %s\n", argv[1]);                   
    printf("│ • Tokenizing input (scanner.l)\n");
    printf("│ • Parsing grammar rules (parser.y)\n");
    printf("│ • Building Abstract Syntax Tree\n");
    printf("└──────────────────────────────────────────────────────────┘\n");
    
    BenchmarkResult* bench_parse = start_benchmark();
    int parse_result = yyparse();
    end_benchmark(bench_parse, "Phase 1: Lexical & Syntax Analysis");
    free(bench_parse);
    
    if (parse_result == 0) {
        printf("✓ Parse successful - program is syntactically correct!\n\n");
        
        /* PHASE 2: AST Display */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 2: ABSTRACT SYNTAX TREE (AST)                      │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Tree structure representing the program hierarchy:       │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_ast = start_benchmark();
        printAST(root, 0);
        end_benchmark(bench_ast, "Phase 2: AST Display");
        free(bench_ast);
        printf("\n");
        
        /* PHASE 3: Intermediate Code */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 3: INTERMEDIATE CODE GENERATION                    │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Three-Address Code (TAC) - simplified instructions:      │\n");
        printf("│ • Each instruction has at most 3 operands                │\n");
        printf("│ • Temporary variables (t0, t1, ...) for expressions      │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_tac = start_benchmark();
        initTAC();
        generateTAC(root);
        printTAC();
        printTACToFile2("tac.txt");
        end_benchmark(bench_tac, "Phase 3: TAC Generation");
        free(bench_tac);
        printf("\n");
        
        /* PHASE 4: Optimization */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 4: CODE OPTIMIZATION                               │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Applying optimizations:                                  │\n");
        printf("│ • Constant folding (evaluate compile-time expressions)   │\n");
        printf("│ • Copy propagation (replace variables with values)       │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_opt = start_benchmark();
        optimizeTAC2();
        printOptimizedTAC2();
        printOptimizedTACToFile2("tac-optimized.txt");
        end_benchmark(bench_opt, "Phase 4: Optimization");
        free(bench_opt);
        printf("\n");
        
        /* PHASE 5: Code Generation */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 5: MIPS CODE GENERATION                            │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Translating optimized TAC to MIPS assembly:              │\n");
        printf("│ • Variables stored on stack                              │\n");
        printf("│ • Using $t0-$t7 for temporary values                     │\n");
        printf("│ • System calls for print operations                      │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_mips = start_benchmark();
        generateMIPSFromOptimizedTAC2(argv[2]);
        end_benchmark(bench_mips, "Phase 5: MIPS Code Generation");
        free(bench_mips);
        printf("✓ MIPS assembly code generated to: %s\n", argv[2]);
        printf("\n");
        
        // Clean up memory to prevent leaks
        freeTACList(&tacList);
        freeTACList(&optimizedList);
        
        // Print total compilation time
        end_benchmark(bench_total, "TOTAL COMPILATION TIME");
        free(bench_total);
        
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
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
    return 0;
}
