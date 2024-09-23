%code requires{
    #include "token.hpp"
}

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string>
    #include "lexer.hpp"
    void yyerror(const char *msg);
%}

%union{
  token::Token* token;
}

%define parse.error verbose
%locations

%start expression
%type  <token> token
%token <token> ID NUMCONST CHARCONST STRINGCONST BOOLCONST

%% 

expression  :   token
            |   expression token
            ;

token       :   ID            { $$ = $1; $$->print(); delete $1; }
            |   NUMCONST      { $$ = $1; $$->print(); delete $1; }
            |   CHARCONST     { $$ = $1; $$->print(); delete $1; }
            |   STRINGCONST   { $$ = $1; $$->print(); delete $1; }
            |   BOOLCONST     { $$ = $1; $$->print(); delete $1; }
            ;

%%

int main(int argc, char **argv) {
   if (argc > 1) {
      yyin = fopen(argv[1], "r");
      if (yyin == NULL){
         printf("Error opening file.\n");
      }
   }
   yyparse();
   return 0;
}

void yyerror(const char *msg) {
   return;
}