%{
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "scanType.h" 

   extern FILE *yyin;
   extern int yydebug;
   
   int yylex();
   void yyerror(const char *msg);
   void printTknData(struct TokenData *tokenData);
%}

%union {
   struct TokenData *tokenData;
}

%token <tokenData> ID NUMCONST CHARCONST STRINGCONST BOOLCONST OTHER
%type <tokenData> token exp

%%
exp : token
    | exp token
    ;

token : ID                    { printTknData($1); } 
      | NUMCONST              { printTknData($1); }
      | CHARCONST             { printTknData($1); }
      | STRINGCONST           { printTknData($1); }
      | BOOLCONST             { printTknData($1); } 
      | OTHER                 { printTknData($1); }      
      ;
%%

void printTknData(struct TokenData *tokenData) {
   if( tokenData->tknClass == ID ) {
      printf("Line %d Token: ID Value: %s\n", tokenData->lineNum, tokenData->tknStr);
   }
   if( tokenData->tknClass == NUMCONST )
   {
      printf("Line %d Token: NUMCONST Value: %d  Input: %s\n", tokenData->lineNum, tokenData->nVal, tokenData->tknStr);
   }
   if( tokenData->tknClass == STRINGCONST )
   {
      printf("Line %d Token: STRINGCONST Value: %s  Len: %ld  Input: %s\n", tokenData->lineNum, tokenData->strVal, strlen(tokenData->strVal) - 2, tokenData->tknStr);
   }
   if( tokenData->tknClass == CHARCONST )
   {
      printf("Line %d Token: CHARCONST Value: \'%c\' Input: %s\n", tokenData->lineNum, tokenData->cVal, tokenData->tknStr);
   }
   if( tokenData->tknClass == BOOLCONST )
   {
      printf("Line %d Token: BOOLCONST Value: %d  Input: %s\n", tokenData->lineNum, tokenData->nVal, tokenData->tknStr);
   }
   if( tokenData->tknClass == OTHER )
   {
      printf("Line %d Token: %s\n", tokenData->lineNum, tokenData->strVal);
   }
}

int main(int argc, char *argv[]) {
   
   if( argc > 1 )
   {
      if( (yyin = fopen(argv[1], "r")) ) 
      {}
      else {
         fprintf(stderr, "ERROR: Failed to open \'%s\'.\n", argv[1]);
         exit(1);
      }
   }

   yyparse();

   return 0;
}

void yyerror(const char* msg) {
   printf("YYERROR: \'%s\'\n", msg);
}