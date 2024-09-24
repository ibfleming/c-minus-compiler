%code requires{
    #include "include/token.hpp"
}

%{
    #include "lexer.hpp"
    void yyerror(const char *msg);
%}

%union{
  token::Token* token;
}

%define parse.error verbose
%locations

%start program
%type  <token> token
%token <token> ID NUMCONST CHARCONST STRINGCONST BOOLCONST
%token <token> INT CHAR BOOL STATIC
%token <token> IF THEN ELSE FOR TO BY DO WHILE BREAK
%token <token> ASGN ADDASS INC DEC GEQ LEQ NEQ
%token <token> AND OR NOT RETURN

%% 

program                   : declarationList
                          ;

declarationList           : declarationList declaration
                          | declaration
                          ;

declaration               : variableDeclaration
                          | functionDeclaration
                          ;

variableDeclaration       : typeSpecifier variableDeclarationList ';'
                          ;

scopedVariableDeclaration : STATIC typeSpecifier variableDeclarationList ';'
                          | typeSpecifier variableDeclarationList ';'
                          ;

variableDeclarationList   : variableDeclarationList ',' variableDeclarationInit 
                          | variableDeclarationInit
                          ;

variableDeclarationInit   : variableDeclarationId 
                          | variableDeclarationId ':' simpleExpression
                          ;

variableDeclarationId     : ID 
                          | ID '[' NUMCONST ']'
                          ;

typeSpecifier             : INT
                          | CHAR
                          | BOOL
                          ;

%%

/*
expression  :   token
            |   expression token
            ;

token       :   ID            { $$ = $1; $$->print(); delete $1; }
            |   NUMCONST      { $$ = $1; $$->print(); delete $1; }
            |   CHARCONST     { $$ = $1; $$->print(); delete $1; }
            |   STRINGCONST   { $$ = $1; $$->print(); delete $1; }
            |   BOOLCONST     { $$ = $1; $$->print(); delete $1; }
            // Types
            |   INT           { $$ = $1; $$->print(); delete $1; }
            |   CHAR          { $$ = $1; $$->print(); delete $1; }
            |   BOOL          { $$ = $1; $$->print(); delete $1; }
            |   STATIC        { $$ = $1; $$->print(); delete $1; }
            // Conditional Block
            |   IF            { $$ = $1; $$->print(); delete $1; }
            |   THEN          { $$ = $1; $$->print(); delete $1; }
            |   ELSE          { $$ = $1; $$->print(); delete $1; }
            // Loops
            |   FOR           { $$ = $1; $$->print(); delete $1; }
            |   TO            { $$ = $1; $$->print(); delete $1; }
            |   BY            { $$ = $1; $$->print(); delete $1; }
            |   DO            { $$ = $1; $$->print(); delete $1; }
            |   WHILE         { $$ = $1; $$->print(); delete $1; }
            |   BREAK         { $$ = $1; $$->print(); delete $1; }
            // Operators
            |   ASGN          { $$ = $1; $$->print(); delete $1; }
            |   ADDASS        { $$ = $1; $$->print(); delete $1; }
            |   INC           { $$ = $1; $$->print(); delete $1; }
            |   DEC           { $$ = $1; $$->print(); delete $1; }
            |   GEQ           { $$ = $1; $$->print(); delete $1; }
            |   LEQ           { $$ = $1; $$->print(); delete $1; }
            |   NEQ           { $$ = $1; $$->print(); delete $1; }
            // Logical Operators
            |   AND           { $$ = $1; $$->print(); delete $1; }
            |   OR            { $$ = $1; $$->print(); delete $1; }
            |   NOT           { $$ = $1; $$->print(); delete $1; }
            // Return
            |   RETURN        { $$ = $1; $$->print(); delete $1; }
            ;
*/