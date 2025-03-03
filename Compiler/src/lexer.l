%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "parser.tab.h"
%}

%option noyywrap noinput nounput

%%

"PROGRAM"               { return PROGRAM; }
"PROCEDURE"             { return PROCEDURE; }
"IS"                    { return IS; }
"IF"                    { return IF; }
"BEGIN"                 { return BEGIN_T; }
"END"                   { return END_T; }
"THEN"                  { return THEN; }
"ELSE"                  { return ELSE; }
"ENDIF"                 { return ENDIF; }
"WHILE"                 { return WHILE; }
"DO"                    { return DO; }
"ENDWHILE"              { return ENDWHILE; }
"REPEAT"                { return REPEAT; }
"UNTIL"                 { return UNTIL; }
"FOR"                   { return FOR; }
"ENDFOR"                { return ENDFOR; }
"FROM"                  { return FROM; }
"TO"                    { return TO; }
"DOWNTO"                { return DOWNTO; }
"READ"                  { return READ; }
"WRITE"                 { return WRITE; }
"T"                     { return T; }
":="                    { return ASSIGN; }

","                     { return COMMA; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"["                     { return LBRACK; }
"]"                     { return RBRACK; }
":"                     { return COLON; }
";"                     { return SEMICOLON; }

"="                     { return EQ; }
"!="                    { return NEQ; }
"<"                     { return LT; }
">"                     { return GT; }
"<="                    { return LEQ; }
">="                    { return GEQ; }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return ASTERISK; }
"/"                     { return FWSLASH; }
"%"                     { return PERCENT; }

[0-9]+                { yylval.num = std::stoll(yytext); return NUM; }
[_a-z]+                 { yylval.str = new std::string(yytext); return PIDENTIFIER_TOKEN; }

[ \t]+                  ; 
\n                      { ++yylineno; }

"#"[^\n]*               { /* Ignore comments starting with '#' */ }

.                       ;

%%