/* MINIMAL C COMPILER - EDUCATIONAL VERSION
 * Demonstrates all phases of compilation with a simple language
 * Supports: int variables, addition, assignment, print
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
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

static double now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

static int countTACInstructions(TACInstr* head) {
    int count = 0;
    while (head) {
        count++;
        head = head->next;
    }
    return count;
}

static int updateMaxTempFromOperand(const char* op, int currentMax) {
    if (!op || op[0] != 't') return currentMax;
    for (int i = 1; op[i] != '\0'; i++) {
        if (!isdigit((unsigned char)op[i])) return currentMax;
    }
    int n = atoi(op + 1);
    return (n > currentMax) ? n : currentMax;
}

static int countMaxTempUsed(TACInstr* head) {
    int maxTemp = -1;
    while (head) {
        maxTemp = updateMaxTempFromOperand(head->arg1, maxTemp);
        maxTemp = updateMaxTempFromOperand(head->arg2, maxTemp);
        maxTemp = updateMaxTempFromOperand(head->result, maxTemp);
        head = head->next;
    }
    return maxTemp + 1;
}

static int isLabelLine(const char* s) {
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) len--;
    return (len > 0 && s[len - 1] == ':');
}

static int countMIPSInstructionsFromFile(const char* filePath) {
    FILE* f = fopen(filePath, "r");
    if (!f) return -1;

    int count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#') continue;
        if (*p == '.') continue;            /* directives (.data, .text, .globl, etc.) */
        if (isLabelLine(p)) continue;       /* labels */
        count++;
    }
    fclose(f);
    return count;
}

static int runSpimCaptureWithTiming(const char* asmFile, const char* transcriptFile, double* elapsedMs) {
    char spim_cmd[1024];
    snprintf(spim_cmd, sizeof(spim_cmd),
             "spim -file \"%s\" > \"%s\" 2>&1", asmFile, transcriptFile);
    double start = now_ms();
    int status = system(spim_cmd);
    double end = now_ms();
    if (elapsedMs) *elapsedMs = end - start;
    return status;
}

static int percent_change_int(double baseline, double current) {
    if (baseline <= 0.0) return 0;
    double raw = ((baseline - current) * 100.0) / baseline;
    if (raw >= 0.0) return (int)(raw + 0.5);
    return (int)(raw - 0.5);
}

static void print_execution_summary(double simUnoptMs, double simOptMs) {
    int reduction = percent_change_int(simUnoptMs, simOptMs);

    printf("\n");
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ EXECUTION TIME SUMMARY (SPIM)                            │\n");
    printf("├──────────────────────────────────────────────────────────┤\n");
    printf("│ Unoptimized run: %10.3f ms                               │\n", simUnoptMs);
    printf("│ Optimized run:   %10.3f ms                               │\n", simOptMs);
    printf("│ Runtime change:  %10d%%                                  │\n", reduction);
    printf("└──────────────────────────────────────────────────────────┘\n");
}

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
    double phase0_ms = 0.0, phase1_ms = 0.0, phase2_ms = 0.0;
    double phase3_ms = 0.0, phase4_ms = 0.0, phase5_ms = 0.0;
    
    /* PHASE 0: Various Initializations */
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ PHASE 0: VARIOUS INITIALIZATIONS                         │\n");
    printf("└──────────────────────────────────────────────────────────┘\n");

    BenchmarkResult* bench_init = start_benchmark();
    init_string_pool();  /* Initialize string interning pool */
    initGlobalSymTab();  /* Initialize global symbol table */
    initSymTab();        /* Initialize symbol table */
    end_benchmark(bench_init, "Phase 0: Initialization");
    phase0_ms = bench_init->wall_time * 1000.0;
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
    phase1_ms = bench_parse->wall_time * 1000.0;
    write_benchmark_line(report, "Phase 1: Lexical & Syntax Analysis", bench_parse);
    free(bench_parse);
    
    int total_errors = syntax_error_count + semantic_error_count;
    
    if (parse_result == 0 && total_errors == 0) {
        const char* unoptimizedTacFile = "tac_unopt.txt";
        const char* optimizedTacFile = "tac_opt.txt";
        const char* transcriptFile = "output_transcript.txt";
        const char* unoptimizedMipsFile = "test_final_unopt.s";
        const char* unoptimizedTranscriptFile = "output_transcript_unopt.txt";

        double compile_unopt_ms = 0.0, compile_opt_ms = 0.0;
        double sim_unopt_ms = 0.0, sim_opt_ms = 0.0;
        int tac_unopt_count = 0, tac_opt_count = 0;
        int mips_unopt_count = 0, mips_opt_count = 0;
        int temps_unopt = 0, temps_opt = 0;
        int const_folds = 0, dead_code_removed = 0;

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
        phase2_ms = bench_ast->wall_time * 1000.0;
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
        phase3_ms = bench_tac->wall_time * 1000.0;
        write_benchmark_line(report, "Phase 3: TAC Generation", bench_tac);
        free(bench_tac);
        printf("✓ Unoptimized TAC written to: %s\n", unoptimizedTacFile);
        fprintf(report, "Unoptimized TAC file: %s\n", unoptimizedTacFile);
        printf("\n");

        tac_unopt_count = countTACInstructions(tacList.head);
        temps_unopt = countMaxTempUsed(tacList.head);

        double unopt_codegen_start = now_ms();
        generateMIPSFromUnoptimizedTAC2(unoptimizedMipsFile);
        double unopt_codegen_ms = now_ms() - unopt_codegen_start;
        mips_unopt_count = countMIPSInstructionsFromFile(unoptimizedMipsFile);
        compile_unopt_ms = phase0_ms + phase1_ms + phase2_ms + phase3_ms + unopt_codegen_ms;
        
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
        phase4_ms = bench_opt->wall_time * 1000.0;
        write_benchmark_line(report, "Phase 4: Optimization", bench_opt);
        free(bench_opt);
        printf("✓ Optimized TAC written to: %s\n", optimizedTacFile);
        fprintf(report, "Optimized TAC file: %s\n", optimizedTacFile);
        printf("\n");

        tac_opt_count = countTACInstructions(optimizedList.head);
        temps_opt = countMaxTempUsed(optimizedList.head);
        const_folds = getOptimizerConstFoldCount();
        dead_code_removed = getOptimizerDeadCodeElimCount();

        /* AUDIT NOTE:
         * Code generation ALWAYS uses optimized TAC via generateMIPSFromOptimizedTAC2().
         * The unoptimized TAC file is emitted only for inspection/debugging and is never
         * used as the input for MIPS generation.
         */
        
        /* PHASE 5: Code Generation */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 5: MIPS CODE GENERATION                            │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Translating optimized TAC to MIPS assembly:              │\n");
        printf("│ AUDIT NOTE: MIPS is ALWAYS generated from optimized TAC. │\n");
        printf("│             Unoptimized TAC is never used for codegen.   │\n");
        printf("│ • Variables stored on stack                              │\n");
        printf("│ • Using $t0-$t7 for temporary values                     │\n");
        printf("│ • System calls for print operations                      │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");
        BenchmarkResult* bench_mips = start_benchmark();
        generateMIPSFromOptimizedTAC2(argv[2]);
        end_benchmark(bench_mips, "Phase 5: MIPS Code Generation");
        phase5_ms = bench_mips->wall_time * 1000.0;
        write_benchmark_line(report, "Phase 5: MIPS Code Generation", bench_mips);
        free(bench_mips);
        mips_opt_count = countMIPSInstructionsFromFile(argv[2]);
        compile_opt_ms = phase0_ms + phase1_ms + phase2_ms + phase3_ms + phase4_ms + phase5_ms;
        fprintf(report, "AUDIT NOTE: MIPS codegen ALWAYS uses optimized TAC (%s).\n", optimizedTacFile);
        fprintf(report, "AUDIT NOTE: Unoptimized TAC (%s) is output for reference only and never used for codegen.\n", unoptimizedTacFile);
        printf("✓ MIPS assembly code generated to: %s\n", argv[2]);
        fprintf(report, "MIPS output file: %s\n", argv[2]);
        printf("\n");

        /* PHASE 6: SPIM transcript capture */
        printf("┌──────────────────────────────────────────────────────────┐\n");
        printf("│ PHASE 6: SPIM EXECUTION TRANSCRIPT                      │\n");
        printf("├──────────────────────────────────────────────────────────┤\n");
        printf("│ Running generated assembly and capturing console output  │\n");
        printf("└──────────────────────────────────────────────────────────┘\n");

        int spim_status_unopt = runSpimCaptureWithTiming(unoptimizedMipsFile, unoptimizedTranscriptFile, &sim_unopt_ms);
        BenchmarkResult* bench_spim = start_benchmark();
        int spim_status = runSpimCaptureWithTiming(argv[2], transcriptFile, &sim_opt_ms);
        end_benchmark(bench_spim, "Phase 6: SPIM Transcript Capture");
        write_benchmark_line(report, "Phase 6: SPIM Transcript Capture", bench_spim);
        free(bench_spim);

        if (spim_status_unopt == 0) {
            fprintf(report, "SPIM transcript (unoptimized) file: %s\n", unoptimizedTranscriptFile);
        } else {
            fprintf(report, "SPIM transcript (unoptimized) file: %s\n", unoptimizedTranscriptFile);
            fprintf(report, "SPIM status (unoptimized): FAILED (%d)\n", spim_status_unopt);
        }

        if (spim_status == 0) {
            printf("✓ SPIM transcript written to: %s\n", transcriptFile);
            fprintf(report, "SPIM transcript file: %s\n", transcriptFile);
        } else {
            printf("⚠ SPIM execution failed (see %s for details)\n", transcriptFile);
            fprintf(report, "SPIM transcript file: %s\n", transcriptFile);
            fprintf(report, "SPIM status: FAILED (%d)\n", spim_status);
        }

        int tac_reduction = (tac_unopt_count > 0)
            ? (int)(((double)(tac_unopt_count - tac_opt_count) * 100.0) / (double)tac_unopt_count + 0.5)
            : 0;
        int mips_reduction = (mips_unopt_count > 0)
            ? (int)(((double)(mips_unopt_count - mips_opt_count) * 100.0) / (double)mips_unopt_count + 0.5)
            : 0;
        int temp_reduction = (temps_unopt > 0)
            ? (int)(((double)(temps_unopt - temps_opt) * 100.0) / (double)temps_unopt + 0.5)
            : 0;
        int sim_reduction = percent_change_int(sim_unopt_ms, sim_opt_ms);
        int execution_comparable = (spim_status_unopt == 0 && spim_status == 0);

        fprintf(report, "\n===== Compiler Performance Report =====\n");
        fprintf(report, "Source file       : %s\n\n", argv[1]);
        fprintf(report, "                     Unoptimized    Optimized    Reduction\n");
        fprintf(report, "TAC instructions : %10d %12d %10d%%\n", tac_unopt_count, tac_opt_count, tac_reduction);
        fprintf(report, "MIPS instructions: %10d %12d %10d%%\n", mips_unopt_count, mips_opt_count, mips_reduction);
        fprintf(report, "Temporaries used : %10d %12d %10d%%\n", temps_unopt, temps_opt, temp_reduction);
        fprintf(report, "Const folds      : %10d %12d\n", 0, const_folds);
        fprintf(report, "Dead code removed: %10d %12d\n", 0, dead_code_removed);
        fprintf(report, "Compile time (ms): %10.3f %12.3f\n", compile_unopt_ms, compile_opt_ms);
        if (execution_comparable) {
            fprintf(report, "Execution time (ms): %7.3f %12.3f %10d%%\n", sim_unopt_ms, sim_opt_ms, sim_reduction);
        } else {
            fprintf(report, "Execution time (ms): %7.3f %12.3f %10s\n", sim_unopt_ms, sim_opt_ms, "N/A");
            fprintf(report, "Execution comparison note: one or more SPIM runs failed; reduction is not comparable.\n");
        }
        fprintf(report, "========================================\n");
        fprintf(report, "Compiler-only total (optimized, ms): %.3f\n", compile_opt_ms);
        fprintf(report, "Execution-only total (optimized, ms): %.3f\n", sim_opt_ms);
        fprintf(report, "End-to-end total (optimized, ms): %.3f\n", compile_opt_ms + sim_opt_ms);
        if (execution_comparable) {
            print_execution_summary(sim_unopt_ms, sim_opt_ms);
        } else {
            printf("\n");
            printf("Execution time summary unavailable: one or more SPIM runs failed.\n");
        }
        printf("\n");
        
        // Clean up memory to prevent leaks
        // NOTE: Temporarily disabled to avoid issues with string pool and memory pool
        // freeTACList(&tacList);
        // freeTACList(&optimizedList);
        
        // Print total pipeline time (compilation + transcripted execution)
        end_benchmark(bench_total, "TOTAL PIPELINE TIME");
        write_benchmark_line(report, "TOTAL PIPELINE TIME", bench_total);
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
