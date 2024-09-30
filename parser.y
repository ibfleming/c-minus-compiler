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
CMPD_OPEN CMPD_CLOSE
RETURN '(' '['

// Non-terminals
%type <node> 
program declarationList declaration variableDeclaration scopedVariableDeclaration 
variableDeclarationList variableDeclarationInit variableDeclarationId typeSpecifier 
functionDeclaration parameters parameterList parameterTypeList parameterIdList 
parameterId statement matchedStatements unmatchedStatements expressionStatement 
compoundStatement localDeclarations statementList matchedIfStatement unmatchedIfStatement 
unmatchedIterationStatement matchedIterationStatement iterationRange returnStatement 
breakStatement expression assignmentOperator simpleExpression andExpression 
unaryRelationalExpression relationalExpression relationalOperator sumExpression sumOperator 
mulExpression mulOperator unaryExpression unaryOperator factor mutable immutable call 
arguments argumentList constant

%% 

program                   : declarationList { node::root = $1; }
                          ;

declarationList           : declarationList declaration
                          {
                            $1->setSibling($2);
                            $$ = $1;
                          }
                          | declaration
                          ;

declaration               : variableDeclaration
                          | functionDeclaration
                          ;

variableDeclaration       : typeSpecifier variableDeclarationList ';'
                          {
                            auto *temp = $2;
                            while (temp != nullptr) {
                              temp->setVarType($1->getVarType());
                              temp = temp->getSibling();
                            }
                            $$ = $2;
                          }
                          ;

scopedVariableDeclaration : STATIC typeSpecifier variableDeclarationList ';'
                          {
                            auto *temp = $3;
                            while (temp != nullptr) {
                              temp->setVarType($2->getVarType());
                                temp = temp->getSibling();
                            }
                            $$ = $3;
                            $$->setNodeType(NT::STATIC_VARIABLE);
                          }
                          | typeSpecifier variableDeclarationList ';'
                          {
                            auto *temp = $2;
                            while (temp != nullptr) {
                              temp->setVarType($1->getVarType());
                              temp = temp->getSibling();
                            }
                            $$ = $2;
                          }
                          ;

variableDeclarationList   : variableDeclarationList ',' variableDeclarationInit
                          {
                            $1->setSibling($3);
                            $$ = $1;
                          }
                          | variableDeclarationInit
                          ;

variableDeclarationInit   : variableDeclarationId
                          | variableDeclarationId ':' simpleExpression
                          {
                            $1->addChild($3);
                            $$ = $1;
                          }
                          ;

variableDeclarationId     : ID           
                          { 
                            $$ = new node::Node($1, NT::VARIABLE);
                          }
                          | ID '[' NUMCONST ']'
                          {
                            $$ = new node::Node($2, NT::VARIABLE_ARRAY);
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
                              $$->addChild($6, 1);
                            }
                          }
                          | ID '(' parameters ')' compoundStatement
                          {
                            $$ = new node::Node($1, NT::FUNCTION);
                            $$->setVarType(VT::VOID);
                            if ($3 != nullptr) {
                              $$->addChild($3);
                            }
                            if ($5 != nullptr) {
                              $$->addChild($5, 1);
                            }
                          }
                          ;

parameters                : parameterList
                          | %empty          { $$ = nullptr; }
                          ;

parameterList             : parameterList ';' parameterTypeList
                          {
                            $1->setSibling($3);
                            $$ = $1;
                          }
                          | parameterTypeList
                          ;

parameterTypeList         : typeSpecifier parameterIdList
                          {
                            auto *temp = $2;
                            while (temp != nullptr) {
                              temp->setVarType($1->getVarType());
                              temp = temp->getSibling();
                            }
                            $$ = $2;
                          }
                          ;

parameterIdList           : parameterIdList ',' parameterId
                          {
                            $1->setSibling($3);
                            $$ = $1;
                          }
                          | parameterId
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

statement                 : matchedStatements
                          | unmatchedStatements       
                          ;

matchedStatements         : expressionStatement
                          | compoundStatement
                          | matchedIfStatement
                          | matchedIterationStatement     
                          | returnStatement    
                          | breakStatement     
                          ;

unmatchedStatements       : unmatchedIfStatement        
                          | unmatchedIterationStatement 
                          ;

expressionStatement       : expression ';'        
                          | ';'                   { $$ = nullptr; }
                          ;

compoundStatement         : CMPD_OPEN localDeclarations statementList CMPD_CLOSE
                          {
                            $$ = new node::Node($1, NT::COMPOUND);
                            if ($2 != nullptr) {
                              $$->addChild($2);
                            }
                            if ($3 != nullptr) {
                              $$->addChild($3, 1);
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

matchedIfStatement        : IF simpleExpression THEN matchedStatements ELSE matchedStatements
                          {
                            $$ = new node::Node($1, NT::IF);
                            $$->addChild($2);
                            if( $4 != nullptr) {
                              $$->addChild($4, 1);
                            }
                            if ($6 != nullptr) {
                              $$->addChild($6, 2);
                            }
                          }
                          ;

unmatchedIfStatement      : IF simpleExpression THEN statement
                          {
                            $$ = new node::Node($1, NT::IF);
                            $$->addChild($2);
                            if( $4 != nullptr) {
                              $$->addChild($4, 1);
                            }
                          }
                          | IF simpleExpression THEN matchedStatements ELSE unmatchedStatements
                          {
                            $$ = new node::Node($1, NT::IF);
                            $$->addChild($2);
                            if( $4 != nullptr) {
                              $$->addChild($4, 1);
                            }
                            if ($6 != nullptr) {
                              $$->addChild($6, 2);
                            }
                          }
                          ;

matchedIterationStatement : WHILE simpleExpression DO matchedStatements
                          {
                            $$ = new node::Node($1, NT::WHILE);
                            $$->addChild($2);
                            if ($4 != nullptr) {
                              $$->addChild($4, 1);
                            }
                          }
                          | FOR ID ASGN iterationRange DO matchedStatements
                          {
                            $$ = new node::Node($1, NT::FOR);
                            $$->addChild(new node::Node($2, NT::VARIABLE, VT::INT));
                            $$->addChild($4, 1);
                            if ($6 != nullptr) {
                              $$->addChild($6, 2);
                            }
                          }

unmatchedIterationStatement : WHILE simpleExpression DO unmatchedStatements
                            {
                              $$ = new node::Node($1, NT::WHILE);
                              $$->addChild($2);
                              if ($4 != nullptr) {
                                $$->addChild($4, 1);
                              }
                            }
                            | FOR ID ASGN iterationRange DO unmatchedStatements
                            {
                              $$ = new node::Node($1, NT::FOR);
                              $$->addChild(new node::Node($2, NT::VARIABLE, VT::INT));
                              $$->addChild($4, 1);
                              if ($6 != nullptr) {
                                $$->addChild($6, 2);
                              }
                            }
                            ;

iterationRange            : simpleExpression TO simpleExpression 
                          {
                            $$ = new node::Node($2, NT::RANGE);
                            $$->addChild($1);
                            $$->addChild($3, 1);
                          }
                          | simpleExpression TO simpleExpression BY simpleExpression
                          {
                            $$ = new node::Node($2, NT::RANGE);
                            $$->addChild($1); 
                            $$->addChild($3, 1); 
                            $$->addChild($5, 2);
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
                            $2->addChild($1);
                            $2->addChild($3, 1);
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
                          | simpleExpression   
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
                            $$->addChild($1);
                            $$->addChild($3, 1);
                          }
                          | andExpression   
                          ;

andExpression             : andExpression AND unaryRelationalExpression
                          {
                            $$ = new node::Node($2, NT::AND);
                            $$->addChild($1);
                            $$->addChild($3, 1);
                          }
                          | unaryRelationalExpression   
                          ;

unaryRelationalExpression : NOT unaryRelationalExpression
                          {
                            $$ = new node::Node($1, NT::NOT);
                            $$->addChild($2);
                          }
                          | relationalExpression       
                          ;

relationalExpression      : sumExpression relationalOperator sumExpression
                          {
                            $2->addChild($1);
                            $2->addChild($3, 1);
                            $$ = $2;
                          }
                          | sumExpression   
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
                            $2->addChild($1);
                            $2->addChild($3, 1);
                            $$ = $2;
                          }
                          | mulExpression   
                          ;

sumOperator               : ADD      { $$ = new node::Node($1, NT::OPERATOR); }
                          | SUB      { $$ = new node::Node($1, NT::OPERATOR); }
                          ;

mulExpression             : mulExpression mulOperator unaryExpression 
                          {
                            $2->addChild($1); 
                            $2->addChild($3, 1);
                            $$ = $2; 
                          }
                          | unaryExpression   
                          ;

mulOperator               : MUL         { $$ = new node::Node($1, NT::OPERATOR); }
                          | DIV         { $$ = new node::Node($1, NT::OPERATOR); }
                          | MOD         { $$ = new node::Node($1, NT::OPERATOR); }
                          ;

unaryExpression           : unaryOperator unaryExpression 
                          { 
                            $1->addChild($2);
                            $$ = $1;
                          }
                          | factor      
                          ;

unaryOperator             : SUB         { $$ = new node::Node($1, NT::CHSIGN_UNARY); }
                          | MUL         { $$ = new node::Node($1, NT::SIZEOF_UNARY); }
                          | QUES        { $$ = new node::Node($1, NT::QUES_UNARY); }
                          ; 

factor                    : mutable     
                          | immutable   

mutable                   : ID  { $$ = new node::Node($1, NT::ID); }
                          | ID '[' expression ']' 
                          {
                            $$ = new node::Node($2, NT::ARRAY);
                            auto *id = new node::Node($1, NT::ID);
                            $$->addChild(id);
                            $$->addChild($3, 1);
                          }
                          ;

immutable                 : '(' expression ')'  { $$ = $2; }
                          | call                
                          | constant            
                          ;

call                      : ID '(' arguments ')' 
                          {
                            $$ = new node::Node($2, NT::CALL);
                            auto *id = new node::Node($1, NT::ID);
                            if ($3 != nullptr) {
                              $$->addChild($3);
                            }
                          }
                          ;

arguments                 : argumentList  
                          | %empty        { $$ = nullptr;}
                          ;

argumentList              : argumentList ',' expression 
                          { 
                            $1->setSibling($3);
                            $$ = $1;
                          }
                          | expression  
                          ;

                          
constant                  : CHARCONST   { $$ = new node::Node($1, NT::CHARACTER, VT::CHAR); }  
                          | NUMCONST    { $$ = new node::Node($1, NT::NUMBER, VT::INT); }   
                          | STRINGCONST { $$ = new node::Node($1, NT::STRING, VT::STRING); }  
                          | BOOLCONST   { $$ = new node::Node($1, NT::BOOLEAN, VT::BOOL); }  
                          ;                    
%%