void printTACToFile2(const char* filename);
#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

#include <stdio.h>
void optimizeTAC2();
void generateMIPSFromOptimizedTAC2(const char* filename);
void printOptimizedTAC2();
void printOptimizedTACToFile2(const char* filename);

#endif
