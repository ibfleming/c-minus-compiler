%{
#include "types.hpp"
#include "token.hpp"
#include "parser.hpp"
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

"int"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::INT_TYPE); return INT; }
"bool"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::BOOL_TYPE); return BOOL; }
"char"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::CHAR_TYPE); return CHAR; }
"static"        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::STATIC_TYPE); return STATIC; }
"if"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::IF_CONTROL); return IF; }
"then"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::THEN_CONTROL); return THEN; }
"else"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::ELSE_CONTROL); return ELSE; }
"for"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::FOR_LOOP); return FOR; }
"to"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::TO_LOOP); return TO; }
"by"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::BY_LOOP); return BY; }
"do"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::DO_LOOP); return DO; }
"while"         { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::WHILE_LOOP); return WHILE; }
"break"         { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::BREAK_LOOP); return BREAK; }
"and"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::AND_OP); return AND; }
"or"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::OR_OP); return OR; }
"="             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::EQL_OP); return EQL; }
"><"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::NEQ_OP); return NEQ; }
"<"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::LESS_OP); return LESS; }
"<="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::LEQ_OP); return LEQ; }
">"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::GREATER_OP); return GREATER; }
">="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::GEQ_OP); return GEQ; }
":="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::ASGN_OP); return ASGN; }
"+="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::ADDASGN_OP); return ADDASGN; }
"-="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::SUBASGN_OP); return SUBASGN; }
"*="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::MULASGN_OP); return MULASGN; }
"/="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::DIVASGN_OP); return DIVASGN; }
"+"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::ADD_OP); return ADD; }
"-"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::SUB_OP); return SUB; }
"*"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::MUL_OP); return MUL; }
"/"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::DIV_OP); return DIV; }
"%"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::MOD_OP); return MOD; }
"--"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::DEC_OP); return DEC; }
"++"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::INC_OP); return INC; }
"not"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::NOT_OP); return NOT; }      
"?"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::QUES_OP); return QUES; }
"{"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::LBRACE); return CMPD_OPEN; }
"}"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::RBRACE); return CMPD_CLOSE; }
"return"        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::RETURN); return RETURN; }
[\[\],;:{}()]   { return yytext[0]; }
{BOOLEAN}       { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::BOOL_CONST); return BOOLCONST; }
{IDENTIFIER}    { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::ID_CONST); return ID; }
{NUMBER}        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::NUM_CONST); return NUMCONST; }
{CHARACTER}     { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::CHAR_CONST); return CHARCONST; }
{STRING}        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, types::TokenType::STRING_CONST); return STRINGCONST; }
[ \t]+          ; /* IGNORE WHITESPACE */
\/\/[^\n]*      ; /* IGNORE SINGLE LINE COMMENTS */
\n              ; /* NEW LINE IGNORED */
.               { printf("ERROR(%d): Invalid or misplaced input character: '%s'. Character Ignored.\n", yylineno, yytext); }

%%