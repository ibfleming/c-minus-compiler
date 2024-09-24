%{   
#include "include/token.hpp"
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

"%"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"*"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"("             { printf("Line %d Token: %s\n", yylineno, yytext); }
")"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"="             { printf("Line %d Token: %s\n", yylineno, yytext); }
"-"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"{"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"}"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"["             { printf("Line %d Token: %s\n", yylineno, yytext); }
"]"             { printf("Line %d Token: %s\n", yylineno, yytext); }
":"             { printf("Line %d Token: %s\n", yylineno, yytext); }
";"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"<"             { printf("Line %d Token: %s\n", yylineno, yytext); }
">"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"?"             { printf("Line %d Token: %s\n", yylineno, yytext); }
","             { printf("Line %d Token: %s\n", yylineno, yytext); }
"/"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"+"             { printf("Line %d Token: %s\n", yylineno, yytext); }
"int"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::INT); return INT; }
"bool"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::BOOL); return BOOL; }
"char"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::CHAR); return CHAR; }
"static"        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::STATIC); return STATIC; }
"if"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::IF); return IF; }
"then"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::THEN); return THEN; }
"else"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::ELSE); return ELSE; }
"for"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::FOR); return FOR; }
"to"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::TO); return TO; }
"by"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::BY); return BY; }
"do"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::DO); return DO; }
"while"         { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::WHILE); return WHILE; }
"break"         { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::BREAK); return BREAK; }
":="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::ASGN); return ASGN; }
"+="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::ADDASS); return ADDASS; }
"++"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::INC); return INC; }        
"--"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::DEC); return DEC; }
">="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::GEQ); return GEQ; }
"<="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::LEQ); return LEQ; }
"><"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::NEQ); return NEQ; }
"and"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::AND); return AND; }
"or"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::OR); return OR; }
"not"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::NOT); return NOT; }
"return"        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::RETURN); return RETURN; }
{BOOLEAN}       { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::BOOLCONST); return BOOLCONST; }
{IDENTIFIER}    { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::ID); return ID; }
{NUMBER}        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::NUMCONST); return NUMCONST; }
{CHARACTER}     { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::CHARCONST); return CHARCONST; }
{STRING}        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, token::TokenType::STRINGCONST); return STRINGCONST; }
[ \t]+          ; /* IGNORE WHITESPACE */
\/\/[^\n]*      ; /* IGNORE SINGLE LINE COMMENTS */
\n              ; /* NEW LINE IGNORED */
.               { printf("ERROR(%d): Invalid or misplaced input character: '%s'. Character Ignored.\n", yylineno, yytext); }

%%