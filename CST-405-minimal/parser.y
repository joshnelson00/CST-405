%{
/* SYNTAX ANALYZER (PARSER)
 * This is the second phase of compilation - checking grammar rules
 * Bison generates a parser that builds an Abstract Syntax Tree (AST)
 * The parser uses tokens from the scanner to verify syntax is correct
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

/* External declarations for lexer interface */
extern int yylex();      /* Get next token from scanner */
extern int yyparse();    /* Parse the entire input */
extern FILE* yyin;       /* Input file handle */
extern int yyline;       /* Current line number from scanner */
extern int yycolumn;     /* Current column number from scanner */

void yyerror(const char* s);  /* Error handling function */
ASTNode* root = NULL;          /* Root of the Abstract Syntax Tree */

/* Function to check for infinite/dead loops */
void checkWhileLoop(ASTNode* condition) {
    if (!condition) return;
    
    // Check for constant true condition (infinite loop)
    if (condition->type == NODE_NUM) {
        if (condition->data.num != 0) {
            fprintf(stderr, "\n⚠️  Warning: Infinite loop detected at line %d\n", yyline);
            fprintf(stderr, "   Loop condition is always true (non-zero constant)\n");
            fprintf(stderr, "💡 Consider adding a break condition or loop counter\n\n");
        } else {
            fprintf(stderr, "\n⚠️  Warning: Dead loop detected at line %d\n", yyline);
            fprintf(stderr, "   Loop condition is always false (zero constant)\n");
            fprintf(stderr, "💡 Loop body will never execute\n\n");
        }
    }
    // Could add more sophisticated checks here for variable analysis
}
int syntax_error_count = 0;    /* Counter for syntax errors */
extern int semantic_error_count; /* Counter for semantic errors (in symtab.c) */
%}

/* SEMANTIC VALUES UNION
 * Defines possible types for tokens and grammar symbols
 * This allows different grammar rules to return different data types
 */
%union {
    double num;                /* For integer literals */
    char* str;              /* For identifiers */
    struct ASTNode* node;   /* For AST nodes */
}

/* TOKEN DECLARATIONS with their semantic value types */
%token <num> NUM        /* Number token carries an integer value */
%token <num> FLT        /* Float token carries a float value */
%token <str> ID         /* Identifier token carries a string */
%token <str> STRING     /* String literal token */
%token INT             /* Type keywords */
%token FLOAT
%token VOID
%token PRINT           /* Statement keywords */
%token RETURN
%token WHILE FOR       /* Loop keywords */
%token EQ NE LT GT LE GE   /* Comparison operators */

/* NON-TERMINAL TYPES - Define what type each grammar rule returns */
%type <node> program func_list func param_list param stmt_list stmt decl assign expr print_stmt return_stmt func_call arg_list while_stmt for_stmt for_init for_cond for_update

/* OPERATOR PRECEDENCE AND ASSOCIATIVITY */
%left EQ NE LT GT LE GE
%left '+' '-'
%left '*' '/'

%%

/* GRAMMAR RULES - Define the structure of our language */

/* PROGRAM RULE - Entry point of our grammar */
program:
    func_list { 
        /* Action: Save the function list as our AST root */
        root = $1;  /* $1 refers to the first symbol (func_list) */
    }
    | func_list error          { root = $1; yyerrok; }
    ;

/* FUNCTION LIST RULES */
func_list:
    func { $$ = $1; }
  | func func_list { 
        $1->data.func.next = $2;
        $$ = $1;
    }

/* FUNCTION RULES */
func:
    INT ID '(' param_list ')' { prepareFunctionScope($2, TYPE_INT); addParamsToScope($4); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($2, TYPE_INT, $4, $8);  /* Function with return type and parameters */
        addFunction($2, TYPE_INT, $$);         /* Update with AST */
        free($2);
    }
    | INT ID '(' ')' { prepareFunctionScope($2, TYPE_INT); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($2, TYPE_INT, NULL, $7);  /* Function with no parameters */
        addFunction($2, TYPE_INT, $$);            /* Update with AST */
        free($2);
    }
    | FLOAT ID '(' param_list ')' { prepareFunctionScope($2, TYPE_FLOAT); addParamsToScope($4); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($2, TYPE_FLOAT, $4, $8);  /* Float function with parameters */
        addFunction($2, TYPE_FLOAT, $$);           /* Update with AST */
        free($2);
    }
    | FLOAT ID '(' ')' { prepareFunctionScope($2, TYPE_FLOAT); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($2, TYPE_FLOAT, NULL, $7);  /* Float function with no parameters */
        addFunction($2, TYPE_FLOAT, $$);            /* Update with AST */
        free($2);
    }
    | VOID ID '(' param_list ')' { prepareFunctionScope($2, TYPE_VOID); addParamsToScope($4); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($2, TYPE_VOID, $4, $8);  /* Void function with parameters */
        addFunction($2, TYPE_VOID, $$);           /* Update with AST */
        free($2);
    }
    | VOID ID '(' ')' { prepareFunctionScope($2, TYPE_VOID); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($2, TYPE_VOID, NULL, $7);  /* Void function with no parameters */
        addFunction($2, TYPE_VOID, $$);            /* Update with AST */
        free($2);
    }
    | INT '[' ']' ID '(' param_list ')' { prepareFunctionScope($4, TYPE_INT); addParamsToScope($6); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($4, TYPE_INT, $6, $10);  /* Function returning int array with parameters */
        $$->data.func.return_type = TYPE_INT;
        addFunction($4, TYPE_INT, $$);
        free($4);
    }
    | INT '[' ']' ID '(' ')' { prepareFunctionScope($4, TYPE_INT); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($4, TYPE_INT, NULL, $9);  /* Function returning int array, no parameters */
        $$->data.func.return_type = TYPE_INT;
        addFunction($4, TYPE_INT, $$);
        free($4);
    }
    | FLOAT '[' ']' ID '(' param_list ')' { prepareFunctionScope($4, TYPE_FLOAT); addParamsToScope($6); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($4, TYPE_FLOAT, $6, $10);  /* Function returning float array with parameters */
        $$->data.func.return_type = TYPE_FLOAT;
        addFunction($4, TYPE_FLOAT, $$);
        free($4);
    }
    | FLOAT '[' ']' ID '(' ')' { prepareFunctionScope($4, TYPE_FLOAT); } '{' stmt_list '}' { 
        exitFunction();
        $$ = createFunc($4, TYPE_FLOAT, NULL, $9);  /* Function returning float array, no parameters */
        $$->data.func.return_type = TYPE_FLOAT;
        addFunction($4, TYPE_FLOAT, $$);
        free($4);
    }
    | error '}' { yyerrok; }
    ;

/* PARAMETER LIST RULES */
param_list:
    param { 
        $$ = createParamList($1, NULL);  /* Single parameter */
    }
    | param_list ',' param { 
        $$ = createParamList($1, $3);  /* Multiple parameters */
    }
    ;

/* PARAMETER RULE */
param:
    INT ID { 
        $$ = createParam($2, TYPE_INT);  /* Integer parameter */
        free($2);
    }
    | FLOAT ID { 
        $$ = createParam($2, TYPE_FLOAT);  /* Float parameter */
        free($2);
    }
    | INT ID '[' ']' {
        $$ = createArrayParam($2, TYPE_INT);  /* Integer array parameter */
        free($2);
    }
    | FLOAT ID '[' ']' {
        $$ = createArrayParam($2, TYPE_FLOAT);  /* Float array parameter */
        free($2);
    }
    ;

/* STATEMENT LIST RULES */
stmt_list:
    stmt { 
        $$ = $1;  /* Single statement */
    }
    | stmt_list stmt { 
        $$ = createStmtList($1, $2);  /* Multiple statements */
    }
    ;

/* STATEMENT RULES */
stmt:
    decl        /* Variable declaration */
    | assign    /* Assignment statement */
    | print_stmt /* Print statement */
    | return_stmt /* Return statement */
    | while_stmt /* While loop statement */
    | for_stmt   /* For loop statement */
    | func_call ';' {
        /* Bare function call as statement (e.g., fillNumbers(arr);) */
        $$ = $1;
    }
    | '{' stmt_list '}' {
        /* Nested block statement */
        $$ = $2;
    }
    | error ';' { yyerrok; }
    | error { yyerrok; }
    ;

/* DECLARATION RULES */
decl:
    INT ID ';' { 
        addVar($2, TYPE_INT);                    
        $$ = createDecl($2, TYPE_INT);  
        free($2);  
        printSymTab();          
    }
    | FLOAT ID ';' { 
        addVar($2, TYPE_FLOAT); 
        $$ = createDecl($2, TYPE_FLOAT); 
        free($2);                       
        printSymTab();          
    }
    | INT ID '[' NUM ']' ';' {
        /* Array declaration with size validation */
        int size = (int)$4;
        if (size == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Array '%s' cannot have zero size\n", $2);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Array size must be at least 1\n");
            fprintf(stderr, "   • Example: int %s[10];\n\n", $2);
            semantic_error_count++;
        } else {
            addArrayVar($2, TYPE_INT, size); /* Add INT array to symbol table */
        }
        $$ = createArrayDecl($2, TYPE_INT, size); /* Create array declaration node */
        free($2);                       /* Free the identifier string */
        printSymTab();          /* Print symbol table for verification */
    }
    | FLOAT ID '[' NUM ']' ';' {
        /* Array declaration with size validation */
        int size = (int)$4;
        if (size == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Array '%s' cannot have zero size\n", $2);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Array size must be at least 1\n");
            fprintf(stderr, "   • Example: float %s[10];\n\n", $2);
            semantic_error_count++;
        } else {
            addArrayVar($2, TYPE_FLOAT, size); /* Add FLOAT array to symbol table */
        }
        $$ = createArrayDecl($2, TYPE_FLOAT, size); /* Create array declaration node */
        free($2);                       /* Free the identifier string */
        printSymTab();          /* Print symbol table for verification */
    }
    | INT ID '[' '-' NUM ']' ';' {
        /* ERROR: Negative array size */
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have negative size (-%d)\n", $2, (int)$5);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Array size must be a positive integer\n");
        fprintf(stderr, "   • Example: int %s[%d];\n\n", $2, (int)$5);
        semantic_error_count++;
        $$ = createArrayDecl($2, TYPE_INT, 1); /* Dummy node for recovery */
        free($2);
    }
    | FLOAT ID '[' '-' NUM ']' ';' {
        /* ERROR: Negative array size */
        fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
        fprintf(stderr, "   Array '%s' cannot have negative size (-%d)\n", $2, (int)$5);
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Array size must be a positive integer\n");
        fprintf(stderr, "   • Example: float %s[%d];\n\n", $2, (int)$5);
        semantic_error_count++;
        $$ = createArrayDecl($2, TYPE_FLOAT, 1); /* Dummy node for recovery */
        free($2);
    }
    ;

/* ASSIGNMENT RULE */
assign:
    ID '=' expr ';' { 
        /* Check if left-side variable is declared */
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the variable first: int %s;\n", $1);
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        } else if (isArrayVar($1)) {
            fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
            fprintf(stderr, "   '%s' is an array but assigned without subscript\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Use array indexing: %s[index] = value;\n", $1);
            fprintf(stderr, "   • Access specific element: %s[0] = value;\n\n", $1);
            semantic_error_count++;
        }
        /* Check if right side is a bare array name (array used as scalar) */
        if ($3 && $3->type == NODE_VAR && isArrayVar($3->data.var.name)) {
            fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
            fprintf(stderr, "   '%s' is an array but used without subscript [index]\n", $3->data.var.name);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Use array indexing: %s[index]\n", $3->data.var.name);
            fprintf(stderr, "   • Access a specific element: %s[0]\n\n", $3->data.var.name);
            semantic_error_count++;
        }
        $$ = createAssign($1, $3);  
        free($1);                   
    } 
    | ID '[' expr ']' '=' expr ';' {
        /* Check if variable is declared and is an array */
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the array first: int %s[size];\n", $1);
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        } else if (!isArrayVar($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not an array but used with subscript\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Assign without subscript: %s = value;\n", $1);
            fprintf(stderr, "   • Redeclare as array: int %s[size];\n\n", $1);
            semantic_error_count++;
        }
        /* Array element assignment with bounds checking */
        // Check if index is constant and in bounds
        if ($3->type == NODE_NUM) {
            int index = $3->data.num;
            if (isArrayVar($1)) {
                int size = getArraySize($1);
                if (index < 0) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is negative (out of bounds)\n", $1, index);
                    fprintf(stderr, "💡 Suggestion: Array indices must be >= 0\n\n");
                    semantic_error_count++;
                } else if (index >= size) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is out of bounds (size is %d, valid range [0..%d])\n",
                           $1, index, size, size - 1);
                    fprintf(stderr, "💡 Suggestion: Use an index between 0 and %d\n\n", size - 1);
                    semantic_error_count++;
                }
            }
        }
        $$ = createArrayAssign($1, $3, $6); /* $1 = ID, $3 = index expr, $6 = value expr */
        free($1);                           /* Free the identifier string */
    }
    ;

/* EXPRESSION RULES */
expr:
    NUM { 
        $$ = createNum($1);  
    }
    | FLT { 
        $$ = createFlt($1);  
    }
    | STRING {
        $$ = createStr($1);
        free($1);
    }
    | ID { 
        /* Check if variable is declared */
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the variable first: int %s;\n", $1);
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        /* Note: array-as-scalar check is done in the assign rule context,
         * since bare array names are valid as function arguments */
        $$ = createVar($1);  
        free($1);            
    }
    | func_call { 
        $$ = $1;  
    }
    | expr '+' expr { 
        $$ = createBinOp('+', $1, $3);  
    }
    | expr '-' expr { 
        $$ = createBinOp('-', $1, $3);  
    }
    | expr '*' expr { 
        $$ = createBinOp('*', $1, $3);  
    }
    | expr '/' expr { 
        $$ = createBinOp('/', $1, $3);  
    }
    | expr EQ expr {
        $$ = createBinOp(OP_EQ, $1, $3);
    }
    | expr NE expr {
        $$ = createBinOp(OP_NE, $1, $3);
    }
    | expr LT expr {
        $$ = createBinOp(OP_LT, $1, $3);
    }
    | expr GT expr {
        $$ = createBinOp(OP_GT, $1, $3);
    }
    | expr LE expr {
        $$ = createBinOp(OP_LE, $1, $3);
    }
    | expr GE expr {
        $$ = createBinOp(OP_GE, $1, $3);
    }
    | '(' expr ')' {
        $$ = $2;
    }
    | ID '[' expr ']' {
        /* Check if variable is declared and is an array */
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Declare the array first: int %s[size];\n", $1);
            fprintf(stderr, "   • Check for typos in the variable name\n\n");
            semantic_error_count++;
        } else if (!isArrayVar($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not an array but used with subscript\n", $1);
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Remove the subscript if '%s' is a scalar variable\n", $1);
            fprintf(stderr, "   • Redeclare as array: int %s[size];\n\n", $1);
            semantic_error_count++;
        }
        /* Array element access with bounds checking */
        // Check if index is constant and in bounds
        if ($3->type == NODE_NUM) {
            int index = $3->data.num;
            if (isArrayVar($1)) {
                int size = getArraySize($1);
                if (index < 0) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is negative (out of bounds)\n", $1, index);
                    fprintf(stderr, "💡 Suggestion: Array indices must be >= 0\n\n");
                    semantic_error_count++;
                } else if (index >= size) {
                    fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
                    fprintf(stderr, "   Array '%s' index %d is out of bounds (size is %d, valid range [0..%d])\n",
                           $1, index, size, size - 1);
                    fprintf(stderr, "💡 Suggestion: Use an index between 0 and %d\n\n", size - 1);
                    semantic_error_count++;
                }
            }
        }
        $$ = createArrayAccess($1, $3);  /* $1 = ID, $3 = index expression */
        free($1);                        /* Free the identifier string */
    }
    ;

/* PRINT STATEMENT */
print_stmt:
    PRINT '(' expr ')' ';' { 
        $$ = createPrint($3);  
    }
    ;

/* RETURN STATEMENT */
return_stmt:
    RETURN expr ';' { 
        $$ = createReturn($2);  
    }
    | RETURN ';' { 
        $$ = createReturn(NULL);  
    }
    ;

/* FUNCTION CALL RULES */
func_call:
    ID '(' arg_list ')' { 
        $$ = createFuncCall($1, $3);  
        free($1);
    }
    | ID '(' ')' { 
        $$ = createFuncCall($1, NULL);  
        free($1);
    }
    ;

/* ARGUMENT LIST RULES */
arg_list:
    expr { 
        $$ = createArgList($1, NULL);  /* Single argument */
    }
    | arg_list ',' expr { 
        $$ = createArgList($3, $1);  /* Multiple arguments - new arg with link to previous list */
    }
    ;

/* WHILE LOOP RULE */
while_stmt:
    WHILE '(' expr ')' '{' stmt_list '}' {
        checkWhileLoop($3);  /* Semantic check for infinite/dead loops */
        $$ = createWhile($3, $6);
    }
    ;

/* FOR LOOP RULE
 * Positions: FOR '(' for_init ';' for_cond ';' for_update ')' stmt
 *             1    2    3      4    5      6    7       8    9
 */
for_stmt:
    FOR '(' for_init ';' for_cond ';' for_update ')' stmt {
        $$ = createFor($3, $5, $7, $9);
    }
    ;

/* FOR INIT - optional scalar or array assignment without trailing semicolon */
for_init:
    /* empty */ { $$ = NULL; }
    | INT ID '=' expr {
        /* Inline declaration: for (int i = 0; ...) */
        addVar($2, TYPE_INT);
        printSymTab();
        $$ = createStmtList(createDecl($2, TYPE_INT), createAssign($2, $4));
        free($2);
    }
    | FLOAT ID '=' expr {
        /* Inline declaration: for (float x = 0.0; ...) */
        addVar($2, TYPE_FLOAT);
        printSymTab();
        $$ = createStmtList(createDecl($2, TYPE_FLOAT), createAssign($2, $4));
        free($2);
    }
    | ID '=' expr {
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "\U0001f4a1 Suggestions:\n");
            fprintf(stderr, "   \u2022 Declare the variable first: int %s;\n", $1);
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        $$ = createAssign($1, $3);
        free($1);
    }
    | ID '[' expr ']' '=' expr {
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        $$ = createArrayAssign($1, $3, $6);
        free($1);
    }
    ;

/* FOR COND - optional condition expression (NULL signals always-true) */
for_cond:
    /* empty */ { $$ = NULL; }
    | expr      { $$ = $1; }
    ;

/* FOR UPDATE - optional scalar or array assignment without trailing semicolon */
for_update:
    /* empty */ { $$ = NULL; }
    | ID '=' expr {
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "\U0001f4a1 Suggestions:\n");
            fprintf(stderr, "   \u2022 Declare the variable first: int %s;\n", $1);
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        $$ = createAssign($1, $3);
        free($1);
    }
    | ID '[' expr ']' '=' expr {
        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n\u274c Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "   \u2022 Check for typos in the variable name\n\n");
            semantic_error_count++;
        }
        $$ = createArrayAssign($1, $3, $6);
        free($1);
    }
    ;

%%

/* Maximum number of displayed errors before the compiler gives up */
#define MAX_ERRORS 30

/* Track internal yyerror calls to detect infinite loops */
static int yyerror_total_calls = 0;
#define MAX_YYERROR_CALLS 200

/* ERROR HANDLING - Enhanced error reporting with context */
void yyerror(const char* s) {
    /* Detect infinite error recovery loops */
    yyerror_total_calls++;
    if (yyerror_total_calls > MAX_YYERROR_CALLS) {
        int total = syntax_error_count + semantic_error_count;
        fprintf(stderr, "\n🛑 Too many cascading errors. Stopping compilation.\n\n");
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
        printf("║  Total errors:    %3d                                       ║\n", total);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  Review the detailed error messages above (on stderr).      ║\n");
        printf("║  Each error includes 💡 suggestions for how to fix it.      ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        exit(1);
    }
    
    /* Suppress duplicate errors on the same line to reduce noise */
    static int last_error_line = -1;
    if (yyline == last_error_line) {
        /* Already reported an error on this line - suppress to avoid spam */
        return;
    }
    last_error_line = yyline;
    
    /* Only count errors that are actually displayed */
    syntax_error_count++;
    
    /* Stop after too many displayed errors */
    if (syntax_error_count > MAX_ERRORS) {
        int total = syntax_error_count + semantic_error_count;
        fprintf(stderr, "\n🛑 Too many errors (%d+). Stopping compilation.\n\n", MAX_ERRORS);
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
        printf("║  Total errors:    %3d                                       ║\n", total);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  Review the detailed error messages above (on stderr).      ║\n");
        printf("║  Each error includes 💡 suggestions for how to fix it.      ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        exit(1);
    }
    
    fprintf(stderr, "\n❌ Syntax Error at line %d, column %d:\n", yyline, yycolumn);
    
    // Show the problematic line with pointer
    fprintf(stderr, "   %d | ", yyline);
    
    // Provide context-specific suggestions based on the error message
    if (strstr(s, "syntax error")) {
        fprintf(stderr, "Unexpected token or incomplete statement\n");
        fprintf(stderr, "💡 Common fixes:\n");
        fprintf(stderr, "   • Add missing semicolon ';' at the end of the previous statement\n");
        fprintf(stderr, "   • Check for unmatched braces '{' or '}'\n");
        fprintf(stderr, "   • Check for unmatched parentheses '(' or ')'\n");
        fprintf(stderr, "   • Verify variable/function declarations are correct\n");
        fprintf(stderr, "   • Remove any stray characters or typos\n");
    } else if (strstr(s, "unexpected")) {
        fprintf(stderr, "Unexpected token in expression\n");
        fprintf(stderr, "💡 Suggestions:\n");
        fprintf(stderr, "   • Check operator precedence and parentheses\n");
        fprintf(stderr, "   • Ensure expressions are complete (e.g., a + b, not a +)\n");
    } else {
        fprintf(stderr, "%s\n", s);
    }
    
    fprintf(stderr, "\n");
}
