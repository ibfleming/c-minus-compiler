%code requires{
    #include "token.hpp"
    #include "node.hpp"
}

%{
    #include "lexer.hpp"
    #define NT types::NodeType
    #define VT types::VarType
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
RETURN '{'

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
                          {
                            $$ = $1;
                            $$->setSibling($2);
                          }
                          | declaration   { $$ = $1; }
                          ;

declaration               : variableDeclaration   { $$ = $1; }
                          | functionDeclaration   { $$ = $1; }
                          ;

variableDeclaration       : typeSpecifier variableDeclarationList ';'
                          {
                            auto *temp = $2;
                            while (temp != nullptr) {
                              temp->setVarType($1->getVarType());
                              if( temp->getSibling() != nullptr ) {
                                temp = temp->getSibling();
                              } else {
                                temp = nullptr;
                              }
                            }
                            $$ = $2;
                          }
                          ;

scopedVariableDeclaration : STATIC typeSpecifier variableDeclarationList ';'
                          {
                            auto *temp = $3;
                            while( temp != nullptr ) {
                              temp->setVarType($2->getVarType());
                              if( temp->getSibling() != nullptr ) {
                                temp = temp->getSibling();
                              } else {
                                temp = nullptr;
                              }
                            }
                            $$ = $3;
                            $$->setNodeType(NT::STATIC_VARIABLE);
                          }
                          | typeSpecifier variableDeclarationList ';'
                          {
                            auto *temp = $2;
                            while (temp != nullptr) {
                              temp->setVarType($1->getVarType());
                              if( temp->getSibling() != nullptr ) {
                                temp = temp->getSibling();
                              } else {
                                temp = nullptr;
                              }
                            }
                            $$ = $2;
                          }
                          ;

variableDeclarationList   : variableDeclarationList ',' variableDeclarationInit
                          {
                            $$ = $1;
                            $$->setSibling($3);
                          }
                          | variableDeclarationInit   { $$ = $1; }
                          ;

variableDeclarationInit   : variableDeclarationId   { $$ = $1; } 
                          | variableDeclarationId ':' simpleExpression
                          {
                            $$ = $1;
                            $$->addChild($3);
                          }
                          ;

variableDeclarationId     : ID           
                          { 
                            $$ = new node::Node($1, NT::VARIABLE);
                          }
                          | ID '[' NUMCONST ']'
                          {
                            $$ = new node::Node($1, NT::VARIABLE_ARRAY);
                          }
                          ;

typeSpecifier             : INT    { $$->setVarType(VT::INT); }
                          | CHAR   { $$->setVarType(VT::CHAR); }
                          | BOOL   { $$->setVarType(VT::BOOL); }
                          ;

functionDeclaration       : typeSpecifier ID '(' parameters ')' compoundStatement
                          {
                            $$ = new node::Node($2, NT::FUNCTION);
                            $$->setVarType($1->getVarType());
                            if ($4 != nullptr) {
                              $$->addChild($4);
                            }
                            if ($6 != nullptr) {
                              $$->addChild($6);
                            }
                          }
                          | ID '(' parameters ')' compoundStatement
                          {
                            $$ = new node::Node($1, NT::FUNCTION);
                            if ($3 != nullptr) {
                              $$->addChild($3);
                            }
                            if ($5 != nullptr) {
                              $$->addChild($5);
                            }
                          }
                          ;

parameters                : parameterList   { $$ = $1; }
                          | %empty   { $$ = nullptr; }
                          ;

parameterList             : parameterList ';' parameterTypeList
                          {
                            $$ = $1;
                            $$->setSibling($3);
                          }
                          | parameterTypeList   { $$ = $1; }
                          ;

parameterTypeList         : typeSpecifier parameterIdList
                          {
                            auto *temp = $2;
                            while (temp != nullptr) {
                              temp->setVarType($1->getVarType());
                              if( temp->getSibling() != nullptr ) {
                                temp = temp->getSibling();
                              } else {
                                temp = nullptr;
                              }
                            }
                            $$ = $2;
                          }
                          ;

parameterIdList           : parameterIdList ',' parameterId
                          {
                            $$ = $1;
                            $1->setSibling($3);
                          }
                          | parameterId   { $$ = $1; }
                          ;

parameterId               : ID           
                          { 
                            $$ = new node::Node($1, NT::PARAMETER);
                          }
                          | ID '[' ']'   
                          {  
                            $$ = new node::Node($1, NT::PARAMETER_ARRAY); 
                          }
                          ;

statement                 : expressionStatement   { $$ = $1; }
                          | compoundStatement     { $$ = $1; }
                          | selectStatement       { $$ = $1; }
                          | iterationStatement    { $$ = $1; }
                          | returnStatement       { $$ = $1; }
                          | breakStatement        { $$ = $1; }
                          ;

expressionStatement       : expression ';'        { $$ = $1; }
                          | ';'                   { $$ = nullptr; }
                          ;

compoundStatement         : '{' localDeclarations statementList '}'
                          {
                            $$ = new node::Node($1, NT::COMPOUND);
                            if ($2 != nullptr) {
                              $$->addChild($2);
                            }
                            if ($3 != nullptr) {
                              $$->addChild($3);
                            }
                          }
                          ;

localDeclarations         : localDeclarations scopedVariableDeclaration
                          {
                            if ($1 != nullptr) {
                              $1->setSibling($2);
                              $$ = $1;
                            } else {
                              $$ = $2;
                            }
                          }
                          | %empty   { $$ = nullptr; }
                          ;

statementList             : statementList statement
                          {
                            if ($1 != nullptr) {
                              $1->setSibling($2);
                              $$ = $1;
                            } else {
                              $$ = $2;
                            }
                          }
                          | %empty   { $$ = nullptr; }
                          ;

selectStatement           : ifStatement ';'   { $$ = $1; }
                          ;

ifStatement               : IF simpleExpression THEN statement
                          {
                            $$ = new node::Node($1, NT::IF);
                            $$->addChild($2);
                            $$->addChild($4);
                          }
                          | IF simpleExpression THEN statement ELSE ifStatement
                          {
                            $$ = new node::Node($1, NT::IF);
                            $$->addChild($2);
                            $$->addChild($4);
                            $$->addChild($6);
                          }
                          ;

iterationStatement        : WHILE simpleExpression DO statement
                          {
                            $$ = new node::Node($1, NT::WHILE);
                            $$->addChild($2);
                            $$->addChild($4);
                          }
                          | FOR ID ASGN iterationRange DO statement
                          {
                            $$ = new node::Node($1, NT::FOR);
                            $$->addChild(new node::Node($2, NT::VARIABLE, VT::INT));
                            $$->addChild($4);
                            $$->addChild($6);
                          }
                          ;

iterationRange            : simpleExpression TO simpleExpression 
                          {
                            $$ = new node::Node($2, NT::RANGE);
                            $$->addChild($1); $$->addChild($3);
                          }
                          | simpleExpression TO simpleExpression BY simpleExpression
                          {
                            $$ = new node::Node($2, NT::RANGE);
                            $$->addChild($1); $$->addChild($3); $$->addChild($5);
                          }
                          ;

returnStatement           : RETURN ';'   
                          { 
                            $$ = new node::Node($1, NT::RETURN); 
                          }
                          | RETURN expression ';' 
                          {
                            $$ = new node::Node($1, NT::RETURN);
                            $$->addChild($2);
                          }
                          ;

breakStatement            : BREAK ';'   { $$ = new node::Node($1, NT::BREAK); }
                          ;

expression                : mutable assignmentOperator expression
                          {
                            $2->addChild($1); $2->addChild($3);
                            $$ = $2;
                          }
                          | mutable INC        
                          { 
                            $$ = new node::Node($2, NT::ASSIGNMENT); 
                            $$->addChild($1);
                          }
                          | mutable DEC        
                          { 
                            $$ = new node::Node($2, NT::ASSIGNMENT); 
                            $$->addChild($1);
                          }
                          | simpleExpression   { $$ = $1; }
                          ; 

assignmentOperator        : ASGN        { $$ = new node::Node($1, NT::ASSIGNMENT); }
                          | ADDASGN     { $$ = new node::Node($1, NT::ASSIGNMENT); }
                          | SUBASGN     { $$ = new node::Node($1, NT::ASSIGNMENT); }
                          | MULASGN     { $$ = new node::Node($1, NT::ASSIGNMENT); }
                          | DIVASGN     { $$ = new node::Node($1, NT::ASSIGNMENT); }
                          ;

simpleExpression          : simpleExpression OR andExpression
                          {
                            $$ = new node::Node($2, NT::OR);
                            $$->addChild($1); $$->addChild($3);
                          }
                          | andExpression   { $$ = $1; }
                          ;

andExpression             : andExpression AND unaryRelationalExpression
                          {
                            $$ = new node::Node($2, NT::AND);
                            $$->addChild($1); $$->addChild($3);
                          }
                          | unaryRelationalExpression   { $$ = $1; }
                          ;

unaryRelationalExpression : NOT unaryRelationalExpression
                          {
                            $$ = new node::Node($1, NT::NOT);
                            $$->addChild($2);
                          }
                          | relationalExpression   { $$ = $1; }    
                          ;

relationalExpression      : sumExpression relationalOperator sumExpression
                          {
                            $2->addChild($1); $2->addChild($3);
                            $$ = $2;
                          }
                          | sumExpression   { $$ = $1; }
                          ;

relationalOperator        : LESS      { $$ = new node::Node($1, NT::OPERATOR); }
                          | LEQ       { $$ = new node::Node($1, NT::OPERATOR); }
                          | GREATER   { $$ = new node::Node($1, NT::OPERATOR); }
                          | GEQ       { $$ = new node::Node($1, NT::OPERATOR); }
                          | EQL       { $$ = new node::Node($1, NT::OPERATOR); }
                          | NEQ       { $$ = new node::Node($1, NT::OPERATOR); }
                          ;

sumExpression             : sumExpression sumOperator mulExpression 
                          {
                            $2->addChild($1); $2->addChild($3);
                            $$ = $2;
                          }
                          | mulExpression   { $$ = $1; }
                          ;

sumOperator               : ADD      { $$ = new node::Node($1, NT::OPERATOR); }
                          | SUB      { $$ = new node::Node($1, NT::OPERATOR); }
                          ;

mulExpression             : mulExpression mulOperator unaryExpression 
                          {
                            $2->addChild($1); $2->addChild($3);
                            $$ = $2; 
                          }
                          | unaryExpression   { $$ = $1; }
                          ;

mulOperator               : MUL         { $$ = new node::Node($1, NT::OPERATOR); }
                          | DIV         { $$ = new node::Node($1, NT::OPERATOR); }
                          | MOD         { $$ = new node::Node($1, NT::OPERATOR); }
                          ;

unaryExpression           : unaryOperator unaryExpression 
                          { 
                            $$->addChild($1); $$->addChild($2); 
                          }
                          | factor      { $$ = $1; }
                          ;

unaryOperator             : SUB         { $$ = new node::Node($1, NT::UNARY); }
                          | MUL         { $$ = new node::Node($1, NT::UNARY); }
                          | QUES        { $$ = new node::Node($1, NT::UNARY); }
                          ; 

factor                    : mutable     { $$ = $1; }
                          | immutable   { $$ = $1; }

mutable                   : ID  { $$ = new node::Node($1, NT::ID); }
                          | ID '[' expression ']' 
                          {
                            $$ = new node::Node($1, NT::ARRAY);
                            $$->addChild($3);
                          }
                          ;

immutable                 : '(' expression ')'  { $$ = $2; }
                          | call                { $$ = $1; }
                          | constant            { $$ = $1; }
                          ;

call                      : ID '(' arguments ')' 
                          {
                            $$ = new node::Node($1, NT::CALL);
                            if ( $3 != nullptr ) {
                              $$->addChild($3);
                            }
                          }
                          ;

arguments                 : argumentList  { $$ = $1; }
                          | %empty        { $$ = nullptr;}
                          ;

argumentList              : argumentList ',' expression 
                          { 
                            if ($1 != nullptr) {
                              $1->setSibling($3);
                              $$ = $1;
                            }
                            else {
                              $$ = $3;
                            }
                          }
                          | expression  { $$ = $1; }
                          ;

                          | CHARCONST   { $$ = new node::Node($1, NT::CONSTANT, VT::CHAR); }  
constant                  : NUMCONST    { $$ = new node::Node($1, NT::CONSTANT, VT::INT); }   
                          | STRINGCONST { $$ = new node::Node($1, NT::CONSTANT, VT::STRING); }  
                          | BOOLCONST   { $$ = new node::Node($1, NT::CONSTANT, VT::BOOL); }  
                          ;                    
%%