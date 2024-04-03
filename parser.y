%{

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>

   #include "TokenData.h"
   #include "AST.h"
   #include "semantics.h"

   #define TYPES true
   #define NOTYPES false

   extern FILE *yyin;
   extern int yydebug;
   extern int yylineno;
   
   int yylex();
   void yyerror(const char *msg);

   int errs = 0;
   int warns = 0;

   Node * AST;
   int printTreeFlag = 0;
   int printAnnotatedTreeFlag = 0;

%}

%locations

%union {
   struct TokenData * tokenData;
   Node * nodePtr;
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

%%
program : declList   { AST = $1; }           

declList : 
   declList decl
   {
      if( $1 != NULL )
      {
         $$ = addSib($1, $2);
      } else {
         $$ = $2;
      }
   }      
   | decl               { $$ = $1; }
   ;

decl : varDecl                { $$ = $1; }   
     | funDecl                { $$ = $1; }
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
   };

scopedVarDecl : 
   STATIC typeSpec varDeclList ';'   
   {
      $3->nodeType = StaticNT;
      $3->isStatic = true;
      Node * tmp = $3;
      while( tmp != NULL )
      {
         tmp->dataType = $2->dataType;
         if( tmp->sibling != NULL )
         {
            tmp = tmp->sibling;
            tmp->isStatic = true;
         } else {
            tmp = NULL;
         }
      }
      $$ = $3;
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
   }   
   | varDeclInit                    { $$ = $1; }        
   ;

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
            }      
            ;

varDeclId : ID                      { $$ = createNode($1, VarNT); $$->isDeclared = true; $$->isInit = false; }       
          | ID OBRK NUMCONST ']'    { $$ = createNode($1, VarArrNT); $$->isArray = true; $$->isDeclared = true; $$->isInit = false; }
          ;

typeSpec : BOOL      { $$->dataType = BoolDT; }                
         | CHAR      { $$->dataType = CharDT; }            
         | INT       { $$->dataType = IntDT; }     
         ;

funDecl : 
   typeSpec ID '(' parms ')' compoundStmt
   {
      $$ = createNode($2, FuncNT);
      $$->dataType = $1->dataType;
      $$->isFunction = true;
      $$->isDeclared = true;
      $$->isUsed = true;
      $$->isInit = true;
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);

   }
   | ID '(' parms ')' compoundStmt
   {
      $$ = createNode($1, FuncNT);
      $$->dataType = VoidDT;
      $$->isFunction = true;
      $$->isDeclared = true;
      $$->isUsed = true;
      $$ = addChild($$, $3);
      $$ = addChild($$, $5);
   }
   ;          

parms : parmList                    { $$ = $1; }
      | empty                                   
      ;

parmList : 
   parmList ';' parmTypeList        
   { 
      $$ = $1;
      if( $3 != NULL )
      {
         $$ = addSib($1, $3);
      }
   }    
   | parmTypeList                   { $$ = $1; }               
   ;

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
   };            

parmIdList : 
   parmIdList ',' parmId
   {
      if( $1 != NULL )
      {
         $$ = addSib($1, $3);
      } else {
         $$ = $1;
      }
   }                
   | parmId                         { $$ = $1; }         
   ; 

parmId : ID                         { $$ = createNode($1, ParmNT); $$->isDeclared = true; $$->isInit = true; }
       | ID OBRK ']'                { $$ = createNode($1, ParmArrNT); $$->isArray = true; $$->isDeclared = true; $$->isInit = true; }
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
        | ';'           { $$ = NULL; }                                    
        ;

compoundStmt : 
   BEG localDecls stmtList END       
   {
      $$ = createNode($1, CompoundNT);
      $$ = addChild($$, $2);
      $$ = addChild($$, $3);
   }

localDecls : 
   localDecls scopedVarDecl
   {
      if( $1 != NULL ) 
      {
         $$ = addSib($1, $2);
      } else
      {
         $$ = $2;
      }
   }              
   | empty
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
   | empty
   ;

matchedSelectStmt : 
   IF simpleExp THEN matched ELSE matched
   {
      $$ = createNode($1, IfNT);
      if( $2->nodeType == IdNT ) { $2->isInit = true; }
      $$ = addChild($$, $2);
      $$ = addChild($$, $4); 
      $$ = addChild($$, $6);      
   };

unmatchedSelectStmt : 
   IF simpleExp THEN stmt
   {
      $$ = createNode($1, IfNT);
      if( $2->nodeType == IdNT ) { $2->isInit = true; }
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);   
   }
   | IF simpleExp THEN matched ELSE unmatched
   {
      $$ = createNode($1, IfNT);
      if( $2->nodeType == IdNT ) { $2->isInit = true; }
      $$ = addChild($$, $2);
      $$ = addChild($$, $4); 
      $$ = addChild($$, $6); 
   };
                    
matchedIterStmt : 
   WHILE simpleExp DO matched
   {
      $$ = createNode($1, IterNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);
   }
   | FOR ID ASGN iterRange DO matched
   {
      $$ = createNode($1, ToNT);
      Node * IDToken = createNode($2, VarNT);
      IDToken->dataType = IntDT;
      IDToken->isInit = true;
      IDToken->isDeclared = true;
      IDToken->isUsed = true;
      $$ = addChild($$, IDToken);
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);
   };

unmatchedIterStmt : 
   WHILE simpleExp DO unmatched
   {
      $$ = createNode($1, IterNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);
   }
   | FOR ID ASGN iterRange DO unmatched
   {
      $$ = createNode($1, ToNT);
      Node * IDToken = createNode($2, VarNT);
      IDToken->dataType = IntDT;
      IDToken->isInit = true;
      IDToken->isDeclared = true;
      IDToken->isUsed = true;
      $$ = addChild($$, IDToken);
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);
   };

iterRange :
   simpleExp TO simpleExp
   {
      $$ = createNode($2, RangeNT);
      $$->dataType = IntDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
   }                 
   | simpleExp TO simpleExp BY simpleExp
   {
      $$ = createNode($2, RangeNT);
      $$->dataType = IntDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
      $$ = addChild($$, $5);
   };

returnStmt : RETURN ';'     { $$ = createNode($1, ReturnNT); $$->dataType = VoidDT; }    
           | RETURN exp ';'
           {
               $$ = createNode($1, ReturnNT);
               $$ = addChild($$, $2); 
           };

breakStmt : BREAK ';'       { $$ = createNode($1, BreakNT); }    

exp : 
   mutable assignop exp
   {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;
   }       
   | mutable INC            { $$ = createNode($2, AssignNT); $$ = addChild($$, $1); $$->dataType = IntDT; }    
   | mutable DEC            { $$ = createNode($2, AssignNT); $$ = addChild($$, $1); $$->dataType = IntDT; }    
   | simpleExp              { $$ = $1; }   
   ;

assignop : ASGN              { $$ = createNode($1, AssignNT); }    
         | ADDASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         | SUBASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         | MULASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         | DIVASS            { $$ = createNode($1, AssignNT); $$->dataType = IntDT; }    
         ;

simpleExp :
   simpleExp OR andExp
   {
      $$ = createNode($2, OrNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);      
   } 
   | andExp                  { $$ = $1; }
   ;

andExp : 
   andExp AND unaryRelExp
   {
      $$ = createNode($2, AndNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
   } 
   | unaryRelExp             { $$ = $1; }
   ;

unaryRelExp : 
   NOT unaryRelExp
   {
      $$ = createNode($1, NotNT);
      $$->dataType = BoolDT;
      $$ = addChild($$, $2);
   } 
   | relExp                  { $$ = $1; }    
   ;

relExp : 
   sumExp relop sumExp
   {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;      
   }    
   | sumExp                  { $$ = $1; }
   ;

relop : LESS                 { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      | LEQ                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }       
      | GREAT                { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      | GEQ                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }   
      | EQL                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      | NEQ                  { $$ = createNode($1, OpNT); $$->dataType = BoolDT; }    
      ;

sumExp : 
   sumExp sumop mulExp
   {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;
   }    
   | mulExp               { $$ = $1; }     
   ;

sumop : ADD                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; } 
      | SUB                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }    
      ;

mulExp : 
   mulExp mulop unaryExp
   {
      $2 = addChild($2, $1);
      $2 = addChild($2, $3);
      $$ = $2;
   }   
   | unaryExp                { $$ = $1; }     
   ;

mulop : MUL                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }                     
      | DIV                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }    
      | MOD                  { $$ = createNode($1, OpNT); $$->dataType = IntDT; }    
      ;

unaryExp : unaryop unaryExp  { $$ = addChild($1, $2); }
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
                    
immutable : '(' exp ')'       { $$ = $2; }
          | call              { $$ = $1; }   
          | constant          { $$ = $1; }   
          ;

call : 
   ID '(' args ')' { 
      $$ = createNode($1, CallNT);
      if( $3 != NULL ) {
         $$ = addChild($$, $3);            
      }
   };

args : argList                { $$ = $1; }
     | empty
     ;                                       

argList : 
   argList ',' exp { 
         if( $1 != NULL )
         {
            $$ = addSib($1, $3);
         } else {
            $$ = $3;
         }
      }    
      | exp                   { $$ = $1; }               
      ;

constant : NUMCONST           { $$ = createNode($1, NumConstNT); $$->dataType = IntDT; $$->isConst = true; }              
         | CHARCONST          { $$ = createNode($1, CharConstNT); $$->dataType = CharDT; $$->isConst = true;   }           
         | STRINGCONST        { $$ = createNode($1, StringConstNT); $$->dataType = CharDT; $$->isConst = true; }          
         | BOOLCONST          { $$ = createNode($1, BoolConstNT); $$->dataType = BoolDT; $$->isConst = true;   }                 
         ;

empty: %empty { $$ = NULL; }                  
%%

int main(int argc, char *argv[]) {

   char * fext = (char *)"c-";
   char * file;

   if( argc == 2 )
   {
      file = argv[argc-1];
      FILE *fptr = fopen(file, "r");
      if(fptr) {
         yyin = fptr;
      } else {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs =+ 1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);  
      }
   }

   if( argc > 2 )
   {
      file = argv[argc-1];

      for(int i = 1; i < argc-1; i++)
      {
         if( argv[i][0] == '-' )
         {
            switch(argv[i][1])
            {
               case 'p':
                  //printf("Printing.\n");
                  printTreeFlag = 1;
                  break;
               case 'P':
                  printAnnotatedTreeFlag = 1;
                  break;
               case 'd':
                  //printf("Debug\n");
                  yydebug = 1;
                  break;
               default:
                  printf("Not a valid parameter option!\n");
                  exit(1);
            }
         }
      }

      if( strstr(file, fext) == NULL )
      {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs =+ 1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);
      }

      file = argv[argc-1];
      FILE *fptr = fopen(file, "r");
      if(fptr) {
         yyin = fptr;
      } else {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs =+ 1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);  
      }      
   }

   yyparse();

   SymbolTable symbolTable;
   semanticAnalysis(AST, &symbolTable);

   //if( printTreeFlag == 1 ) printAST(AST, 0, NOTYPES);
   if( errs == 0 ) {
      if( printAnnotatedTreeFlag == 1 ) printAST(AST, 0, TYPES);
   }

   printf("Number of warnings: %d\n", warns);
   printf("Number of errors: %d\n", errs);

   return 0;
}

void yyerror(const char* msg) {
   printf("YYERROR (%d) : \'%s\'\n", yylineno, msg);
}