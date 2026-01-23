#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

/* OPTIMIZER FUNCTIONS */
void optimizeTAC();                                                /* Apply optimizations to TAC */
void printTACToFile(const char* filename);                        /* Print unoptimized TAC to file */
void printOptimizedTAC();                                          /* Display optimized TAC to console */
void printOptimizedTACToFile(const char* filename);               /* Print optimized TAC to file */
void generateMIPSFromOptimizedTAC(const char* filename);          /* Generate MIPS from optimized TAC */

#endif
