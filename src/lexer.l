%{
/* lexer.l — Flex lexer for our mini-language */
#include "tac.h"
#include "parser.tab.h"
#include <cstring>
#include <cstdlib>

static char* dup_cstr(const char* s) {
    if (s == nullptr) return nullptr;

    std::size_t n = std::strlen(s) + 1;
    char* copy = static_cast<char*>(std::malloc(n));

    if (copy != nullptr) {
        std::memcpy(copy, s, n);
    }

    return copy;
}

extern int yylineno;
%}

%option noyywrap
%option yylineno
%option never-interactive

DIGIT    [0-9]
LETTER   [a-zA-Z_]
ID       {LETTER}({LETTER}|{DIGIT})*
INT      {DIGIT}+
FLOAT    {DIGIT}+\.{DIGIT}*

%%

[ \t\r]+        { /* skip whitespace */ }
\n              { /* newline — yylineno auto-incremented */ }
"//".*          { /* skip line comments */ }

"if"            { return IF; }
"else"          { return ELSE; }
"return"        { return RETURN; }
"while"         { return WHILE; }
"int"           { return TINT; }
"float"         { return TFLOAT; }

{ID}            { yylval.sval = dup_cstr(yytext); return ID; }
{INT}           { yylval.ival = atoi(yytext);   return INT_LIT; }
{FLOAT}         { yylval.fval = atof(yytext);   return FLOAT_LIT; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return STAR; }
"/"             { return SLASH; }
"="             { return ASSIGN; }
"=="            { return EQ; }
"!="            { return NEQ; }
"<"             { return LT; }
">"             { return GT; }
"<="            { return LE; }
">="            { return GE; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
";"             { return SEMI; }
","             { return COMMA; }

.               { fprintf(stderr, "Unknown char '%s' at line %d\n",
                           yytext, yylineno); }

%%