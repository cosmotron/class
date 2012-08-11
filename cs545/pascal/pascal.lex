%{
#include <stdio.h>
#include "tree.h"
#include "type.h"
#include "pascal.tab.h"

 //int line_count = 1;
%}

letter              [a-zA-Z]
digit               [0-9]
id                  {letter}({letter}|{digit})*

digits              {digit}{digit}*
fraction            ("."{digits})
 /* optional_exponent   (E("+"|"-")?{digits})? */
rnum                {digits}{fraction}
inum                {digits}

addop               ("+"|"-"|"or")
mulop               ("*"|"/"|"div"|"mod"|"and")
relop               ("="|"<>"|"<"|"<="|">"|">=")

whitespace          [ \t]

%%

"array"             { /*fprintf(stderr, "[ARRAY:%s]", yytext);*/ return ARRAY; }
"begin"             { /*fprintf(stderr, "[BEGIN:%s]", yytext);*/ return BBEGIN; }
"do"                { /*fprintf(stderr, "[DO:%s]", yytext);*/ return DO; }
"else"              { /*fprintf(stderr, "[ELSE:%s]", yytext);*/ return ELSE; }
"end"               { /*fprintf(stderr, "[END:%s]", yytext);*/ return END; }
"for"               { /*fprintf(stderr, "[FOR:%s]", yytext);*/ return FOR; }
"function"          { /*fprintf(stderr, "[FUNCTION:%s]", yytext);*/ return FUNCTION; }
"if"                { /*fprintf(stderr, "[IF:%s]", yytext);*/ return IF; }
"integer"           { /*fprintf(stderr, "[INTEGER:%s]", yytext);*/ return INTEGER; }
"not"               { /*fprintf(stderr, "[NOT:%s]", yytext);*/ return NOT; }
"of"                { /*fprintf(stderr, "[OF:%s]", yytext);*/ return OF; }
"procedure"         { /*fprintf(stderr, "[PROCEDURE:%s]", yytext);*/ return PROCEDURE; }
"program"           { /*fprintf(stderr, "[PROGRAM:%s]", yytext);*/ return PROGRAM; }
"real"              { /*fprintf(stderr, "[REAL:%s]", yytext);*/ return REAL; }
"then"              { /*fprintf(stderr, "[THEN:%s]", yytext);*/ return THEN; }
"to"                { /*fprintf(stderr, "[TO:%s]", yytext);*/ return TO; }
"var"               { /*fprintf(stderr, "[VAR:%s]", yytext);*/ return VAR; }
"while"             { /*fprintf(stderr, "[WHILE:%s]", yytext);*/ return WHILE; }
":="                { /*fprintf(stderr, "[ASSIGNOP:%s]", yytext);*/ return ASSIGNOP; }
{addop}             { /*fprintf(stderr, "[ADDOP:%s]", yytext);*/
                      yylval.opval = strdup(yytext); return ADDOP; }
{mulop}             { /*fprintf(stderr, "[MULOP:%s]", yytext);*/
                      yylval.opval = strdup(yytext); return MULOP; }
{relop}             { /*fprintf(stderr, "[RELOP:%s]", yytext);*/
                      yylval.opval = strdup(yytext); return RELOP; }
{id}                { /*fprintf(stderr, "[ID:%s]", yytext);*/
                      yylval.sval = strdup(yytext); return ID; }
{rnum}              { /*fprintf(stderr, "[RNUM:%s]", yytext);*/
                      yylval.rval = atof(yytext); return RNUM; }
{inum}              { /*fprintf(stderr, "[INUM:%s]", yytext);*/
                      yylval.ival = atoi(yytext); return INUM; }
"(*"                |
"{"                 { int c;
                      while (c = input()) {
                        if (c == '}' && yytext[0] == '{')
                          break;
                        else if (c == '*')
                          if ((c = input()) == ')')
                            break;
                          else
                            unput(c);
                      } }
{whitespace}        { /*fprintf(stderr, "%s", yytext);*/ }
\n                  { /*fprintf(stderr, "%s", yytext); ++line_count; */ }
.                   { /*fprintf(stderr, "[TOKEN:%s]", yytext);*/ return yytext[0]; }

%%

void yyerror(char *msg) {
  fprintf(stderr, "error [line: %d]: %s\n", yylineno, msg);
  exit(1);
}

void error(char *msg) {
  fprintf(stderr, "error [line: %d]: %s\n", yylineno, msg);
  exit(1);
}
