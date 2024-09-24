#include <cstdio>
#include <cstdlib>
#include "lexer.hpp"
#include "parser.hpp"

extern FILE *yyin;

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