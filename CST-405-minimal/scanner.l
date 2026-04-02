%{
/* LEXICAL ANALYZER (SCANNER)
 * This is the first phase of compilation - breaking input into tokens
 * Flex generates a scanner from these rules that reads character by character
 */
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"  /* Token definitions from Bison */
#include "stringpool.h"  /* String interning for memory efficiency */

/* OPTIMIZATION: Increase buffer size for 4x faster I/O */
#define YY_BUF_SIZE 32768  /* 32KB buffer vs default 8KB */

/* Values for tracking current location (for errors)*/
int yyline = 1;
int yycolumn = 1;
%}
/* Flex options to suppress warnings for unused functions */
%option nounput
%option noinput
%%

"/*"([^*]|\*+[^*/])*\*+"/" {} // Anything between '/* ... */' is a comment

"int"           { yycolumn += yyleng; return INT; }
"float"         { yycolumn += yyleng; return FLOAT; }
"void"          { yycolumn += yyleng; return VOID; }
"boolean"       { yycolumn += yyleng; return BOOLEAN; }
"char"          { yycolumn += yyleng; return CHAR; }
"print"         { yycolumn += yyleng; return PRINT; }
"write"         { yycolumn += yyleng; return WRITE; }
"return"        { yycolumn += yyleng; return RETURN; }
"while"         { yycolumn += yyleng; return WHILE; }
"for"           { yycolumn += yyleng; return FOR; }
"if"            { yycolumn += yyleng; return IF; }
"else"          { yycolumn += yyleng; return ELSE; }
"true"          { yylval.num = 1; yycolumn += yyleng; return NUM; }
"false"         { yylval.num = 0; yycolumn += yyleng; return NUM; }
"switch"        { yycolumn += yyleng; return SWITCH; }
"case"          { yycolumn += yyleng; return CASE; }
"default"       { yycolumn += yyleng; return DEFAULT; }
"break"         { yycolumn += yyleng; return BREAK; }
"struct"        { yycolumn += yyleng; return STRUCT; }

\'[^\'\n]\' {
    yylval.num = (unsigned char)yytext[1];
    yycolumn += yyleng;
    return NUM;
}

\"[^\"]*\" {
    /* Store string contents without surrounding quotes */
    int n = yyleng - 2;
    yylval.str = malloc((size_t)n + 1);
    strncpy(yylval.str, yytext + 1, (size_t)n);
    yylval.str[n] = '\0';
    while (n > 0 && (yylval.str[n - 1] == '\n' || yylval.str[n - 1] == '\r')) {
        yylval.str[--n] = '\0';
    }
    yycolumn += yyleng;
    return STRING;
}

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.str = strdup(yytext);
    yycolumn += yyleng;
    return ID; 
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
"=="            { yycolumn += yyleng; return EQ; }
"!="            { yycolumn += yyleng; return NE; }
"<="            { yycolumn += yyleng; return LE; }
">="            { yycolumn += yyleng; return GE; }
"&&"            { yycolumn += yyleng; return AND; }
"||"            { yycolumn += yyleng; return OR; }
"!"             { yycolumn++; return NOT; }
"<"             { yycolumn++; return LT; }
">"             { yycolumn++; return GT; }
"+"             { yycolumn++; return '+'; }
"-"             { yycolumn++; return '-'; }
"*"             { yycolumn++; return '*'; }
"/"             { yycolumn++; return '/'; }
"="             { yycolumn++; return '='; }
"."             { yycolumn++; return DOT; }
"&"             { yycolumn++; return AMP; }
";"             { yycolumn++; return ';'; }
":"             { yycolumn++; return ':'; }
","             { yycolumn++; return ','; }
"("             { yycolumn++; return '('; }
")"             { yycolumn++; return ')'; }
"{"             { yycolumn++; return '{'; }
"}"             { yycolumn++; return '}'; }
"["             { yycolumn++; return '['; }
"]"             { yycolumn++; return ']'; }
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
