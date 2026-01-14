#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdbool.h>

// Expression result structure
typedef struct {
    int reg;        // register index
    VarType type;   // TYPE_INT or TYPE_FLOAT
} ExprResult;

// Register pool for better resource management
typedef struct {
    bool tempRegs[8];     // $t0-$t7 availability
    bool floatRegs[32];   // $f0-$f31 availability
    int tempStackTop;     // Stack spill tracking
    int floatStackTop;
} RegisterPool;

// Code generation context
typedef struct {
    FILE* output;
    RegisterPool regPool;
    int floatConstCount;
    int floatConstIndex;
    int floatConstCapacity;
    // Float constants dynamic array
    struct {
        float value;
        int id;
    }* floatConsts;
} CodeGenContext;

// Register management functions
int allocateTempReg(RegisterPool* pool);
int allocateFloatReg(RegisterPool* pool);
void releaseTempReg(RegisterPool* pool, int reg);
void releaseFloatReg(RegisterPool* pool, int reg);
void initRegisterPool(RegisterPool* pool);

// Code generation functions
ExprResult genExpr(CodeGenContext* ctx, ASTNode* node);
void genStmt(CodeGenContext* ctx, ASTNode* node);
void generateMIPS(ASTNode* root, const char* filename);

#endif