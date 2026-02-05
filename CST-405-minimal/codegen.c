#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "codegen.h"
#include "ast.h"
#include "symtab.h"

/* External declarations for line tracking */
extern int yyline;

// Register allocator (simple LRU for $t0-$t7)
typedef struct {
    int reg_index;
    char* var_name;  // Variable currently in this register
    int is_free;
    int last_used;   // For LRU eviction
} Register;

typedef struct {
    Register temp_regs[8];  // $t0-$t7
    int clock;              // For LRU tracking
} RegisterAllocator;

static RegisterAllocator reg_alloc;

static void init_register_allocator() {
    for (int i = 0; i < 8; i++) {
        reg_alloc.temp_regs[i].reg_index = i;
        reg_alloc.temp_regs[i].is_free = 1;
        reg_alloc.temp_regs[i].var_name = NULL;
        reg_alloc.temp_regs[i].last_used = 0;
    }
    reg_alloc.clock = 0;
}

static int find_var_reg(const char* var) {
    for (int i = 0; i < 8; i++) {
        if (reg_alloc.temp_regs[i].var_name &&
            strcmp(reg_alloc.temp_regs[i].var_name, var) == 0) {
            return i;
        }
    }
    return -1;
}

static void invalidate_var_cache(CodeGenContext* ctx, const char* var) {
    int idx = find_var_reg(var);
    if (idx >= 0) {
        free(reg_alloc.temp_regs[idx].var_name);
        reg_alloc.temp_regs[idx].var_name = NULL;
        reg_alloc.temp_regs[idx].is_free = 1;
        ctx->regPool.tempRegs[idx] = false;
    }
}

static int allocate_var_reg(CodeGenContext* ctx, const char* var, int* pinned) {
    // Already cached
    int idx = find_var_reg(var);
    if (idx >= 0) {
        reg_alloc.temp_regs[idx].last_used = reg_alloc.clock++;
        if (pinned) *pinned = 1;
        return reg_alloc.temp_regs[idx].reg_index;
    }

    // Find a free register not used by temp pool
    for (int i = 0; i < 8; i++) {
        if (reg_alloc.temp_regs[i].is_free && !ctx->regPool.tempRegs[i]) {
            reg_alloc.temp_regs[i].is_free = 0;
            reg_alloc.temp_regs[i].var_name = strdup(var);
            reg_alloc.temp_regs[i].last_used = reg_alloc.clock++;
            ctx->regPool.tempRegs[i] = true;

            if (isVarDeclared((char*)var)) {
                int offset = getVarOffset((char*)var);
                fprintf(ctx->output, "    lw $t%d, %d($sp)\n", i, offset);
            }
            if (pinned) *pinned = 1;
            return i;
        }
    }

    // Evict LRU cached var
    int lru_idx = -1;
    int min_time = INT_MAX;
    for (int i = 0; i < 8; i++) {
        if (!reg_alloc.temp_regs[i].is_free &&
            reg_alloc.temp_regs[i].last_used < min_time) {
            min_time = reg_alloc.temp_regs[i].last_used;
            lru_idx = i;
        }
    }

    if (lru_idx >= 0) {
        if (reg_alloc.temp_regs[lru_idx].var_name) {
            int offset = getVarOffset(reg_alloc.temp_regs[lru_idx].var_name);
            fprintf(ctx->output, "    sw $t%d, %d($sp)\n", lru_idx, offset);
            free(reg_alloc.temp_regs[lru_idx].var_name);
        }

        reg_alloc.temp_regs[lru_idx].var_name = strdup(var);
        reg_alloc.temp_regs[lru_idx].last_used = reg_alloc.clock++;
        reg_alloc.temp_regs[lru_idx].is_free = 0;
        ctx->regPool.tempRegs[lru_idx] = true;

        if (isVarDeclared((char*)var)) {
            int offset = getVarOffset((char*)var);
            fprintf(ctx->output, "    lw $t%d, %d($sp)\n", lru_idx, offset);
        }

        if (pinned) *pinned = 1;
        return lru_idx;
    }

    // Fallback: allocate a temp register and load (not cached)
    int temp_reg = allocateTempReg(&ctx->regPool);
    int offset = getVarOffset((char*)var);
    fprintf(ctx->output, "    lw $t%d, %d($sp)\n", temp_reg, offset);
    if (pinned) *pinned = 0;
    return temp_reg;
}

// Register pool management functions
void initRegisterPool(RegisterPool* pool) {
    for (int i = 0; i < 8; i++) {
        pool->tempRegs[i] = false;  // All temp registers available
    }
    for (int i = 0; i < 32; i++) {
        pool->floatRegs[i] = false; // All float registers available
    }
    pool->tempStackTop = 0;
    pool->floatStackTop = 0;
}

int allocateTempReg(RegisterPool* pool) {
    // Find first available temp register
    for (int i = 0; i < 8; i++) {
        if (!pool->tempRegs[i]) {
            pool->tempRegs[i] = true;
            return i;
        }
    }
    
    // If no registers available, implement spilling (for now, return 0)
    // In a real implementation, you'd spill to stack
    fprintf(stderr, "Warning: No available temp registers, reusing $t0\n");
    return 0;
}

int allocateFloatReg(RegisterPool* pool) {
    // Find first available float register
    for (int i = 0; i < 32; i++) {
        if (!pool->floatRegs[i]) {
            pool->floatRegs[i] = true;
            return i;
        }
    }
    
    // If no registers available, implement spilling
    fprintf(stderr, "Warning: No available float registers, reusing $f0\n");
    return 0;
}

void releaseTempReg(RegisterPool* pool, int reg) {
    if (reg >= 0 && reg < 8) {
        pool->tempRegs[reg] = false;
    }
}

void releaseFloatReg(RegisterPool* pool, int reg) {
    if (reg >= 0 && reg < 32) {
        pool->floatRegs[reg] = false;
    }
}

// Float constants management with dynamic allocation
int addFloatConst(CodeGenContext* ctx, float value) {
    // Check if constant already exists
    for (int i = 0; i < ctx->floatConstIndex; i++) {
        if (ctx->floatConsts[i].value == value) {
            return ctx->floatConsts[i].id;
        }
    }
    
    // Expand array if needed
    if (ctx->floatConstIndex >= ctx->floatConstCapacity) {
        int newCapacity = ctx->floatConstCapacity == 0 ? 16 : ctx->floatConstCapacity * 2;
        ctx->floatConsts = realloc(ctx->floatConsts, newCapacity * sizeof(ctx->floatConsts[0]));
        ctx->floatConstCapacity = newCapacity;
    }
    
    // Add new constant
    ctx->floatConsts[ctx->floatConstIndex].value = value;
    ctx->floatConsts[ctx->floatConstIndex].id = ctx->floatConstCount++;
    return ctx->floatConsts[ctx->floatConstIndex++].id;
}

int getFloatConstID(CodeGenContext* ctx, float value) {
    for (int i = 0; i < ctx->floatConstIndex; i++) {
        if (ctx->floatConsts[i].value == value) {
            return ctx->floatConsts[i].id;
        }
    }
    return -1; // Should never happen if collectFloatConsts was called
}

// Collect all float constants from AST
void collectFloatConsts(CodeGenContext* ctx, ASTNode* node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_FLT:
            addFloatConst(ctx, node->data.flt);
            break;
        case NODE_BINOP:
            collectFloatConsts(ctx, node->data.binop.left);
            collectFloatConsts(ctx, node->data.binop.right);
            break;
        case NODE_ASSIGN:
            collectFloatConsts(ctx, node->data.assign.value);
            break;
        case NODE_PRINT:
            collectFloatConsts(ctx, node->data.expr);
            break;
        case NODE_STMT_LIST:
            collectFloatConsts(ctx, node->data.stmtlist.stmt);
            collectFloatConsts(ctx, node->data.stmtlist.next);
            break;
        default:
            break;
    }
}

// Emit float constants to .data section
void emitFloatConsts(CodeGenContext* ctx) {
    for (int i = 0; i < ctx->floatConstIndex; i++) {
        fprintf(ctx->output, "flt_%d: .float %f\n", ctx->floatConsts[i].id, ctx->floatConsts[i].value);
    }
}
ExprResult genExpr(CodeGenContext* ctx, ASTNode* node) {
    ExprResult res;

    if (!node) {
        res.reg = -1;
        res.type = TYPE_INT;
        res.is_pinned = 0;
        return res;
    }

    switch (node->type) {

        case NODE_NUM: {
            res.reg = allocateTempReg(&ctx->regPool);
            res.type = TYPE_INT;
            res.is_pinned = 0;
            fprintf(ctx->output, "    li $t%d, %d\n", res.reg, node->data.num);
            return res;
        }

        case NODE_FLT: {
            res.reg = allocateFloatReg(&ctx->regPool);
            res.type = TYPE_FLOAT;
            res.is_pinned = 0;
            int id = getFloatConstID(ctx, node->data.flt);
            fprintf(ctx->output, "    l.s $f%d, flt_%d\n", res.reg, id);
            return res;
        }

        case NODE_VAR: {
            int offset = getVarOffset(node->data.var.name);
            VarType t = getVarType(node->data.var.name);

            if (offset == -1) {
                fprintf(stderr, "Error: Variable %s not declared\n", node->data.var.name);
                exit(1);
            }

            res.type = t;
            res.is_pinned = 0;

            if (t == TYPE_FLOAT) {
                res.reg = allocateFloatReg(&ctx->regPool);
                fprintf(ctx->output, "    l.s $f%d, %d($sp)\n", res.reg, offset);
            } else {
                int pinned = 0;
                res.reg = allocate_var_reg(ctx, node->data.var.name, &pinned);
                res.is_pinned = pinned;
            }

            return res;
        }

        case NODE_BINOP: {
            ExprResult L = genExpr(ctx, node->data.binop.left);
            ExprResult R = genExpr(ctx, node->data.binop.right);

            // Result type promotion
            if (L.type == TYPE_FLOAT || R.type == TYPE_FLOAT) {
                res.type = TYPE_FLOAT;
                res.is_pinned = 0;

                int lf = L.reg;
                int rf = R.reg;

                // int → float conversion
                if (L.type == TYPE_INT) {
                    lf = allocateFloatReg(&ctx->regPool);
                    fprintf(ctx->output, "    mtc1 $t%d, $f%d\n", L.reg, lf);
                    fprintf(ctx->output, "    cvt.s.w $f%d, $f%d\n", lf, lf);
                    if (!L.is_pinned) releaseTempReg(&ctx->regPool, L.reg);
                }

                if (R.type == TYPE_INT) {
                    rf = allocateFloatReg(&ctx->regPool);
                    fprintf(ctx->output, "    mtc1 $t%d, $f%d\n", R.reg, rf);
                    fprintf(ctx->output, "    cvt.s.w $f%d, $f%d\n", rf, rf);
                    if (!R.is_pinned) releaseTempReg(&ctx->regPool, R.reg);
                }

                res.reg = allocateFloatReg(&ctx->regPool);

                switch (node->data.binop.op) {
                    case '+':
                        fprintf(ctx->output, "    add.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    case '-':
                        fprintf(ctx->output, "    sub.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    case '*':
                        fprintf(ctx->output, "    mul.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    case '/':
                        fprintf(ctx->output, "    div.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    default:
                        fprintf(stderr, "Unknown operator %c\n", node->data.binop.op);
                        exit(1);
                }

                // Release temporary registers
                if (L.type == TYPE_FLOAT) releaseFloatReg(&ctx->regPool, lf);
                if (R.type == TYPE_FLOAT) releaseFloatReg(&ctx->regPool, rf);

                return res;
            }

            // Integer-only path
            res.type = TYPE_INT;
            res.reg = allocateTempReg(&ctx->regPool);
            res.is_pinned = 0;

            switch (node->data.binop.op) {
                case '+':
                    fprintf(ctx->output, "    add $t%d, $t%d, $t%d\n", res.reg, L.reg, R.reg);
                    break;
                case '-':
                    fprintf(ctx->output, "    sub $t%d, $t%d, $t%d\n", res.reg, L.reg, R.reg);
                    break;
                case '*':
                    fprintf(ctx->output, "    mult $t%d, $t%d\n", L.reg, R.reg);
                    fprintf(ctx->output, "    mflo $t%d\n", res.reg);
                    break;
                case '/':
                    fprintf(ctx->output, "    div $t%d, $t%d\n", L.reg, R.reg);
                    fprintf(ctx->output, "    mflo $t%d\n", res.reg);
                    break;
                default:
                    fprintf(stderr, "Unknown operator %c\n", node->data.binop.op);
                    exit(1);
            }

            // Release operand registers
            if (!L.is_pinned) releaseTempReg(&ctx->regPool, L.reg);
            if (!R.is_pinned) releaseTempReg(&ctx->regPool, R.reg);

            return res;
        }

        default:
            res.reg = -1;
            res.type = TYPE_INT;
            res.is_pinned = 0;
            return res;
    }
}

void genStmt(CodeGenContext* ctx, ASTNode* node) {
    if (!node) return;

    switch(node->type) {

        case NODE_DECL: {
            int offset = addVar(node->data.var.name, node->data.var.type);
            if (offset == -1) {
                fprintf(stderr, "Error: Variable %s already declared\n",
                        node->data.var.name);
                exit(1);
            }

            fprintf(ctx->output, "    # Declared %s (%s) at offset %d\n",
                    node->data.var.name,
                    node->data.var.type == TYPE_FLOAT ? "float" : "int",
                    offset);
            break;
        }

        case NODE_ASSIGN: {
            int offset = getVarOffset(node->data.assign.var);
            if (offset == -1) {
                fprintf(stderr, "Error: Variable %s not declared\n",
                        node->data.assign.var);
                exit(1);
            }

            VarType varType = getVarType(node->data.assign.var);
            ExprResult val = genExpr(ctx, node->data.assign.value);

            // Exact match
            if (varType == val.type) {
                if (varType == TYPE_FLOAT) {
                    fprintf(ctx->output, "    s.s $f%d, %d($sp)\n", val.reg, offset);
                    releaseFloatReg(&ctx->regPool, val.reg);
                } else {
                    fprintf(ctx->output, "    sw $t%d, %d($sp)\n", val.reg, offset);
                    if (!val.is_pinned) releaseTempReg(&ctx->regPool, val.reg);
                }
                if (varType == TYPE_INT) {
                    invalidate_var_cache(ctx, node->data.assign.var);
                }
                break;
            }
            
            // Allow int→float promotion (C-like behavior)
            if (varType == TYPE_FLOAT && val.type == TYPE_INT) {
                int ftmp = allocateFloatReg(&ctx->regPool);
                fprintf(ctx->output, "    mtc1 $t%d, $f%d\n", val.reg, ftmp);
                fprintf(ctx->output, "    cvt.s.w $f%d, $f%d\n", ftmp, ftmp);
                fprintf(ctx->output, "    s.s $f%d, %d($sp)\n", ftmp, offset);
                if (!val.is_pinned) releaseTempReg(&ctx->regPool, val.reg);
                releaseFloatReg(&ctx->regPool, ftmp);
                invalidate_var_cache(ctx, node->data.assign.var);
                break;
            }
            
            // Allow float→int conversion (with truncation, C-like behavior)
            if (varType == TYPE_INT && val.type == TYPE_FLOAT) {
                int itmp = allocateTempReg(&ctx->regPool);
                fprintf(ctx->output, "    trunc.w.s $f%d, $f%d\n", val.reg, val.reg);
                fprintf(ctx->output, "    mfc1 $t%d, $f%d\n", itmp, val.reg);
                fprintf(ctx->output, "    sw $t%d, %d($sp)\n", itmp, offset);
                releaseFloatReg(&ctx->regPool, val.reg);
                releaseTempReg(&ctx->regPool, itmp);
                invalidate_var_cache(ctx, node->data.assign.var);
                break;
            }
            
            // Everything else is illegal
            fprintf(stderr, "\n❌ Type Error at line %d:\n", yyline);
            fprintf(stderr, "   Cannot assign %s value to %s variable '%s'\n", 
                    val.type == TYPE_FLOAT ? "float" : "int",
                    varType == TYPE_FLOAT ? "float" : "int",
                    node->data.assign.var);
            fprintf(stderr, "💡 Possible solutions:\n");
            if (varType == TYPE_FLOAT && val.type == TYPE_INT) {
                fprintf(stderr, "   • This should work - int should convert to float\n");
                fprintf(stderr, "   • Check if variable was properly declared as float\n");
            } else if (varType == TYPE_INT && val.type == TYPE_FLOAT) {
                fprintf(stderr, "   • Use explicit cast: int %s = (int)%s\n", 
                        node->data.assign.var, node->data.assign.var);
                fprintf(stderr, "   • Or change variable '%s' to float type\n", 
                        node->data.assign.var);
            }
            fprintf(stderr, "\n");
            exit(1);
        }

        case NODE_PRINT: {
            ExprResult val = genExpr(ctx, node->data.expr);

            if (val.type == TYPE_FLOAT) {
                fprintf(ctx->output, "    mov.s $f12, $f%d\n", val.reg);
                fprintf(ctx->output, "    li $v0, 2\n");
                fprintf(ctx->output, "    syscall\n");
                releaseFloatReg(&ctx->regPool, val.reg);
            } else {
                fprintf(ctx->output, "    move $a0, $t%d\n", val.reg);
                fprintf(ctx->output, "    li $v0, 1\n");
                fprintf(ctx->output, "    syscall\n");
                if (!val.is_pinned) releaseTempReg(&ctx->regPool, val.reg);
            }

            // newline
            fprintf(ctx->output, "    li $v0, 11\n");
            fprintf(ctx->output, "    li $a0, 10\n");
            fprintf(ctx->output, "    syscall\n");
            break;
        }

        case NODE_STMT_LIST:
            genStmt(ctx, node->data.stmtlist.stmt);
            genStmt(ctx, node->data.stmtlist.next);
            break;

        default:
            break;
    }
}

void generateMIPS(ASTNode* root, const char* filename) {
    // Create and initialize code generation context
    CodeGenContext ctx;
    ctx.output = fopen(filename, "w");
    if (!ctx.output) {
        fprintf(stderr, "Cannot open output file %s\n", filename);
        exit(1);
    }
    
    // Initialize register pool
    initRegisterPool(&ctx.regPool);
    init_register_allocator();
    
    // Initialize float constants
    ctx.floatConstCount = 0;
    ctx.floatConstIndex = 0;
    ctx.floatConstCapacity = 0;
    ctx.floatConsts = NULL;
    
    // Initialize symbol table
    initSymTab();
    
    // First pass: collect all float constants
    collectFloatConsts(&ctx, root);
    
    // MIPS program header
    fprintf(ctx.output, ".data\n");
    
    // Emit float constants
    emitFloatConsts(&ctx);
    
    fprintf(ctx.output, "\n.text\n");
    fprintf(ctx.output, ".globl main\n");
    fprintf(ctx.output, "main:\n");
    
    // Allocate stack space (max 100 variables * 4 bytes)
    fprintf(ctx.output, "    # Allocate stack space\n");
    fprintf(ctx.output, "    addi $sp, $sp, -400\n\n");
    
    // Generate code for statements
    genStmt(&ctx, root);
    
    // Program exit
    fprintf(ctx.output, "\n    # Exit program\n");
    fprintf(ctx.output, "    addi $sp, $sp, 400\n");
    fprintf(ctx.output, "    li $v0, 10\n");
    fprintf(ctx.output, "    syscall\n");
    
    // Cleanup
    fclose(ctx.output);
    if (ctx.floatConsts) {
        free(ctx.floatConsts);
    }
}
