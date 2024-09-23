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

%token <token> NUMCONST
%type <token> token

%% 

expression  :   token
            |   expression token
            ;

token       :   NUMCONST { $$ = $1; $$->print(); delete $1; }
            ;

%%

int main(int argc, char **argv) {
   if (argc > 1) {
      yyin = fopen(argv[1], "r");
      if (yyin == NULL){
         printf("syntax: %s filename\n", argv[0]);
      }
   }
   yyparse();
   return 0;
}

void yyerror(const char *msg) {
   printf("ERROR(%d): %s\n", yylloc.first_line, msg);
}