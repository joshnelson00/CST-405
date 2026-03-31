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
#include "stringpool.h"

// External declarations for TAC lists
extern TACList tacList;
extern TACList optimizedList;

extern int yyparse();
extern FILE* yyin;
extern ASTNode* root;
extern int syntax_error_count;
extern int semantic_error_count;

static void write_benchmark_line(FILE* report, const char* phase, BenchmarkResult* bench) {
    if (!report || !phase || !bench) return;
    fprintf(report, "%s\n", phase);
    fprintf(report, "  CPU Time: %.6f seconds\n", bench->cpu_time);
    fprintf(report, "  Wall Time: %.6f seconds\n", bench->wall_time);
    fprintf(report, "  Memory Delta: %.2f KB\n", bench->peak_memory / 1024.0);
}

int main(int argc, char* argv[]) {
    const char* reportFile = "report.txt";
    FILE* report = fopen(reportFile, "w");
    if (!report) {
        fprintf(stderr, "Error: Cannot open report file '%s'\n", reportFile);
        return 1;
    }

    if (argc != 3) {
        printf("Usage: %s <input.c> <output.s>\n", argv[0]);
        printf("Example: ./minicompiler test.c output.s\n");
        fprintf(report, "Compilation Report\n");
        fprintf(report, "Status: FAILED\n");
        fprintf(report, "Reason: invalid command-line arguments\n");
        fclose(report);
        return 1;
    }

    time_t now = time(NULL);
    fprintf(report, "Compilation Report\n");
    fprintf(report, "Input: %s\n", argv[1]);
    fprintf(report, "Output: %s\n", argv[2]);
    if (now != (time_t)-1) {
        fprintf(report, "Timestamp: %s", ctime(&now));
    }
    fprintf(report, "\n");
    
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", argv[1]);
        fprintf(report, "Status: FAILED\n");
        fprintf(report, "Reason: cannot open input file\n");
        fclose(report);
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
    init_string_pool();  /* Initialize string interning pool */
    initGlobalSymTab();  /* Initialize global symbol table */
    initSymTab();        /* Initialize symbol table */
    end_benchmark(bench_init, "Phase 0: Initialization");
    write_benchmark_line(report, "Phase 0: Initialization", bench_init);
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
    write_benchmark_line(report, "Phase 1: Lexical & Syntax Analysis", bench_parse);
    free(bench_parse);
    
    int total_errors = syntax_error_count + semantic_error_count;
    
    if (parse_result == 0 && total_errors == 0) {
        const char* unoptimizedTacFile = "tac_unopt.txt";
        const char* optimizedTacFile = "tac_opt.txt";
        const char* transcriptFile = "output_transcript.txt";

        fprintf(report, "Status: SUCCESS\n");
        fprintf(report, "Syntax Errors: %d\n", syntax_error_count);
        fprintf(report, "Semantic Errors: %d\n", semantic_error_count);
        fprintf(report, "Total Errors: %d\n\n", total_errors);

        printf("✓ Parse successful - program is syntactically and semantically correct!\n\n");
        
        /* PHASE 2: AST Display */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 2: ABSTRACT SYNTAX TREE (AST)                      │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Tree structure representing the program hierarchy:       │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_ast = start_benchmark();
        printAST(root, 0);
        end_benchmark(bench_ast, "Phase 2: AST Display");
        write_benchmark_line(report, "Phase 2: AST Display", bench_ast);
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
        printTACToFile2(unoptimizedTacFile);
        end_benchmark(bench_tac, "Phase 3: TAC Generation");
        write_benchmark_line(report, "Phase 3: TAC Generation", bench_tac);
        free(bench_tac);
        printf("✓ Unoptimized TAC written to: %s\n", unoptimizedTacFile);
        fprintf(report, "Unoptimized TAC file: %s\n", unoptimizedTacFile);
        printf("\n");
        
        /* PHASE 4: Optimization */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 4: CODE OPTIMIZATION                               │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Applying optimizations:                                  │\n");
        printf("│ • Constant folding  (arithmetic + comparisons, any depth)│\n");
        printf("│ • Copy propagation  (replace vars with values, pre-loop) │\n");
        printf("│ • Dead branch elim. (IF_FALSE <true const> → removed)    │\n");
        printf("│ • Dead loop  elim.  (IF_FALSE 0 → skip entire body)      │\n");
        printf("│ • Proper nesting    (loop depth counter, not bool flag)   │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_opt = start_benchmark();
        optimizeTAC2();
        printOptimizedTAC2();
        printOptimizedTACToFile2(optimizedTacFile);
        end_benchmark(bench_opt, "Phase 4: Optimization");
        write_benchmark_line(report, "Phase 4: Optimization", bench_opt);
        free(bench_opt);
        printf("✓ Optimized TAC written to: %s\n", optimizedTacFile);
        fprintf(report, "Optimized TAC file: %s\n", optimizedTacFile);
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
        write_benchmark_line(report, "Phase 5: MIPS Code Generation", bench_mips);
        free(bench_mips);
        printf("✓ MIPS assembly code generated to: %s\n", argv[2]);
        fprintf(report, "MIPS output file: %s\n", argv[2]);
        printf("\n");

        /* PHASE 6: SPIM transcript capture */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 6: SPIM EXECUTION TRANSCRIPT                      │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Running generated assembly and capturing console output  │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");

        BenchmarkResult* bench_spim = start_benchmark();
        char spim_cmd[1024];
        snprintf(spim_cmd, sizeof(spim_cmd),
                 "spim -file \"%s\" > \"%s\" 2>&1", argv[2], transcriptFile);
        int spim_status = system(spim_cmd);
        end_benchmark(bench_spim, "Phase 6: SPIM Transcript Capture");
        write_benchmark_line(report, "Phase 6: SPIM Transcript Capture", bench_spim);
        free(bench_spim);

        if (spim_status == 0) {
            printf("✓ SPIM transcript written to: %s\n", transcriptFile);
            fprintf(report, "SPIM transcript file: %s\n", transcriptFile);
        } else {
            printf("⚠ SPIM execution failed (see %s for details)\n", transcriptFile);
            fprintf(report, "SPIM transcript file: %s\n", transcriptFile);
            fprintf(report, "SPIM status: FAILED (%d)\n", spim_status);
        }
        printf("\n");
        
        // Clean up memory to prevent leaks
        // NOTE: Temporarily disabled to avoid issues with string pool and memory pool
        // freeTACList(&tacList);
        // freeTACList(&optimizedList);
        
        // Print total compilation time
        end_benchmark(bench_total, "TOTAL COMPILATION TIME");
        write_benchmark_line(report, "TOTAL COMPILATION TIME", bench_total);
        free(bench_total);
        
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
        printf("║                  COMPILATION SUCCESSFUL!                   ║\n");
        printf("║         Run the output file in a MIPS simulator            ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        fprintf(report, "Status: FAILED\n");
        fprintf(report, "Syntax Errors: %d\n", syntax_error_count);
        fprintf(report, "Semantic Errors: %d\n", semantic_error_count);
        fprintf(report, "Total Errors: %d\n\n", total_errors);

        end_benchmark(bench_total, "TOTAL COMPILATION TIME");
        write_benchmark_line(report, "TOTAL COMPILATION TIME", bench_total);
        free(bench_total);

        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                   ❌ COMPILATION FAILED                      ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║                      ERROR SUMMARY                          ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        if (syntax_error_count > 0)
            printf("║  Syntax errors:   %3d                                       ║\n", syntax_error_count);
        if (semantic_error_count > 0)
            printf("║  Semantic errors:  %3d                                      ║\n", semantic_error_count);
        printf("║  ─────────────────────                                      ║\n");
        printf("║  Total errors:    %3d                                       ║\n", total_errors);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  Review the detailed error messages above.                  ║\n");
        printf("║  Each error includes suggestions for how to fix it.        ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  💡 Most Common Fixes:                                      ║\n");
        printf("║    • Add missing semicolons ';' after statements            ║\n");
        printf("║    • Declare all variables before use (int x;)              ║\n");
        printf("║    • Remove duplicate variable/function declarations        ║\n");
        printf("║    • Use correct number of arguments in function calls      ║\n");
        printf("║    • Use positive integers for array sizes                  ║\n");
        printf("║    • Check for unmatched braces '{' '}'                     ║\n");
        printf("║    • Access arrays with subscript: arr[index]               ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        fclose(report);
        fclose(yyin);
        return 1;
    }
    
    fprintf(report, "\nReport file: %s\n", reportFile);
    fclose(report);
    fclose(yyin);
    return 0;
}
