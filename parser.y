%{
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>

   #include "TokenData.h"
   #include "AST.h"

   extern FILE *yyin;
   extern int yydebug;
   extern int yylineno;
   
   int yylex();
   void yyerror(const char *msg);

   Node * AST;
   int printTreeFlag = 0;

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
      Node * tmp = $3;
      while( tmp != NULL )
      {
         tmp->dataType = $2->dataType;
         if( tmp->sibling != NULL )
         {
            tmp = tmp->sibling;
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
                  if( $3 != NULL ) {
                     $$ = addChild($1, $3);
                  }
               } else {
                  $$ = $1;
               }
            }      
            ;

varDeclId : ID                      { $$ = createNode($1, VarNT); }       
          | ID OBRK NUMCONST ']'    { $$ = createNode($1, VarArrNT); }
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
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);

   }
   | ID '(' parms ')' compoundStmt
   {
      $$ = createNode($1, FuncNT);
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

parmId : ID                         { $$ = createNode($1, ParmNT); }
       | ID OBRK ']'                { $$ = createNode($1, ParmArrNT); }
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
      $$ = addChild($$, $2);
      $$ = addChild($$, $4); 
      $$ = addChild($$, $6);      
   };

unmatchedSelectStmt : 
   IF simpleExp THEN stmt
   {
      $$ = createNode($1, IfNT);
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);   
   }
   | IF simpleExp THEN matched ELSE unmatched
   {
      $$ = createNode($1, IfNT);
      $$ = addChild($$, $2);
      $$ = addChild($$, $4); 
      $$ = addChild($$, $6); 
   };
                    
matchedIterStmt : 
   WHILE simpleExp DO matched
   {
      $$ = createNode($1, IterNT);
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);
   }
   | FOR ID ASGN iterRange DO matched
   {
      $$ = createNode($1, ToNT);
      Node * IDToken = createNode($2, VarNT);
      IDToken->dataType = IntDT;
      $$ = addChild($$, IDToken);
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);
   };

unmatchedIterStmt : 
   WHILE simpleExp DO unmatched
   {
      $$ = createNode($1, IterNT);
      $$ = addChild($$, $2);
      $$ = addChild($$, $4);
   }
   | FOR ID ASGN iterRange DO unmatched
   {
      $$ = createNode($1, ToNT);
      Node * IDToken = createNode($2, VarNT);
      IDToken->dataType = IntDT;
      $$ = addChild($$, IDToken);
      $$ = addChild($$, $4);
      $$ = addChild($$, $6);
   };

iterRange :
   simpleExp TO simpleExp
   {
      $$ = createNode($2, RangeNT);
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
   }                 
   | simpleExp TO simpleExp BY simpleExp
   {
      $$ = createNode($2, RangeNT);
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
      $$ = addChild($$, $5);
   };

returnStmt : RETURN ';'     { $$ = createNode($1, ReturnNT); }    
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
   | mutable INC            { $$ = createNode($2, AssignNT); $$ = addChild($$, $1); }    
   | mutable DEC            { $$ = createNode($2, AssignNT); $$ = addChild($$, $1); }    
   | simpleExp              { $$ = $1; }    
   ;

assignop : ASGN              { $$ = createNode($1, AssignNT); }    
         | ADDASS            { $$ = createNode($1, AssignNT); }    
         | SUBASS            { $$ = createNode($1, AssignNT); }    
         | MULASS            { $$ = createNode($1, AssignNT); }    
         | DIVASS            { $$ = createNode($1, AssignNT); }    
         ;

simpleExp : 
   simpleExp OR andExp
   {
      $$ = createNode($2, OrNT);
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);      
   } 
   | andExp                  { $$ = $1; }
   ;

andExp : 
   andExp AND unaryRelExp
   {
      $$ = createNode($2, AndNT);
      $$ = addChild($$, $1);
      $$ = addChild($$, $3);
   } 
   | unaryRelExp             { $$ = $1; }
   ;

unaryRelExp : 
   NOT unaryRelExp
   {
      $$ = createNode($1, NotNT);
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

relop : LESS                 { $$ = createNode($1, OpNT); }    
      | LEQ                  { $$ = createNode($1, OpNT); }       
      | GREAT                { $$ = createNode($1, OpNT); }    
      | GEQ                  { $$ = createNode($1, OpNT); }   
      | EQL                  { $$ = createNode($1, OpNT); }    
      | NEQ                  { $$ = createNode($1, OpNT); }    
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

sumop : ADD                  { $$ = createNode($1, OpNT); } 
      | SUB                  { $$ = createNode($1, OpNT); }    
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

mulop : MUL                  { $$ = createNode($1, OpNT); }                     
      | DIV                  { $$ = createNode($1, OpNT); }    
      | MOD                  { $$ = createNode($1, OpNT); }    
      ;

unaryExp : unaryop unaryExp  { $$ = addChild($1, $2); }
         | factor            { $$ = $1; }  
         ;

unaryop : SUB                { $$ = createNode($1, SignNT); }    
        | MUL                { $$ = createNode($1, SizeOfNT); }    
        | QUES               { $$ = createNode($1, QuesNT); }    
        ;

factor : mutable             { $$ = $1; }
       | immutable           { $$ = $1; }   
       ;

mutable : ID                 { $$ = createNode($1, IdNT); }     
        | ID OBRK exp ']'
         {
            $$ = createNode($2, ArrNT);
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

constant : NUMCONST           { $$ = createNode($1, NumConstNT);    }              
         | CHARCONST          { $$ = createNode($1, CharConstNT);   }           
         | STRINGCONST        { $$ = createNode($1, StringConstNT); }          
         | BOOLCONST          { $$ = createNode($1, BoolConstNT);   }                 
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
         fprintf(stderr, "ERROR: Failed to open \'%s\'.\n", file);
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
         fprintf(stderr, "ERROR: Failed to open \'%s\'.\n", file);
         exit(1);
      }

      file = argv[argc-1];
      FILE *fptr = fopen(file, "r");
      if(fptr) {
         yyin = fptr;
      } else {
         fprintf(stderr, "ERROR: Failed to open \'%s\'.\n", file);
         exit(1);  
      }      
   }

   yyparse();
   if( printTreeFlag == 1 ) printAST(AST, 0);
   return 0;
}

void yyerror(const char* msg) {
   printf("YYERROR (%d) : \'%s\'\n", yylineno, msg);
}