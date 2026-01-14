#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "ast.h"
#include "symtab.h"

FILE* output;
int tempReg = 0;
int floatReg = 0;
int floatConstCount = 0;

// Array to store float constants
typedef struct {
    float value;
    int id;
} FloatConst;

typedef struct {
    int reg;        // register index
    VarType type;   // TYPE_INT or TYPE_FLOAT
} ExprResult;

FloatConst floatConsts[100];
int floatConstIndex = 0;

int getNextTemp() {
    int reg = tempReg++;
    if (tempReg > 7) tempReg = 0;  // Reuse $t0-$t7
    return reg;
}

int getNextFloat() {
    int reg = floatReg++;
    if (floatReg > 31) floatReg = 0;  // reuse $f0-$f31
    return reg;
}

// Add or find a float constant, return its ID
int addFloatConst(float value) {
    // Check if constant already exists
    for (int i = 0; i < floatConstIndex; i++) {
        if (floatConsts[i].value == value) {
            return floatConsts[i].id;
        }
    }
    
    // Add new constant
    floatConsts[floatConstIndex].value = value;
    floatConsts[floatConstIndex].id = floatConstCount++;
    return floatConsts[floatConstIndex++].id;
}

// Get the ID for a float constant value
int getFloatConstID(float value) {
    for (int i = 0; i < floatConstIndex; i++) {
        if (floatConsts[i].value == value) {
            return floatConsts[i].id;
        }
    }
    return -1; // Should never happen if collectFloatConsts was called
}

// Collect all float constants from AST
void collectFloatConsts(ASTNode* node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_FLT:
            // Store the ID and use it later - we'll use num field to store the ID
            addFloatConst(node->data.flt);
            break;
        case NODE_BINOP:
            collectFloatConsts(node->data.binop.left);
            collectFloatConsts(node->data.binop.right);
            break;
        case NODE_ASSIGN:
            collectFloatConsts(node->data.assign.value);
            break;
        case NODE_PRINT:
            collectFloatConsts(node->data.expr);
            break;
        case NODE_STMT_LIST:
            collectFloatConsts(node->data.stmtlist.stmt);
            collectFloatConsts(node->data.stmtlist.next);
            break;
        default:
            break;
    }
}

// Emit float constants to .data section
void emitFloatConsts() {
    for (int i = 0; i < floatConstIndex; i++) {
        fprintf(output, "flt_%d: .float %f\n", floatConsts[i].id, floatConsts[i].value);
    }
}
ExprResult genExpr(ASTNode* node) {
    ExprResult res;

    if (!node) {
        res.reg = -1;
        res.type = TYPE_INT;
        return res;
    }

    switch (node->type) {

        case NODE_NUM: {
            res.reg = getNextTemp();
            res.type = TYPE_INT;
            fprintf(output, "    li $t%d, %d\n", res.reg, node->data.num);
            return res;
        }

        case NODE_FLT: {
            res.reg = getNextFloat();
            res.type = TYPE_FLOAT;
            int id = getFloatConstID(node->data.flt);
            fprintf(output, "    l.s $f%d, flt_%d\n", res.reg, id);
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

            if (t == TYPE_FLOAT) {
                res.reg = getNextFloat();
                fprintf(output, "    l.s $f%d, %d($sp)\n", res.reg, offset);
            } else {
                res.reg = getNextTemp();
                fprintf(output, "    lw $t%d, %d($sp)\n", res.reg, offset);
            }

            return res;
        }

        case NODE_BINOP: {
            ExprResult L = genExpr(node->data.binop.left);
            ExprResult R = genExpr(node->data.binop.right);

            // Result type promotion
            if (L.type == TYPE_FLOAT || R.type == TYPE_FLOAT) {
                res.type = TYPE_FLOAT;

                int lf = L.reg;
                int rf = R.reg;

                // int → float conversion
                if (L.type == TYPE_INT) {
                    lf = getNextFloat();
                    fprintf(output, "    mtc1 $t%d, $f%d\n", L.reg, lf);
                    fprintf(output, "    cvt.s.w $f%d, $f%d\n", lf, lf);
                }

                if (R.type == TYPE_INT) {
                    rf = getNextFloat();
                    fprintf(output, "    mtc1 $t%d, $f%d\n", R.reg, rf);
                    fprintf(output, "    cvt.s.w $f%d, $f%d\n", rf, rf);
                }

                res.reg = getNextFloat();

                switch (node->data.binop.op) {
                    case '+':
                        fprintf(output, "    add.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    case '-':
                        fprintf(output, "    sub.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    case '*':
                        fprintf(output, "    mul.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    case '/':
                        fprintf(output, "    div.s $f%d, $f%d, $f%d\n", res.reg, lf, rf);
                        break;
                    default:
                        fprintf(stderr, "Unknown operator %c\n", node->data.binop.op);
                        exit(1);
                }

                return res;
            }

            // Integer-only path
            res.type = TYPE_INT;
            res.reg = getNextTemp();

            switch (node->data.binop.op) {
                case '+':
                    fprintf(output, "    add $t%d, $t%d, $t%d\n", res.reg, L.reg, R.reg);
                    break;
                case '-':
                    fprintf(output, "    sub $t%d, $t%d, $t%d\n", res.reg, L.reg, R.reg);
                    break;
                case '*':
                    fprintf(output, "    mult $t%d, $t%d\n", L.reg, R.reg);
                    fprintf(output, "    mflo $t%d\n", res.reg);
                    break;
                case '/':
                    fprintf(output, "    div $t%d, $t%d\n", L.reg, R.reg);
                    fprintf(output, "    mflo $t%d\n", res.reg);
                    break;
                default:
                    fprintf(stderr, "Unknown operator %c\n", node->data.binop.op);
                    exit(1);
            }

            return res;
        }

        default:
            res.reg = -1;
            res.type = TYPE_INT;
            return res;
    }
}

void genStmt(ASTNode* node) {
    if (!node) return;

    switch(node->type) {

        case NODE_DECL: {
            int offset = addVar(node->data.var.name, node->data.var.type);
            if (offset == -1) {
                fprintf(stderr, "Error: Variable %s already declared\n",
                        node->data.var.name);
                exit(1);
            }

            fprintf(output, "    # Declared %s (%s) at offset %d\n",
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
            ExprResult val = genExpr(node->data.assign.value);

            // Exact match
            if (varType == val.type) {
                if (varType == TYPE_FLOAT) {
                    fprintf(output, "    s.s $f%d, %d($sp)\n", val.reg, offset);
                } else {
                    fprintf(output, "    sw $t%d, %d($sp)\n", val.reg, offset);
                }
                break;
            }
            
            // Allow int→float promotion (C-like behavior)
            if (varType == TYPE_FLOAT && val.type == TYPE_INT) {
                int ftmp = getNextFloat();
                fprintf(output, "    mtc1 $t%d, $f%d\n", val.reg, ftmp);
                fprintf(output, "    cvt.s.w $f%d, $f%d\n", ftmp, ftmp);
                fprintf(output, "    s.s $f%d, %d($sp)\n", ftmp, offset);
                break;
            }
            
            // Allow float→int conversion (with truncation, C-like behavior)
            if (varType == TYPE_INT && val.type == TYPE_FLOAT) {
                int itmp = getNextTemp();
                fprintf(output, "    trunc.w.s $f%d, $f%d\n", val.reg, val.reg);
                fprintf(output, "    mfc1 $t%d, $f%d\n", itmp, val.reg);
                fprintf(output, "    sw $t%d, %d($sp)\n", itmp, offset);
                break;
            }
            
            // Everything else is illegal
            fprintf(stderr, "Type error: assigning %s to %s\n",
                    val.type == TYPE_FLOAT ? "float" : "int",
                    varType == TYPE_FLOAT ? "float" : "int");
            exit(1);
        }

        case NODE_PRINT: {
            ExprResult val = genExpr(node->data.expr);

            if (val.type == TYPE_FLOAT) {
                fprintf(output, "    mov.s $f12, $f%d\n", val.reg);
                fprintf(output, "    li $v0, 2\n");
                fprintf(output, "    syscall\n");
            } else {
                fprintf(output, "    move $a0, $t%d\n", val.reg);
                fprintf(output, "    li $v0, 1\n");
                fprintf(output, "    syscall\n");
            }

            // newline
            fprintf(output, "    li $v0, 11\n");
            fprintf(output, "    li $a0, 10\n");
            fprintf(output, "    syscall\n");
            break;
        }

        case NODE_STMT_LIST:
            genStmt(node->data.stmtlist.stmt);
            genStmt(node->data.stmtlist.next);
            break;

        default:
            break;
    }
}

void generateMIPS(ASTNode* root, const char* filename) {
    output = fopen(filename, "w");
    if (!output) {
        fprintf(stderr, "Cannot open output file %s\n", filename);
        exit(1);
    }
    
    // Initialize symbol table
    initSymTab();
    
    // Reset float constant tracking
    floatConstIndex = 0;
    floatConstCount = 0;
    
    // First pass: collect all float constants
    collectFloatConsts(root);
    
    // MIPS program header
    fprintf(output, ".data\n");
    
    // Emit float constants
    emitFloatConsts();
    
    fprintf(output, "\n.text\n");
    fprintf(output, ".globl main\n");
    fprintf(output, "main:\n");
    
    // Allocate stack space (max 100 variables * 4 bytes)
    fprintf(output, "    # Allocate stack space\n");
    fprintf(output, "    addi $sp, $sp, -400\n\n");
    
    // Generate code for statements
    genStmt(root);
    
    // Program exit
    fprintf(output, "\n    # Exit program\n");
    fprintf(output, "    addi $sp, $sp, 400\n");
    fprintf(output, "    li $v0, 10\n");
    fprintf(output, "    syscall\n");
    
    fclose(output);
}
