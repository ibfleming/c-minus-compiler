#include "utils.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <iostream>

#define argDebug true

extern FILE *yyin;

int main(int argc, char **argv) {
   argc--; argv++; // skip the first argument

   bool printAST = false;

   #if argDebug
   std::cout << "ARGUMENTS(" << argc << "): ";
   for( size_t i = 0; i < argc; i++) {
      std::cout << argv[i] << " ";
   }
   std::cout << std::endl;
   #endif

   if( argc == 1 ) { // if only one argument is passed; assume it is the file name
      if( utils::checkFileExtension(argv[0]) ) { 
         yyin = fopen(argv[0], "r");
         if (yyin == NULL){
            std::cerr << "Error: Opening file: '" << argv[0] << "'" << std::endl;
            return 0;
         }
      }
      return 0;
   }

   if( argc > 1 ) { // more than one argument is passed, i.e. '-p file.c-'
      for( size_t i = 0; i < argc; i++) {
         if (argv[i][0] == '-') { // check if the argument is a flag
            switch (argv[i][1]) {
               case 'p': // print the AST
                  printAST = true;
                  #if argDebug
                  std::cout << "PRINT AST" << std::endl;
                  #endif
                  break;
               default:
                  std::cerr << "Error: Invalid argument '" << argv[i] << "'" << std::endl;
                  return 0;
            }
         }
         else {
            if( utils::checkFileExtension(argv[i]) ) { 
               yyin = fopen(argv[i], "r");
               if (yyin == NULL){
                  std::cerr << "Error: Opening file: '" << argv[i] << "'" << std::endl;
                  return 0;
               }
            }
         }
      }
   }

   yyparse();

   if (printAST) { utils::printTree(); }

   return 0;
}

void yyerror(const char *msg) {
   std::cout << "YYERROR(" << yylineno << "): " << msg << std::endl;
}