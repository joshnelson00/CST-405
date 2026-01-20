#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* SEMANTIC ANALYZER
 * Performs semantic checks on the AST before code generation
 * Checks for semantic errors like:
 * - Using undeclared variables
 * - Duplicate variable declarations
 * - Type mismatches between expressions and assignments
 * - Type compatibility in binary operations
 */

/* Global semantic error tracking */
extern int semanticErrors;

/* SEMANTIC ANALYSIS FUNCTIONS */
void initSemantic();                    /* Initialize semantic analyzer */
int analyzeProgram(ASTNode* root);      /* Analyze entire program, returns 1 if valid, 0 if errors */
void analyzeStmt(ASTNode* node);        /* Analyze a statement */
void analyzeExpr(ASTNode* node);        /* Analyze an expression */
VarType getExprType(ASTNode* node);   /* Get the type of an expression for type checking */
void reportSemanticError(const char* msg);  /* Report a semantic error */

#endif
