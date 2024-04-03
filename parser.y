%{

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>

   #include "TokenData.h"
   #include "AST.hpp"
   #include "semantics.hpp"
   #include "routines.hpp"
   #include "yyerror.hpp"

   #define YYERROR_VERBOSE
   
   extern int yylex();
   extern Node * AST;
   extern int lnNum;
   extern int errs;
   extern int warns;

%}

%union {
   TokenData * tokenData;
   Node * nodePtr;
   char * literal;
}

%token <tokenData> ID NUMCONST CHARCONST STRINGCONST BOOLCONST 
%token <tokenData> BOOL CHAR INT STATIC 
%token <tokenData> SUB MUL QUES DIV MOD ADD
%token <tokenData> LESS GREAT LEQ GEQ EQL NEQ
%token <tokenData> AND NOT OR ASGN ADDASS SUBASS MULASS DIVASS INC DEC OBRK 
%token <tokenData> RETURN BREAK TO BY WHILE DO FOR IF THEN ELSE BEG END

%type <nodePtr> 
constant immutable mutable factor unaryop unaryExp mulop mulExp sumop sumExp relop relExp
unaryRelExp andExp simpleExp assignop exp argList args call breakStmt returnStmt iterRange
stmtList localDecls compoundStmt expStmt stmt parmId parmIdList parmTypeList parmList parms
funDecl typeSpec varDeclId varDeclInit varDeclList scopedVarDecl varDecl decl declList program 
empty matched unmatched matchedSelectStmt matchedIterStmt unmatchedIterStmt unmatchedSelectStmt

%define parse.error verbose

%%

program : declList   { AST = addSib(AST, $1); /* Did this based on my modifications in addSib(). */ }
        | empty { 
            printf("ERROR(%d): Syntax error, unexpected end of input, expecting \"bool\" or \"char\" or \"int\" or identifier.\n", lnNum);
            errs += 1; 
        };           

declList : 
   declList decl
   {
      if( $1 != NULL && $2 != NULL )
      {
         $$ = addSib($1, $2);
      } else {
         $$ = $2;
      }
   }      
   | decl                     { $$ = $1; }
   ;

decl : varDecl                { $$ = $1; }   
     | funDecl                { $$ = $1; }
     | error                  { $$ = NULL; }
     ;

varDecl : 
   typeSpec varDeclList ';'
   {
      Node * tmp = $2;
      while( tmp != NULL )
      {
         tmp->dataType = $1->dataType;
         if( tmp->sibling != NULL )
         {
            tmp = tmp->sibling;
         } else {
            tmp = NULL;
         }
      }
      $$ = $2;
      yyerrok;
   }|
   error varDeclList ';' {
      $$ = NULL;
      yyerrok;
   }|
   typeSpec error ';' {
      $$ = NULL;
      yyerrok;
   };

scopedVarDecl : 
   STATIC typeSpec varDeclList ';'   
   {
      Node * tmp = $3;
      while( tmp != NULL )
      {
         tmp->nodeType = StaticNT;
         tmp->refType = StaticRT;
         tmp->dataType = $2->dataType;
         tmp->isStatic = true;
         tmp->isInit = 1;
         tmp->is_decl = true;
         if( tmp->sibling != NULL )
         {
            tmp = tmp->sibling;
         } else {
            tmp = NULL;
         }
      }
      $$ = $3;
      yyerrok;
   }
   | typeSpec varDeclList ';' 
   {
      Node * tmp = $2;
      while( tmp != NULL )
      {
         tmp->dataType = $1->dataType;
         if( tmp->sibling != NULL )
         {
            tmp = tmp->sibling;
         } else {
            tmp = NULL;
         }
      }
      $$ = $2;
      yyerrok;
   };         

varDeclList : 
   varDeclList ',' varDeclInit
   {
      if( $1 != NULL )
      {
         $$ = addSib($1, $3);
      } else {
         $$ = $3;
      }
      yyerrok;
   }|
   varDeclList ',' error {
      $$ = NULL;
   }| 
   varDeclInit                    
   { 
      $$ = $1; 
   }|
   error {
      $$ = NULL;
   };

varDeclInit : varDeclId             { $$ = $1; }             
            | varDeclId ':' simpleExp
            {
               if( $1 != NULL )
               {
                  $$->isInit = true;
                  if( $3 != NULL ) {
                     $$ = addChild($1, $3);
                  }
               } else {
                  $$ = $1;
               }
            }|
            error ':' simpleExp 
            {
               $$ = NULL;
               yyerrok;
            };

varDeclId : ID {                 
               $$ = createNode($1, VarNT); $$->isInit = false; $$->is_decl = true; 
            }| 
            ID OBRK NUMCONST ']' { 
               $$ = createNode($1, VarArrNT); 
               $$->is_decl = true;
               $$->isArray = true;
               $$->isIndexed = false; 
               $$->size = $3->nVal + 1;
            }| 
            ID OBRK error {
               $$ = NULL;
            }|
            error ']' {
               $$ = NULL;
               yyerrok;
            };

typeSpec : BOOL      { $$ = createNode($1, TypeNT); $$->dataType = BoolDT;  }                
         | CHAR      { $$ = createNode($1, TypeNT); $$->dataType = CharDT; }            
         | INT       { $$ = createNode($1, TypeNT); $$->dataType = IntDT; }     
         ;

funDecl : 
   typeSpec ID '(' parms ')' compoundStmt {
      $$ = createNode($2, FuncNT);
      $$->refType = GlobalRT;
      $$->dataType = $1->dataType;
      $$->isInit = true;
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);

   }|
   typeSpec error {
      $$ = NULL;
   }|
   typeSpec ID '(' error {
      $$ = NULL;
   }| 
   ID '(' parms ')' compoundStmt {
      $$ = createNode($1, FuncNT);
      $$->refType = GlobalRT;
      $$->dataType = VoidDT;
      $$ = addChild($$, $3);
      $$ = addChild($$, $5);
   }|
   ID '(' error {
      $$ = NULL;
   }|
   ID '(' parms ')' error {
      $$ = NULL;
   };          

parms : parmList                    { $$ = $1; }
      | empty                       { $$ = NULL; }                 
      ;

parmList : 
   parmList ';' parmTypeList        
   { 
      if( $1 != NULL && $3 != NULL) {
         $$ = addSib($1, $3);
      }
      else {
         $$ = $1;
      }
   }|
   parmList ';' error {
      $$ = NULL;
   }| 
   parmTypeList { 
      $$ = $1; 
   }|
   error {
      $$ = NULL;
   };

parmTypeList : 
   typeSpec parmIdList
   {
      Node * temp = $2;
      while( temp != NULL )
      {
         temp->dataType = $1->dataType;
         if( temp->sibling != NULL )
         {
            temp = temp->sibling;
         } else {
            temp = NULL;
         }
      }
      $$ = $2;
   }|
   typeSpec error {
      $$ = NULL;
   };            

parmIdList : 
   parmIdList ',' parmId
   {
      if( $1 != NULL && $3 != NULL )
      {
         $$ = addSib($1, $3);
      } else {
         $$ = $1;
      }
      yyerrok;
   }|
   parmIdList ',' error {
      $$ = NULL;
   }      
   | parmId                         { $$ = $1; }    
   | error                          { $$ = NULL; }     
   ; 

parmId : ID                         { $$ = createNode($1, ParmNT); $$->isInit = true; $$->refType = ParamRT; $$->is_decl = true; }
       | ID OBRK ']'                { $$ = createNode($1, ParmArrNT); $$->isArray = true; $$->isInit = true; $$->refType = ParamRT; $$->is_decl = true; }
       ;

stmt : matched                      { $$ = $1; }
     | unmatched                    { $$ = $1; }
     ;

matched : expStmt                   { $$ = $1; }
        | compoundStmt              { $$ = $1; }
        | matchedSelectStmt         { $$ = $1; }
        | matchedIterStmt           { $$ = $1; }
        | returnStmt                { $$ = $1; }
        | breakStmt                 { $$ = $1; }
        ;

unmatched : unmatchedSelectStmt     { $$ = $1; }
          | unmatchedIterStmt       { $$ = $1; }
          ;

expStmt : exp ';'       { $$ = $1;   }
        | error ';'     { $$ = NULL; yyerrok; }                          
        | ';'           { $$ = NULL; }                                    
        ;

compoundStmt : 
   BEG localDecls stmtList END       
   {
      $$ = createNode($1, CompoundNT);
      $$ = addChild($$, $2);
      $$ = addChild($$, $3);
      yyerrok;
   }

localDecls : 
   localDecls scopedVarDecl
   {
      if( $1 != NULL && $2 != NULL ) {
         $$ = addSib($1, $2);
      } else {
         $$ = $2;
      }
   }              
   | empty        { $$ = NULL; }
   ;

stmtList : 
   stmtList stmt
   {
      if( $1 != NULL )
      {
         $$ = addSib($1, $2);
      } else {
         $$ = $2;
      }
   }                    
   | empty        { $$ = NULL; }
   ;

matchedSelectStmt : 
   IF simpleExp THEN matched ELSE matched {
      $$ = createNode($1, IfNT);
      if( $2->nodeType == IdNT ) { $2->isInit = true; }
      $$ = addChild($$, $2);
      $$ = addChild($$, $4); 
      $$ = addChild($$, $6);      
   }|
   IF error {
      $$ = NULL;
   }|
   IF error ELSE matched {
      $$ = NULL;
      yyerrok;
   }|
   IF error THEN matched ELSE matched {
      $$ = NULL;
      yyerrok;
   }
   ;

unmatchedSelectStmt : 
   IF simpleExp THEN stmt {
      $$ = createNode($1, IfNT);
      if( $2->nodeType == IdNT ) { $2->isInit = true; }
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);   
   }| 
   IF error THEN stmt {
      $$ = NULL;
      yyerrok;
   }|
   IF simpleExp THEN matched ELSE unmatched {
      $$ = createNode($1, IfNT);
      if( $2->nodeType == IdNT ) { $2->isInit = true; }
      $$ = addChild($$, $2);
      $$ = addChild($$, $4); 
      $$ = addChild($$, $6); 
   }|
   IF error THEN matched ELSE unmatched {
      $$ = NULL;
      yyerrok;
   }
   ;
                    
matchedIterStmt : 
   WHILE simpleExp DO matched {
      $$ = createNode($1, IterNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);
   }|
   WHILE error DO matched {
      $$ = NULL;
      yyerrok;
   }|
   WHILE error {
      $$ = NULL;
   }| 
   FOR ID ASGN iterRange DO matched {
      $$ = createNode($1, ToNT);
      Node * IDToken = createNode($2, VarNT);
      IDToken->is_decl = true;
      IDToken->dataType = IntDT;
      IDToken->isInit = true;
      $$ = addChild($$, IDToken);
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);
   }| 
   FOR ID ASGN error DO matched {
      $$ = NULL;
      yyerrok;
   }|
   FOR error {
      $$ = NULL;
   }
   ;

unmatchedIterStmt : 
   WHILE simpleExp DO unmatched {
      $$ = createNode($1, IterNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);
   }|
   WHILE error DO unmatched {
      $$ = NULL;
      yyerrok;
   }| 
   FOR ID ASGN iterRange DO unmatched {
      $$ = createNode($1, ToNT);
      Node * IDToken = createNode($2, VarNT);
      IDToken->is_decl = true;
      IDToken->dataType = IntDT;
      IDToken->isInit = true;
      $$ = addChild($$, IDToken);
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);
   }|
   FOR ID ASGN error DO unmatched {
      $$ = NULL;
      yyerrok;
   }
   ;

iterRange :
   simpleExp TO simpleExp {
      $$ = createNode($2, RangeNT);
      $$->dataType = IntDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
   }| 
   simpleExp TO simpleExp BY simpleExp {
      $$ = createNode($2, RangeNT);
      $$->dataType = IntDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
      $$ = addChild($$, $5);
   }|
   simpleExp TO error {
      $$ = NULL;
   }|
   error BY error {
      $$ = NULL;
      yyerrok;
   }|
   simpleExp TO simpleExp BY error {
      $$ = NULL;
   }
   ;

returnStmt : 
   RETURN ';' { 
      $$ = createNode($1, ReturnNT);
      $$->dataType = VoidDT; 
   }| 
   RETURN exp ';' {
      $$ = createNode($1, ReturnNT);
      $$ = addChild($$, $2);
      yyerrok; 
   }| 
   RETURN error ';' {
      $$ = NULL;
      yyerrok;
   }
   ;

breakStmt : 
   BREAK ';' { 
      $$ = createNode($1, BreakNT); 
   }
   ;

exp : 
   mutable assignop exp {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;
   }|
   error assignop exp {
      $$ = NULL;
      yyerrok;
   }|
   mutable assignop error {
      $$ = NULL;
   }| 
   mutable INC { 
      $$ = createNode($2, AssignNT); 
      $$ = addChild($$, $1); 
      $$->dataType = IntDT; 
   }| 
   mutable DEC { 
      $$ = createNode($2, AssignNT); 
      $$ = addChild($$, $1); 
      $$->dataType = IntDT; 
   }| 
   simpleExp { 
      $$ = $1; 
   }|
   error INC {
      $$ = NULL;
      yyerrok;
   }|
   error DEC {
      $$ = NULL;
      yyerrok;
   }
   ;

assignop : ASGN              { $$ = createNode($1, AssignNT); }    
         | ADDASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         | SUBASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         | MULASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         | DIVASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         ;

simpleExp :
   simpleExp OR andExp {
      $$ = createNode($2, OrNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);      
   }|
   simpleExp OR error {
      $$ = NULL;   
   }| 
   andExp { 
      $$ = $1; 
   }
   ;

andExp : 
   andExp AND unaryRelExp {
      $$ = createNode($2, AndNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
   }| 
   unaryRelExp { 
      $$ = $1; 
   }|
   andExp AND error {
      $$ = NULL;
   }
   ;

unaryRelExp : 
   NOT unaryRelExp {
      $$ = createNode($1, NotNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $2);
   }|
   NOT error {
      $$ = NULL;
   }| relExp { 
      $$ = $1; 
   }    
   ;

relExp : 
   sumExp relop sumExp {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;      
   }| 
   sumExp relop error { 
      $$ = NULL; 
   }|
   sumExp {
      $$ = $1;
   }
   ;

relop : LESS                 { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      | LEQ                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }       
      | GREAT                { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      | GEQ                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }   
      | EQL                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      | NEQ                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      ;

sumExp : 
   sumExp sumop mulExp {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;
   }|
   sumExp sumop error {
      $$ = NULL;
   }| 
   mulExp { 
      $$ = $1; 
   }     
   ;

sumop : ADD                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; } 
      | SUB                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }    
      ;

mulExp : 
   mulExp mulop unaryExp {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;
   }|
   mulExp mulop error {
      $$ = NULL;
   }| 
   unaryExp { 
      $$ = $1; 
   }     
   ;

mulop : MUL                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }                     
      | DIV                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }    
      | MOD                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }    
      ;

unaryExp : unaryop unaryExp  { $$ = addChild($1, $2); }
         | unaryop error     { $$ = NULL; }
         | factor            { $$ = $1; }  
         ;

unaryop : SUB                { $$ = createNode($1, SignNT); $$->dataType = IntDT; $$->literal = (char *)"chsign"; }    
        | MUL                { $$ = createNode($1, SizeOfNT); $$->dataType = IntDT; $$->literal = (char *)"sizeof";}    
        | QUES               { $$ = createNode($1, QuesNT); $$->dataType = IntDT; }    
        ;

factor : mutable             { $$ = $1; }
       | immutable           { $$ = $1; }   
       ;

mutable : ID                 { $$ = createNode($1, IdNT); }     
        | ID OBRK exp ']'
         {
            $$ = createNode($2, ArrNT);
            $$->isArray = true;
            Node * child = createNode($1, IdNT);
            $$ = addChild($$, child);
            $$ = addChild($$, $3);
         };          
                    
immutable : '(' exp ')'       { $$ = $2; yyerrok; }
          | '(' error         { $$ = NULL; }
          | call              { $$ = $1; }   
          | constant          { $$ = $1; }   
          ;

call : 
   ID '(' args ')' { 
      $$ = createNode($1, CallNT);
      $$->dataType = VoidDT;
      if( $3 != NULL ) {
         $$ = addChild($$, $3);            
      }
   }|
   error '(' {
      $$ = NULL;
      yyerrok;
   }
   ;

args : argList                { $$ = $1; }
     | empty                  { $$ = NULL; }
     ;                                       

argList : 
   argList ',' exp { 
      if( $1 != NULL )
      {
         $$ = addSib($1, $3);
      } else {
         $$ = $3;
      }
      yyerrok;
   }|
   argList ',' error {
      $$ = NULL;
   }    
   | exp                   { $$ = $1; }               
   ;

constant : NUMCONST           { $$ = createNode($1, NumConstNT); $$->dataType = IntDT; $$->isConst = true; $$->isInit = true; }              
         | CHARCONST          { $$ = createNode($1, CharConstNT); $$->dataType = CharDT; $$->isConst = true; $$->isInit = true; }           
         | STRINGCONST { 
            $$ = createNode($1, StringConstNT); 
            $$->dataType = CharDT;
            $$->refType = GlobalRT; 
            $$->isConst = true; 
            $$->isArray = true; 
            $$->isInit = true;
            $$->size = $1->tknLen - 1;
         } 
         | BOOLCONST          { $$ = createNode($1, BoolConstNT); $$->dataType = BoolDT; $$->isConst = true; $$->isInit = true; }                 
         ;

empty: %empty { ; }                  
%%

