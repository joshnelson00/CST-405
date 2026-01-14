%{
/* LEXICAL ANALYZER (SCANNER)
 * This is the first phase of compilation - breaking input into tokens
 * Flex generates a scanner from these rules that reads character by character
 */
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"  /* Token definitions from Bison */

/* Values for tracking current location (for errors)*/
int yyline = 1;
int yycolumn = 1;
%}

/* Flex options to suppress warnings for unused functions */
%option nounput
%option noinput

%%

"int"           { yycolumn += yyleng; return INT; }
"float"           { yycolumn += yyleng; return FLOAT; }
"print"         { yycolumn += yyleng; return PRINT; }

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
    printf("Lexical Error at line %d, column %d: Unknown character '%c' (ASCII %d)\n", 
           yyline, yycolumn, *yytext, *yytext);
    yycolumn++; 
}

%%

/* Required by Flex - signals end of input */
int yywrap() {
    return 1;  /* 1 means no more input files */
}
