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
%token INT /* Keywords have no semantic value */
%token FLOAT /* Keywords have no semantic value */
%token PRINT /* Keywords have no semantic value */

/* NON-TERMINAL TYPES - Define what type each grammar rule returns */
%type <node> program stmt_list stmt decl assign expr print_stmt

/* OPERATOR PRECEDENCE AND ASSOCIATIVITY */
%left '+' '-'
%left '*' '/'

%%

/* GRAMMAR RULES - Define the structure of our language */

/* PROGRAM RULE - Entry point of our grammar */
program:
    stmt_list { 
        /* Action: Save the statement list as our AST root */
        root = $1;  /* $1 refers to the first symbol (stmt_list) */
    }
    | stmt_list error          { root = $1; yyerrok; }
    ;

/* STATEMENT LIST - Handles multiple statements */
stmt_list:
    stmt { 
        /* Base case: single statement */
        $$ = $1;  /* Pass the statement up as-is */
    }
    | stmt_list stmt { 
        /* Recursive case: list followed by another statement */
        $$ = createStmtList($1, $2);  /* Build linked list of statements */
    }
    ;

/* STATEMENT TYPES - The three kinds of statements we support */
stmt:
    decl        /* Variable declaration */
    | assign    /* Assignment statement */
    | print_stmt /* Print statement */
    | error ';' { yyerrok; }
    | error { yyerrok; }
    ;

/* DECLARATION RULE - "int x;" */
decl:
    INT ID ';' { 
        /* Add variable to symbol table before freeing the string */
        addVar($2, TYPE_INT);                    /* Add the INT variable to symbol table */
        /* Create declaration node and free the identifier string */
        $$ = createDecl($2, TYPE_INT);  /* $2 is the ID token's string value */
        free($2);  
        printSymTab();          /* Print symbol table for verification */
    }
    | FLOAT ID ';' { 
        /* Add variable to symbol table before freeing the string */
        addVar($2, TYPE_FLOAT); /* Add the FLOAT variable to symbol table */
        /* Create declaration node and free the identifier string */
        $$ = createDecl($2, TYPE_FLOAT); /* $2 is the ID token's string value */
        free($2);                       /* Free the string copy from scanner */
        printSymTab();          /* Print symbol table for verification */
    }
    ;

/* ASSIGNMENT RULE - "x = expr;" */
assign:
    ID '=' expr ';' { 
        /* Create assignment node with variable name and expression */
        $$ = createAssign($1, $3);  /* $1 = ID, $3 = expr */
        free($1);                   /* Free the identifier string */
    }
    ;

/* EXPRESSION RULES - Build expression trees */
expr:
    NUM { 
        /* Literal number */
        $$ = createNum($1);  /* Create leaf node with number value */
    }
    | FLT { 
        $$ = createFlt($1);  // Add float literal handling
    }
    | ID { 
        /* Variable reference */
        $$ = createVar($1);  /* Create leaf node with variable name */
        free($1);            /* Free the identifier string */
    }
    | expr '+' expr { 
        /* Addition operation - builds binary tree */
        $$ = createBinOp('+', $1, $3);  /* Left child, op, right child */
    }
    | expr '-' expr { 
        /* Subtraction operation - builds binary tree */
        $$ = createBinOp('-', $1, $3);  /* Left child, op, right child */
    }
    | expr '*' expr { 
        /* Multiplication operation - builds binary tree */
        $$ = createBinOp('*', $1, $3);  /* Left child, op, right child */
    }
    | expr '/' expr { 
        /* Division operation - builds binary tree */
        $$ = createBinOp('/', $1, $3);  /* Left child, op, right child */
    }
    | '(' expr ')' {
        $$ = $2;
    }
    ;
/* PRINT STATEMENT - "print(expr);" */
print_stmt:
    PRINT '(' expr ')' ';' { 
        /* Create print node with expression to print */
        $$ = createPrint($3);  /* $3 is the expression inside parens */
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
