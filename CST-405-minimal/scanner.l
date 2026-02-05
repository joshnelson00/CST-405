%{
/* LEXICAL ANALYZER (SCANNER)
 * This is the first phase of compilation - breaking input into tokens
 * Flex generates a scanner from these rules that reads character by character
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"  /* Token definitions from Bison */
/* Values for tracking current location (for errors)*/
int yyline = 1;
int yycolumn = 1;

// Increase buffer size for better I/O performance
#define YY_BUF_SIZE 32768  // 32KB instead of default 8KB

// Keyword hash table for O(1) lookup (small set here)
struct keyword {
    const char* word;
    int token;
};

#define NUM_KEYWORDS 3
static struct keyword keywords[] = {
    {"int", INT},
    {"float", FLOAT},
    {"print", PRINT}
};

// Simple hash for our keyword set
static int keyword_hash(const char* str) {
    int len = (int)strlen(str);
    return (len + (unsigned char)str[0]) % 13;
}

static int check_keyword(const char* str) {
    (void)keyword_hash(str); // keep hash in case you expand this to a real table
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(keywords[i].word, str) == 0) {
            return keywords[i].token;
        }
    }
    return ID;  // Not a keyword, must be identifier
}
%}
/* Flex options to suppress warnings for unused functions */
%option nounput
%option noinput
%%
[a-zA-Z_][a-zA-Z0-9_]* { 
    int token = check_keyword(yytext);
    if (token == ID) {
        yylval.str = strdup(yytext);
    }
    yycolumn += yyleng;
    return token; 
}
[0-9]+\.[0-9]+ {
    yylval.num = atof(yytext);  // Use atof for floats
    yycolumn += yyleng;
    return FLT;                 // Return FLT token
}
[0-9]+ {
    yylval.num = atoi(yytext);  // Keep atoi for integers
    yycolumn += yyleng;
    return NUM; 
}
"+"             { yycolumn++; return '+'; }
"-"             { yycolumn++; return '-'; }
"*"             { yycolumn++; return '*'; }
"/"             { yycolumn++; return '/'; }
"="             { yycolumn++; return '='; }
";"             { yycolumn++; return ';'; }
"("             { yycolumn++; return '('; }
")"             { yycolumn++; return ')'; }
[ \t]+          { yycolumn += yyleng; }  /* Track spaces/tabs only */
\n              { yyline++; yycolumn = 1; }  /* New line resets column */
.               { 
    fprintf(stderr, "\n❌ Lexical Error at line %d, column %d:\n", yyline, yycolumn);
    fprintf(stderr, "   Invalid character '%c' (ASCII %d)\n", *yytext, *yytext);
    fprintf(stderr, "💡 Allowed characters: letters, digits, spaces, and symbols: + - * / = ; ( )\n");
    fprintf(stderr, "   Suggestion: Remove or replace the invalid character\n\n");
    yycolumn++; 
}
%%
/* Required by Flex - signals end of input */
int yywrap() {
    return 1;  /* 1 means no more input files */
}
