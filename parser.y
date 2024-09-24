%code requires{
    #include "token.hpp"
    #include "node.hpp"
}

%{
    #include "lexer.hpp"
    void yyerror(const char *msg);
%}

%union{
  token::Token *token;
  node::Node *node;
}

%define parse.error verbose
%locations

%start program

// Tokens
%token <token>
ID NUMCONST CHARCONST STRINGCONST BOOLCONST
INT CHAR BOOL STATIC
IF THEN ELSE 
FOR TO BY DO WHILE BREAK
AND OR EQL NEQ LESS LEQ GREATER GEQ
ASGN ADDASGN SUBASGN MULASGN DIVASGN
ADD SUB MUL DIV MOD 
DEC INC NOT QUES 
RETURN

// Non-terminals
%type <node> 
program declarationList declaration variableDeclaration scopedVariableDeclaration 
variableDeclarationList variableDeclarationInit variableDeclarationId typeSpecifier 
functionDeclaration parameters parameterList parameterTypeList parameterIdList parameterId 
statement expressionStatement compoundStatement localDeclarations statementList selectStatement 
ifStatement iterationStatement iterationRange returnStatement breakStatement expression assignmentOperator 
simpleExpression andExpression unaryRelationalExpression relationalExpression relationalOperator sumExpression 
sumOperator mulExpression mulOperator unaryExpression unaryOperator factor mutable immutable call arguments 
argumentList constant

%% 

program                   : declarationList { node::root = $1; }
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

functionDeclaration       : typeSpecifier ID '(' parameters ')' compoundStatement
                          | ID '(' parameters ')' compoundStatement
                          ;

parameters                : parameterList
                          | %empty  { $$ = nullptr; }
                          ;

parameterList             : parameterList ';' parameterTypeList
                          | parameterTypeList
                          ;

parameterTypeList         : typeSpecifier parameterIdList
                          ;

parameterIdList           : parameterIdList ',' parameterId
                          | parameterId
                          ;

parameterId               : ID
                          | ID '[' ']'
                          ;

statement                 : expressionStatement
                          | compoundStatement
                          | selectStatement
                          | iterationStatement
                          | returnStatement
                          | breakStatement
                          ;

expressionStatement       : expression ';'
                          | ';'
                          ;

compoundStatement         : '{' localDeclarations statementList '}'

localDeclarations         : localDeclarations scopedVariableDeclaration
                          | %empty  { $$ = nullptr; }
                          ;

statementList             : statementList statement
                          | %empty  { $$ = nullptr; }
                          ;

selectStatement           : ifStatement ';'
                          ;

ifStatement               : IF simpleExpression THEN statement
                          | IF simpleExpression THEN statement ELSE ifStatement

iterationStatement        : WHILE simpleExpression DO statement
                          | FOR ID ASGN iterationRange DO statement

iterationRange            : simpleExpression TO simpleExpression
                          | simpleExpression TO simpleExpression BY simpleExpression

returnStatement           : RETURN ';'
                          | RETURN expression ';'

breakStatement            : BREAK ';'

expression                : mutable assignmentOperator expression
                          | mutable INC
                          | mutable DEC
                          | simpleExpression
                          ; 

assignmentOperator        : ASGN
                          | ADDASGN
                          | SUBASGN
                          | MULASGN
                          | DIVASGN
                          ;

simpleExpression          : simpleExpression OR andExpression 
                          | andExpression
                          ;

andExpression             : andExpression AND unaryRelationalExpression
                          | unaryRelationalExpression
                          ;

unaryRelationalExpression : NOT unaryRelationalExpression
                          | relationalExpression
                          ;

relationalExpression      : sumExpression relationalOperator sumExpression
                          | sumExpression
                          ;

relationalOperator        : LESS
                          | LEQ
                          | GREATER
                          | GEQ
                          | EQL
                          | NEQ
                          ;

sumExpression             : sumExpression sumOperator mulExpression
                          | mulExpression
                          ;

sumOperator               : ADD
                          | SUB
                          ;

mulExpression             : mulExpression mulOperator unaryExpression
                          | unaryExpression
                          ;

mulOperator               : MUL
                          | DIV
                          | MOD
                          ;

unaryExpression           : unaryOperator unaryExpression
                          | factor
                          ;

unaryOperator             : SUB
                          | MUL
                          | QUES
                          ; 

factor                    : mutable
                          | immutable

mutable                   : ID  { $$ = new node::Node($1, types::NodeType::ID); delete $1; }
                          | ID '[' expression ']' 
                          ;

immutable                 : '(' expression ')'  { $$ = $2; delete $2; }
                          | call                { $$ = $1; delete $1; }
                          | constant            { $$ = $1; delete $1; }
                          ;

call                      : ID '(' arguments ')' {
                              $$ = new node::Node($1, types::NodeType::CALL);
                              if ( $3 != nullptr ) {
                                $$->setChild($3);
                              }
                          }
                          ;

arguments                 : argumentList  { $$ = $1; delete $1; }
                          | %empty        { $$ = nullptr;}
                          ;

argumentList              : argumentList ',' expression {
                              if ( $1 != nullptr ) {
                                $1->setSibling($3);
                                $$ = $1;
                                delete $1; delete $3;
                              }
                              else {
                                $$ = $3;
                                delete $1; delete $3;
                              }
                          }
                          | expression  { $$ = $1; delete $1; }
                          ;

constant                  : NUMCONST    { $$ = new node::Node($1, types::NodeType::CONSTANT); delete $1; }   
                          | CHARCONST   { $$ = new node::Node($1, types::NodeType::CONSTANT); delete $1; }  
                          | STRINGCONST { $$ = new node::Node($1, types::NodeType::CONSTANT); delete $1; }  
                          | BOOLCONST   { $$ = new node::Node($1, types::NodeType::CONSTANT); delete $1; }  
                          ;                    
%%
