/* SEMANTIC ANALYZER IMPLEMENTATION
 * Checks the AST for semantic errors before code generation
 * This ensures the program makes sense semantically, even if syntactically correct
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "symtab.h"

/* Track number of semantic errors found */
int semanticErrors = 0;

/* Initialize semantic analyzer */
void initSemantic() {
    semanticErrors = 0;
    printf("SEMANTIC ANALYZER: Initialized\n\n");
}

/* Report a semantic error */
void reportSemanticError(const char* msg) {
    printf("✗ SEMANTIC ERROR: %s\n", msg);
    semanticErrors++;
}

/* Get the type of an expression for type checking */
VarType getExprType(ASTNode* node) {
    if (!node) return TYPE_INT;
    
    switch(node->type) {
        case NODE_NUM:
            return TYPE_INT;
        case NODE_FLT:
            return TYPE_FLOAT;
        case NODE_VAR:
            if (isVarDeclared(node->data.var.name)) {
                return getVarType(node->data.var.name);
            } else {
                return TYPE_INT; // Default for undeclared variables
            }
        case NODE_BINOP: {
            VarType leftType = getExprType(node->data.binop.left);
            VarType rightType = getExprType(node->data.binop.right);
            
            // If either operand is float, result is float
            if (leftType == TYPE_FLOAT || rightType == TYPE_FLOAT) {
                return TYPE_FLOAT;
            }
            return TYPE_INT;
        }
        default:
            return TYPE_INT;
    }
}

/* Analyze an expression for semantic correctness */
void analyzeExpr(ASTNode* node) {
    if (!node) return;

    switch(node->type) {
        case NODE_NUM:
            /* Numbers are always valid */
            printf("  ✓ Integer literal: %d\n", node->data.num);
            break;

        case NODE_FLT:
            /* Float literals are always valid */
            printf("  ✓ Float literal: %f\n", node->data.flt);
            break;

        case NODE_VAR: {
            /* Check if variable has been declared */
            if (!isVarDeclared(node->data.var.name)) {
                char errorMsg[256];
                sprintf(errorMsg, "Variable '%s' used before declaration", node->data.var.name);
                reportSemanticError(errorMsg);
            } else {
                VarType varType = getVarType(node->data.var.name);
                printf("  ✓ Variable '%s' (%s)\n", node->data.var.name, 
                       varType == TYPE_FLOAT ? "float" : "int");
            }
            break;
        }

        case NODE_BINOP: {
            /* Analyze both operands first */
            analyzeExpr(node->data.binop.left);
            analyzeExpr(node->data.binop.right);
            
            /* Check for type compatibility */
            VarType leftType = getExprType(node->data.binop.left);
            VarType rightType = getExprType(node->data.binop.right);
            VarType resultType = getExprType(node);
            
            printf("  ✓ Binary operation '%c': %s %c %s = %s\n", 
                   node->data.binop.op,
                   leftType == TYPE_FLOAT ? "float" : "int",
                   node->data.binop.op,
                   rightType == TYPE_FLOAT ? "float" : "int",
                   resultType == TYPE_FLOAT ? "float" : "int");
            break;
        }

        default:
            break;
    }
}

/* Analyze a statement for semantic correctness */
void analyzeStmt(ASTNode* node) {
    if (!node) return;

    switch(node->type) {
        case NODE_DECL: {
            /* Check if variable is already declared */
            if (isVarDeclared(node->data.var.name)) {
                char errorMsg[256];
                sprintf(errorMsg, "Variable '%s' already declared", node->data.var.name);
                reportSemanticError(errorMsg);
            } else {
                /* Add variable to symbol table */
                int offset = addVar(node->data.var.name, node->data.var.type);
                if (offset != -1) {
                    printf("  ✓ Variable '%s' declared as %s\n", node->data.var.name,
                           node->data.var.type == TYPE_FLOAT ? "float" : "int");
                }
            }
            break;
        }

        case NODE_ASSIGN: {
            /* Check if variable being assigned to has been declared */
            if (!isVarDeclared(node->data.assign.var)) {
                char errorMsg[256];
                sprintf(errorMsg, "Cannot assign to undeclared variable '%s'", node->data.assign.var);
                reportSemanticError(errorMsg);
            } else {
                /* Get types for type checking */
                VarType varType = getVarType(node->data.assign.var);
                VarType exprType = getExprType(node->data.assign.value);
                
                printf("  ✓ Assignment: %s (%s) = expression (%s)\n", 
                       node->data.assign.var,
                       varType == TYPE_FLOAT ? "float" : "int",
                       exprType == TYPE_FLOAT ? "float" : "int");
                
                /* Check for type compatibility with warnings */
                if (varType != exprType) {
                    char errorMsg[256];
                    if (varType == TYPE_FLOAT && exprType == TYPE_INT) {
                        sprintf(errorMsg, "Type warning: assigning int to float variable '%s' (implicit conversion)", 
                                node->data.assign.var);
                        printf("  ⚠ %s\n", errorMsg); // Warning, not error
                    } else if (varType == TYPE_INT && exprType == TYPE_FLOAT) {
                        sprintf(errorMsg, "Type warning: assigning float to int variable '%s' (truncation)", 
                                node->data.assign.var);
                        printf("  ⚠ %s\n", errorMsg); // Warning, not error
                    }
                } else {
                    printf("  ✓ Type compatible assignment\n");
                }
            }
            /* Check the expression being assigned */
            analyzeExpr(node->data.assign.value);
            break;
        }

        case NODE_PRINT: {
            /* Check the expression being printed */
            printf("  Checking print statement expression:\n");
            VarType exprType = getExprType(node->data.expr);
            printf("  ✓ Print expression type: %s\n", 
                   exprType == TYPE_FLOAT ? "float" : "int");
            analyzeExpr(node->data.expr);
            break;
        }

        case NODE_STMT_LIST:
            /* Recursively analyze all statements */
            analyzeStmt(node->data.stmtlist.stmt);
            analyzeStmt(node->data.stmtlist.next);
            break;

        default:
            break;
    }
}

/* Analyze the entire program */
int analyzeProgram(ASTNode* root) {
    printf("Starting semantic analysis...\n");
    printf("───────────────────────────────\n");

    /* Initialize symbol table for semantic checking */
    initSymTab();

    /* Analyze all statements */
    analyzeStmt(root);

    printf("───────────────────────────────\n");

    /* Report results */
    if (semanticErrors == 0) {
        printf("✓ Semantic analysis passed - no errors found!\n");
        return 1;  /* Success */
    } else {
        printf("✗ Semantic analysis failed with %d error(s)\n", semanticErrors);
        return 0;  /* Failure */
    }
}
