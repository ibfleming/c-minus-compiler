#include "utils.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <iostream>

extern FILE *yyin;

int main(int argc, char **argv) {

   if (argc > 1) {

      std::string filename = argv[1];
      if (filename.find(".c-") == std::string::npos) { // check valid file type e.g. program.c-
          std::cerr << "Error: File name must contain '.c-'" << std::endl;
          return 0; // terminate program
      }

      yyin = fopen(argv[1], "r");
      if (yyin == NULL){
          std::cerr << "Error opening file: '" << argv[1] << "'" << std::endl;
          return 0; // terminate program
      }

   }

   yyparse();

   utils::printTree();

   return 0;
}

void yyerror(const char *msg) {
   std::cout << "YYERROR(" << yylineno << "): " << msg << std::endl;
}