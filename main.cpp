#include "utils.hpp"
#include "semantic.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <iostream>

#define ARG_DEBUG false

extern int yydebug;
extern FILE *yyin;

typedef semantic::SemanticAnalyzer SA;

int main(int argc, char **argv) {
   argc--; argv++; // skip the first argument

   bool printAST = false;

   #if ARG_DEBUG
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
   }

   if( argc > 1 ) { // more than one argument is passed, i.e. '-p file.c-'
      for( size_t i = 0; i < argc; i++) {
         if (argv[i][0] == '-') { // check if the argument is a flag
            switch (argv[i][1]) {
               case 'd': // debug the lexer
                  #if ARG_DEBUG
                  std::cout << "YYDEBUG" << std::endl;
                  #endif
                  yydebug = 1;
                  break;
               case 'D': // symbol table debug
                  #if ARG_DEBUG
                  std::cout << "SYMBOL TABLE" << std::endl;
                  #endif
                  break;
               case 'h': // print the help menu
                  #if ARG_DEBUG
                  std::cout << "HELP" << std::endl;
                  #endif
                  utils::printHelpMenu();
                  break;
               case 'p': // print the AST
                  #if ARG_DEBUG
                  std::cout << "PRINT AST" << std::endl;
                  #endif
                  printAST = true;
                  break;
               case 'P': // print the AST w/ types
                  #if ARG_DEBUG
                  std::cout << "PRINT AST TYPES" << std::endl;
                  #endif
                  printAST = utils::PRINT_TYPES = true;
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

   SA *semanticAnalyzer = new SA(node::root); // Semantic analysis

   if (printAST) { utils::printTree(); }

   semanticAnalyzer->printWarnings();
   semanticAnalyzer->printErrors();

   return 0;
}

void yyerror(const char *msg) {
   std::cout << "YYERROR(" << yylineno << "): " << msg << std::endl;
}