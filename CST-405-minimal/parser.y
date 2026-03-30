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
/* Function to check for infinite/dead loops in for statements */
void checkForLoop(ASTNode* condition) {
    // NULL condition means for(;;) — always infinite
    if (!condition) {
        fprintf(stderr, "\n⚠️  Warning: Infinite loop detected at line %d\n", yyline);
        fprintf(stderr, "   for(;;) has no condition — loop runs forever\n");
        fprintf(stderr, "💡 Add a condition, e.g. for(i = 0; i < n; i = i + 1)\n\n");
        return;
    }
    if (condition->type == NODE_NUM) {
        if (condition->data.num != 0) {
            fprintf(stderr, "\n⚠️  Warning: Infinite loop detected at line %d\n", yyline);
            fprintf(stderr, "   for-loop condition is always true (non-zero constant)\n");
            fprintf(stderr, "💡 Consider adding a proper loop counter or variable condition\n\n");
        } else {
            fprintf(stderr, "\n⚠️  Warning: Dead loop detected at line %d\n", yyline);
            fprintf(stderr, "   for-loop condition is always false (zero constant) — body never runs\n");
            fprintf(stderr, "💡 Loop body is unreachable code; remove it or fix the condition\n\n");
        }
    }
}
/* Warn when an if-statement condition is a compile-time constant */
void checkIfCondition(ASTNode* condition) {
    if (!condition) return;
    if (condition->type == NODE_NUM) {
        if (condition->data.num != 0) {
            fprintf(stderr, "\n⚠️  Warning: if condition always true at line %d\n", yyline);
            fprintf(stderr, "   The else-branch (if present) is dead code\n\n");
        } else {
            fprintf(stderr, "\n⚠️  Warning: if condition always false at line %d\n", yyline);
            fprintf(stderr, "   The then-body will never execute\n\n");
        }
    }
}

static int break_context_depth = 0;

static void enterBreakContext(void) {
    break_context_depth++;
}

static void exitBreakContext(void) {
    if (break_context_depth > 0) {
        break_context_depth--;
    }
}

static int isIntegralExpr(ASTNode* expr) {
    if (!expr) return 0;

    switch (expr->type) {
        case NODE_NUM:
            return 1;
        case NODE_FLT:
        case NODE_STR:
            return 0;
        case NODE_VAR:
            return getVarType(expr->data.var.name) == TYPE_INT;
        case NODE_ARRAY_ACCESS:
            return getVarType(expr->data.array_access.name) == TYPE_INT;
        case NODE_FUNC_CALL:
            return getFunctionReturnType(expr->data.func_call.name) == TYPE_INT;
        case NODE_MEMBER_ACCESS:
            return 1;
        case NODE_ADDR_OF:
            return 0;
        case NODE_BINOP:
            if (expr->data.binop.op == OP_EQ || expr->data.binop.op == OP_NE ||
                expr->data.binop.op == OP_LT || expr->data.binop.op == OP_GT ||
                expr->data.binop.op == OP_LE || expr->data.binop.op == OP_GE ||
                expr->data.binop.op == OP_AND || expr->data.binop.op == OP_OR) {
                return 1;
            }
            if (expr->data.binop.op == OP_NOT) {
                return isIntegralExpr(expr->data.binop.left);
            }
            return isIntegralExpr(expr->data.binop.left) &&
                   isIntegralExpr(expr->data.binop.right);
        default:
            return 0;
    }
}

static VarType inferExprType(ASTNode* expr) {
    if (!expr) return TYPE_VOID;

    switch (expr->type) {
        case NODE_NUM:
            return TYPE_INT;
        case NODE_FLT:
            return TYPE_FLOAT;
        case NODE_VAR:
            return getVarType(expr->data.var.name);
        case NODE_ARRAY_ACCESS:
            return getVarType(expr->data.array_access.name);
        case NODE_MEMBER_ACCESS:
            return TYPE_INT; /* Session 1 fields are int-only. */
        case NODE_ADDR_OF:
            return TYPE_STRUCT_PTR;
        case NODE_FUNC_CALL:
            return getFunctionReturnType(expr->data.func_call.name);
        case NODE_BINOP: {
            VarType lt = inferExprType(expr->data.binop.left);
            VarType rt = inferExprType(expr->data.binop.right);
            if (expr->data.binop.op == OP_EQ || expr->data.binop.op == OP_NE ||
                expr->data.binop.op == OP_LT || expr->data.binop.op == OP_GT ||
                expr->data.binop.op == OP_LE || expr->data.binop.op == OP_GE ||
                expr->data.binop.op == OP_AND || expr->data.binop.op == OP_OR ||
                expr->data.binop.op == OP_NOT) {
                return TYPE_INT;
            }
            if (lt == TYPE_STRUCT || rt == TYPE_STRUCT ||
                lt == TYPE_STRUCT_PTR || rt == TYPE_STRUCT_PTR) {
                return TYPE_STRUCT;
            }
            return (lt == TYPE_FLOAT || rt == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
        }
        default:
            return TYPE_INT;
    }
}

static StructType* getMemberBaseStruct(ASTNode* base) {
    if (!base) return NULL;
    if (base->type == NODE_VAR) {
        return getVarStructType(base->data.var.name);
    }
    return NULL;
}

static void validateSwitchCases(ASTNode* cases) {
    int seen_values[256];
    int seen_count = 0;
    int default_count = 0;

    for (ASTNode* curr = cases; curr; curr = curr->data.case_stmt.next) {
        if (curr->type != NODE_CASE) continue;

        if (curr->data.case_stmt.is_default) {
            default_count++;
            if (default_count > 1) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Multiple default clauses in one switch are not allowed\n");
                fprintf(stderr, "💡 Suggestion: Keep only one default clause per switch\n\n");
                semantic_error_count++;
            }
            continue;
        }

        for (int i = 0; i < seen_count; i++) {
            if (seen_values[i] == curr->data.case_stmt.value) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Duplicate case value '%d' in switch\n", curr->data.case_stmt.value);
                fprintf(stderr, "💡 Suggestion: Case labels in one switch must be unique\n\n");
                semantic_error_count++;
                break;
            }
        }

        if (seen_count < 256) {
            seen_values[seen_count++] = curr->data.case_stmt.value;
        }
    }

    if (default_count == 0) {
        fprintf(stderr, "\n⚠️  Warning at line %d:\n", yyline);
        fprintf(stderr, "   switch has no default clause\n");
        fprintf(stderr, "💡 Suggestion: Add default: to handle unmatched values\n\n");
    }
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
%token IF ELSE         /* Conditional keywords */
%token SWITCH CASE DEFAULT BREAK
%token STRUCT DOT AMP
%token EQ NE LT GT LE GE   /* Comparison operators */
%token AND OR NOT

/* NON-TERMINAL TYPES - Define what type each grammar rule returns */
%type <node> program struct_defs_opt struct_def field_list field_decl func_list func param_list param stmt_list stmt stmt_list_opt decl assign expr print_stmt return_stmt func_call arg_list while_stmt for_stmt for_init for_cond for_update if_stmt switch_stmt break_stmt case_clause_list_opt case_clause_list case_clause
%type <num> case_value

/* OPERATOR PRECEDENCE AND ASSOCIATIVITY */
/* Dangling-else: LOWER_THAN_ELSE < ELSE so shift always wins → else attaches to nearest if */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left OR
%left AND
%left EQ NE LT GT LE GE
%left '+' '-'
%left '*' '/'
%right NOT
%left DOT

%%

/* GRAMMAR RULES - Define the structure of our language */

/* PROGRAM RULE - Entry point of our grammar */
program:
    struct_defs_opt func_list {
        /* Action: Save the function list as our AST root */
        root = $2;
    }
    | struct_defs_opt func_list error { root = $2; yyerrok; }
    ;

struct_defs_opt:
    /* empty */ { $$ = NULL; }
    | struct_defs_opt struct_def { $$ = $1; }
    ;

struct_def:
    STRUCT ID '{' field_list '}' ';' {
        StructType st;
        memset(&st, 0, sizeof(st));
        st.name = $2;

        int fieldIndex = 0;
        for (ASTNode* f = $4; f; f = f->data.field_decl.next) {
            if (fieldIndex >= MAX_STRUCT_FIELDS) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Struct '%s' exceeds max field count (%d)\n\n", $2, MAX_STRUCT_FIELDS);
                semantic_error_count++;
                break;
            }

            int duplicate = 0;
            for (int i = 0; i < fieldIndex; i++) {
                if (strcmp(st.fields[i].name, f->data.field_decl.name) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Duplicate field '%s' in struct '%s'\n", f->data.field_decl.name, $2);
                fprintf(stderr, "💡 Suggestion: use unique field names in each struct definition\n\n");
                semantic_error_count++;
                continue;
            }

            st.fields[fieldIndex].name = f->data.field_decl.name;
            st.fields[fieldIndex].offset = fieldIndex * 4;
            fieldIndex++;
        }
        st.numFields = fieldIndex;
        st.totalSize = fieldIndex * 4;

        if (registerStruct(&st) != 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct '%s' is already defined\n", $2);
            fprintf(stderr, "💡 Suggestion: rename or remove the duplicate struct definition\n\n");
            semantic_error_count++;
        }

        $$ = createStructDef($2, $4);
        free($2);
    }
    ;

field_list:
    field_decl { $$ = $1; }
    | field_list field_decl { $$ = appendField($1, $2); }
    ;

field_decl:
    INT ID ';' {
        $$ = createFieldDecl($2);
        free($2);
    }
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
    | STRUCT ID '*' ID {
        if (!lookupStruct($2)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s' in parameter\n", $2);
            fprintf(stderr, "💡 Suggestion: define 'struct %s' before using it as a parameter type\n\n", $2);
            semantic_error_count++;
        }
        $$ = createStructParam($4, $2, 1);
        free($2);
        free($4);
    }
    | STRUCT ID ID {
        if (!lookupStruct($2)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s' in parameter\n", $2);
            fprintf(stderr, "💡 Suggestion: define 'struct %s' before using it as a parameter type\n\n", $2);
            semantic_error_count++;
        }
        $$ = createStructParam($3, $2, 0);
        free($2);
        free($3);
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

stmt_list_opt:
    /* empty */ { $$ = NULL; }
    | stmt_list  { $$ = $1; }
    ;

/* STATEMENT RULES */
stmt:
    decl        /* Variable declaration */
    | assign    /* Assignment statement */
    | print_stmt /* Print statement */
    | return_stmt /* Return statement */
    | while_stmt /* While loop statement */
    | for_stmt   /* For loop statement */
    | if_stmt    /* If / if-else statement */
    | switch_stmt /* Switch statement */
    | break_stmt /* Break statement */
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
    | STRUCT ID ID ';' {
        StructType* st = lookupStruct($2);
        if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s'\n", $2);
            fprintf(stderr, "💡 Suggestion: define it first with: struct %s { ... };\n\n", $2);
            semantic_error_count++;
        } else {
            addStructVar($3, $2);
        }

        $$ = createStructDecl($3, $2);
        free($2);
        free($3);
        printSymTab();
    }
    | STRUCT ID '*' ID ';' {
        StructType* st = lookupStruct($2);
        if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Unknown struct type '%s'\n", $2);
            fprintf(stderr, "💡 Suggestion: define it first with: struct %s { ... };\n\n", $2);
            semantic_error_count++;
        } else {
            addStructPtrVar($4, $2);
        }

        $$ = createDecl($4, TYPE_STRUCT_PTR);
        $$->data.var.struct_name = strdup($2);
        free($2);
        free($4);
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

        if (isVarDeclared($1)) {
            VarType lhs = getVarType($1);
            VarType rhs = inferExprType($3);
            if (lhs == TYPE_STRUCT || rhs == TYPE_STRUCT) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Cannot assign struct values directly\n");
                fprintf(stderr, "💡 Suggestion: assign individual fields (e.g., p.x = value)\n\n");
                semantic_error_count++;
            } else if (lhs == TYPE_STRUCT_PTR && rhs != TYPE_STRUCT_PTR) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Pointer assignment requires an address value\n");
                fprintf(stderr, "💡 Suggestion: use '&var' or another struct pointer expression\n\n");
                semantic_error_count++;
            } else if (lhs != TYPE_STRUCT_PTR && rhs == TYPE_STRUCT_PTR) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Cannot assign an address value to a non-pointer variable\n");
                fprintf(stderr, "💡 Suggestion: assign pointer values only to 'struct T*' variables\n\n");
                semantic_error_count++;
            } else if (lhs != rhs && rhs != TYPE_VOID) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Type mismatch in assignment to '%s'\n", $1);
                fprintf(stderr, "💡 Suggestion: assign a value with matching scalar type\n\n");
                semantic_error_count++;
            }
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

        if (isVarDeclared($1) && !isArrayVar($1)) {
            VarType lhs = getVarType($1);
            VarType rhs = inferExprType($6);
            if (lhs != rhs && rhs != TYPE_VOID) {
                fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
                fprintf(stderr, "   Type mismatch in array assignment for '%s[index]'\n", $1);
                fprintf(stderr, "💡 Suggestion: assign a value with matching element type\n\n");
                semantic_error_count++;
            }
        }

        $$ = createArrayAssign($1, $3, $6); /* $1 = ID, $3 = index expr, $6 = value expr */
        free($1);                           /* Free the identifier string */
    }
    | ID DOT ID '=' expr ';' {
        ASTNode* base = createVar($1);
        StructType* st = getVarStructType($1);

        if (!isVarDeclared($1)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $1);
            fprintf(stderr, "💡 Suggestion: declare it before member assignment\n\n");
            semantic_error_count++;
        } else if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not a struct\n", $1);
            fprintf(stderr, "💡 Suggestion: only struct variables support dot access\n\n");
            semantic_error_count++;
        } else if (getStructFieldOffset(st, $3) < 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct '%s' has no field named '%s'\n", st->name, $3);
            fprintf(stderr, "💡 Suggestion: use one of the defined fields in struct '%s'\n\n", st->name);
            semantic_error_count++;
        }

        if (inferExprType($5) == TYPE_STRUCT) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Cannot assign a struct value to field '%s.%s'\n", $1, $3);
            fprintf(stderr, "💡 Suggestion: fields currently store scalar values (int/float)\n\n");
            semantic_error_count++;
        }

        $$ = createMemberAssign(base, $3, $5);
        free($1);
        free($3);
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
        $$ = createVar($1);
        free($1);
    }
    | AMP ID {
        if (!isVarDeclared($2)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Variable '%s' is not declared\n", $2);
            fprintf(stderr, "💡 Suggestion: declare it before taking its address\n\n");
            semantic_error_count++;
        }
        $$ = createAddrOf(createVar($2));
        free($2);
    }
    | func_call { 
        $$ = $1;  
    }
    | expr '+' expr { 
        if (inferExprType($1) == TYPE_STRUCT || inferExprType($3) == TYPE_STRUCT ||
            inferExprType($1) == TYPE_STRUCT_PTR || inferExprType($3) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '+'\n");
            fprintf(stderr, "💡 Suggestion: use struct fields (e.g., p.x + p.y)\n\n");
            semantic_error_count++;
        }
        $$ = createBinOp('+', $1, $3);  
    }
    | expr '-' expr { 
        if (inferExprType($1) == TYPE_STRUCT || inferExprType($3) == TYPE_STRUCT ||
            inferExprType($1) == TYPE_STRUCT_PTR || inferExprType($3) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '-'\n");
            fprintf(stderr, "💡 Suggestion: use struct fields (e.g., p.x - p.y)\n\n");
            semantic_error_count++;
        }
        $$ = createBinOp('-', $1, $3);  
    }
    | expr '*' expr { 
        if (inferExprType($1) == TYPE_STRUCT || inferExprType($3) == TYPE_STRUCT ||
            inferExprType($1) == TYPE_STRUCT_PTR || inferExprType($3) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '*'\n");
            fprintf(stderr, "💡 Suggestion: multiply scalar fields instead\n\n");
            semantic_error_count++;
        }
        $$ = createBinOp('*', $1, $3);  
    }
    | expr '/' expr { 
        if (inferExprType($1) == TYPE_STRUCT || inferExprType($3) == TYPE_STRUCT ||
            inferExprType($1) == TYPE_STRUCT_PTR || inferExprType($3) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '/'\n");
            fprintf(stderr, "💡 Suggestion: divide scalar fields instead\n\n");
            semantic_error_count++;
        }
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
    | expr AND expr {
        if (inferExprType($1) == TYPE_STRUCT || inferExprType($3) == TYPE_STRUCT ||
            inferExprType($1) == TYPE_STRUCT_PTR || inferExprType($3) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '&&'\n");
            fprintf(stderr, "💡 Suggestion: compare scalar fields and combine those results\n\n");
            semantic_error_count++;
        }
        $$ = createBinOp(OP_AND, $1, $3);
    }
    | expr OR expr {
        if (inferExprType($1) == TYPE_STRUCT || inferExprType($3) == TYPE_STRUCT ||
            inferExprType($1) == TYPE_STRUCT_PTR || inferExprType($3) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '||'\n");
            fprintf(stderr, "💡 Suggestion: compare scalar fields and combine those results\n\n");
            semantic_error_count++;
        }
        $$ = createBinOp(OP_OR, $1, $3);
    }
    | NOT expr %prec NOT {
        if (inferExprType($2) == TYPE_STRUCT || inferExprType($2) == TYPE_STRUCT_PTR) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct values cannot be used with '!'\n");
            fprintf(stderr, "💡 Suggestion: negate a scalar comparison result instead\n\n");
            semantic_error_count++;
        }
        $$ = createBinOp(OP_NOT, $2, NULL);
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
    | expr DOT ID {
        StructType* st = getMemberBaseStruct($1);
        if (!st) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Dot access requires a struct variable on the left side\n");
            fprintf(stderr, "💡 Suggestion: use '<structVar>.field'\n\n");
            semantic_error_count++;
        } else if (getStructFieldOffset(st, $3) < 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   Struct '%s' has no field named '%s'\n", st->name, $3);
            fprintf(stderr, "💡 Suggestion: use a valid field defined in the struct\n\n");
            semantic_error_count++;
        }

        $$ = createMemberAccess($1, $3);
        free($3);
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
    WHILE '(' expr ')' { enterBreakContext(); } '{' stmt_list '}' {
        checkWhileLoop($3);  /* Semantic check for infinite/dead loops */
        exitBreakContext();
        $$ = createWhile($3, $7);
    }
    ;

/* FOR LOOP RULE
 * Positions: FOR '(' for_init ';' for_cond ';' for_update ')' stmt
 *             1    2    3      4    5      6    7       8    9
 */
for_stmt:
    FOR '(' for_init ';' for_cond ';' for_update ')' { enterBreakContext(); } stmt {
        checkForLoop($5);  /* Semantic check for infinite/dead for loops */
        exitBreakContext();
        $$ = createFor($3, $5, $7, $10);
    }
    ;

/* IF STATEMENT RULE
 * Two productions — no-else and with-else.
 * The no-else production carries %prec LOWER_THAN_ELSE so that when Bison
 * sees ELSE after the then-stmt it shifts (ELSE > LOWER_THAN_ELSE), binding
 * the else-clause to the innermost if.  This resolves the dangling-else
 * ambiguity in favour of C-standard behaviour.
 */
if_stmt:
    IF '(' expr ')' stmt %prec LOWER_THAN_ELSE {
        checkIfCondition($3);          /* warn on constant condition */
        $$ = createIf($3, $5, NULL);   /* if without else */
    }
    | IF '(' expr ')' stmt ELSE stmt {
        checkIfCondition($3);          /* warn on constant condition */
        $$ = createIf($3, $5, $7);     /* if with else */
    }
    ;

switch_stmt:
    SWITCH '(' expr ')' { enterBreakContext(); } '{' case_clause_list_opt '}' {
        if (!isIntegralExpr($3)) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   switch controlling expression must be integral (int)\n");
            fprintf(stderr, "💡 Suggestions:\n");
            fprintf(stderr, "   • Use an int expression in switch(...)\n");
            fprintf(stderr, "   • Convert float expressions before switching\n\n");
            semantic_error_count++;
        }

        validateSwitchCases($7);
        exitBreakContext();
        $$ = createSwitch($3, $7);
    }
    ;

case_clause_list_opt:
    /* empty */ { $$ = NULL; }
    | case_clause_list { $$ = $1; }
    ;

case_clause_list:
    case_clause { $$ = $1; }
    | case_clause_list case_clause {
        $$ = appendCase($1, $2);
    }
    ;

case_clause:
    CASE case_value ':' stmt_list_opt {
        $$ = createCase((int)$2, 0, $4);
    }
    | DEFAULT ':' stmt_list_opt {
        $$ = createCase(0, 1, $3);
    }
    ;

case_value:
    NUM      { $$ = $1; }
    | '-' NUM { $$ = -$2; }
    ;

break_stmt:
    BREAK ';' {
        if (break_context_depth == 0) {
            fprintf(stderr, "\n❌ Semantic Error at line %d:\n", yyline);
            fprintf(stderr, "   'break' is only valid inside a loop or switch\n");
            fprintf(stderr, "💡 Suggestion: Place break inside while/for/switch blocks\n\n");
            semantic_error_count++;
        }
        $$ = createBreak();
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
