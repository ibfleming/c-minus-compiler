%{   
#include "token.hpp"
#include "parser.hpp"
#include <string>
using namespace token;
%}

%option noyywrap
%option yylineno

NUMBER    [0-9]+

%%

{NUMBER}        { yylval.token = new Token(std::string(yytext)); return NUMCONST; }
[ \t]+          ;
"\n"            ;
.               { printf("Error\n"); }

%%