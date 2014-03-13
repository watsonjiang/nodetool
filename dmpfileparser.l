/*
 * lex grammar for myshard data dump file.
 */

%{
#include "dmpfileparser.yacc.h"
%}

%option noyywrap
%option reentrant
%option bison-bridge

DIGIT1to9 [1-9]
DIGIT [0-9]
DIGITS {DIGIT}+
INT {DIGIT}|{DIGIT1to9}{DIGITS}|-{DIGIT}|-{DIGIT1to9}{DIGITS}
FRAC [.]{DIGITS}
EXP {E}{DIGITS}
E [eE][+-]?
HEX_DIGIT [0-9a-f]
NUMBER {INT}|{INT}{FRAC}|{INT}{EXP}|{INT}{FRAC}{EXP}

%%
\n                      {return NEWLINE;}
,                       {return COMMA;}
{NUMBER}                {yylval->sval = (char*)malloc(strlen(yytext)+1);
                         strncpy(yylval->sval, yytext, strlen(yytext)+1);
                         return NUMBER;}
\"(\\.|[^\\"])*\"       {yylval->sval = (char*)malloc(strlen(yytext)+1);
                         strncpy(yylval->sval, yytext, strlen(yytext)+1);
                         return STRING;}
\\N                     {return EMPTY;}
%%
