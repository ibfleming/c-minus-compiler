%{
#include "types.hpp"
#include "token.hpp"
#include "parser.hpp"
using namespace token;
typedef types::TokenType TT;    // shorthand for TokenType enum
%}

%option noyywrap
%option yylineno

IDENTIFIER      [a-zA-Z][a-zA-Z0-9]* 
NUMBER          [0-9]+
CHARACTER       '(?:[^\\'\n\0]*|\\.)*' 
STRING          \"(?:[^\\"\n\0]*|\\.)*\"
BOOLEAN         "True"|"true"|"False"|"false"

%%

"int"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::INT_TYPE); return INT; }
"bool"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::BOOL_TYPE); return BOOL; }
"char"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::CHAR_TYPE); return CHAR; }
"static"        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::STATIC_TYPE); return STATIC; }
"if"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::IF_CONTROL); return IF; }
"then"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::THEN_CONTROL); return THEN; }
"else"          { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::ELSE_CONTROL); return ELSE; }
"for"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::FOR_LOOP); return FOR; }
"to"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::TO_LOOP); return TO; }
"by"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::BY_LOOP); return BY; }
"do"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::DO_LOOP); return DO; }
"while"         { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::WHILE_LOOP); return WHILE; }
"break"         { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::BREAK_LOOP); return BREAK; }
"and"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::AND_OP); return AND; }
"or"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::OR_OP); return OR; }
"="             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::EQL_OP); return EQL; }
"><"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::NEQ_OP); return NEQ; }
"<"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::LESS_OP); return LESS; }
"<="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::LEQ_OP); return LEQ; }
">"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::GREATER_OP); return GREATER; }
">="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::GEQ_OP); return GEQ; }
":="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::ASGN_OP); return ASGN; }
"+="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::ADDASGN_OP); return ADDASGN; }
"-="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::SUBASGN_OP); return SUBASGN; }
"*="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::MULASGN_OP); return MULASGN; }
"/="            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::DIVASGN_OP); return DIVASGN; }
"+"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::ADD_OP); return ADD; }
"-"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::SUB_OP); return SUB; }
"*"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::MUL_OP); return MUL; }
"/"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::DIV_OP); return DIV; }
"%"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::MOD_OP); return MOD; }
"--"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::DEC_OP); return DEC; }
"++"            { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::INC_OP); return INC; }
"not"           { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::NOT_OP); return NOT; }      
"?"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::QUES_OP); return QUES; }
"{"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::LBRACE); return CMPD_OPEN; }
"}"             { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::RBRACE); return CMPD_CLOSE; }
"return"        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::RETURN); return RETURN; }
[\[\],;:{}()]   { return yytext[0]; }
{BOOLEAN}       { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::BOOL_CONST); return BOOLCONST; }
{IDENTIFIER}    { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::ID_CONST); return ID; }
{NUMBER}        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::NUM_CONST); return NUMCONST; }
{CHARACTER}     { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::CHAR_CONST); return CHARCONST; }
{STRING}        { yylval.token = new Token(std::string(yytext), yylineno, yyleng, TT::STRING_CONST); return STRINGCONST; }
[ \t]+          ; /* IGNORE WHITESPACE */
\/\/[^\n]*      ; /* IGNORE SINGLE LINE COMMENTS */
\n              ; /* NEW LINE IGNORED */
.               { printf("ERROR(%d): Invalid or misplaced input character: '%s'. Character Ignored.\n", yylineno, yytext); }

%%