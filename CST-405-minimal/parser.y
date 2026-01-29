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
%token INT             /* Type keywords */
%token FLOAT
%token VOID
%token PRINT           /* Statement keywords */
%token RETURN

/* NON-TERMINAL TYPES - Define what type each grammar rule returns */
%type <node> program func_list func param_list param stmt_list stmt decl assign expr print_stmt return_stmt func_call arg_list

/* OPERATOR PRECEDENCE AND ASSOCIATIVITY */
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
    func { $$ = createFuncList($1, NULL); }
  | func_list func { $$ = createFuncList($1, $2); }

/* FUNCTION RULES */
func:
    INT ID '(' param_list ')' '{' stmt_list '}' { 
        $$ = createFunc($2, TYPE_INT, $4, $7);  /* Function with return type and parameters */
        addFunction($2, TYPE_INT, $$);         /* Add to global symbol table */
        free($2);
    }
    | INT ID '(' ')' '{' stmt_list '}' { 
        $$ = createFunc($2, TYPE_INT, NULL, $6);  /* Function with no parameters */
        addFunction($2, TYPE_INT, $$);            /* Add to global symbol table */
        free($2);
    }
    | FLOAT ID '(' param_list ')' '{' stmt_list '}' { 
        $$ = createFunc($2, TYPE_FLOAT, $4, $7);  /* Float function with parameters */
        addFunction($2, TYPE_FLOAT, $$);           /* Add to global symbol table */
        free($2);
    }
    | FLOAT ID '(' ')' '{' stmt_list '}' { 
        $$ = createFunc($2, TYPE_FLOAT, NULL, $6);  /* Float function with no parameters */
        addFunction($2, TYPE_FLOAT, $$);            /* Add to global symbol table */
        free($2);
    }
    | VOID ID '(' param_list ')' '{' stmt_list '}' { 
        $$ = createFunc($2, TYPE_VOID, $4, $7);  /* Void function with parameters */
        addFunction($2, TYPE_VOID, $$);           /* Add to global symbol table */
        free($2);
    }
    | VOID ID '(' ')' '{' stmt_list '}' { 
        $$ = createFunc($2, TYPE_VOID, NULL, $6);  /* Void function with no parameters */
        addFunction($2, TYPE_VOID, $$);            /* Add to global symbol table */
        free($2);
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
    ;

/* ASSIGNMENT RULE */
assign:
    ID '=' expr ';' { 
        $$ = createAssign($1, $3);  
        free($1);                   
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
    | ID { 
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
    | '(' expr ')' {
        $$ = $2;
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
        $$ = createArgList($1, $3);  /* Multiple arguments */
    }
    ;

%%

/* ERROR HANDLING - Enhanced error reporting with context */
void yyerror(const char* s) {
    fprintf(stderr, "\n❌ Syntax Error at line %d, column %d:\n", yyline, yycolumn);
    
    // Show the problematic line with pointer
    fprintf(stderr, "   %d | ", yyline);
    
    // This is a simplified approach - in a real compiler you'd track the full line
    if (strstr(s, "syntax error")) {
        fprintf(stderr, "Unexpected token or incomplete statement\n");
        fprintf(stderr, "💡 Common fixes:\n");
        fprintf(stderr, "   • Add missing semicolon ';'\n");
        fprintf(stderr, "   • Check for unmatched parentheses\n");
        fprintf(stderr, "   • Verify variable declarations\n");
    } else if (strstr(s, "unexpected")) {
        fprintf(stderr, "Unexpected token in expression\n");
        fprintf(stderr, "💡 Check operator precedence and parentheses\n");
    } else {
        fprintf(stderr, "%s\n", s);
    }
    
    fprintf(stderr, "\n");
}
