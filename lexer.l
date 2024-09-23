%{   
#include "token.hpp"
#include "parser.hpp"
#include <string>
using namespace token;
%}

%option noyywrap
%option yylineno

IDENTIFIER      [a-zA-Z][a-zA-Z0-9]* 
NUMBER          [0-9]+
CHARACTER       '(?:[^\\'\n\0]*|\\.)*' 
STRING          \"(?:[^\\"\n\0]*|\\.)*\"
BOOLEAN         "True"|"true"|"False"|"false"

%%

{BOOLEAN}       { yylval.token = new Token(std::string(yytext), yylineno, token::TokenType::BOOLCONST); return BOOLCONST; }
{IDENTIFIER}    { yylval.token = new Token(std::string(yytext), yylineno, token::TokenType::ID); return ID; }
{NUMBER}        { yylval.token = new Token(std::string(yytext), yylineno, token::TokenType::NUMCONST); return NUMCONST; }
{CHARACTER}     { yylval.token = new Token(std::string(yytext), yylineno, token::TokenType::CHARCONST); return CHARCONST; }
{STRING}        { yylval.token = new Token(std::string(yytext), yylineno, token::TokenType::STRINGCONST); return STRINGCONST; }
[ \t]+          ; /* IGNORE WHITESPACE */
\/\/[^\n]*      ; /* IGNORE SINGLE LINE COMMENTS */
\n              ; /* NEW LINE IGNORED */
.               { printf("ERROR(%d): Invalid or misplaced input character: '%s'. Character Ignored.\n", yylineno, yytext); }

%%