#include "AST.h"
#include "symbolTable.h"
#include "semantics.h"
#include "routines.h"
#include "yyerror.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>

#define TYPES true                     // Flag for AST print() with data types.
#define NOTYPES false                  // Flag for AST print() without data types.

extern FILE *yyin;
extern int yydebug;

Node * AST;                            // AST of entire program.
Node * rAST;                           // AST of I/O routines.
int printTreeFlag = 0;                 // Print flag for AST without types.
int printAnnotatedTreeFlag = 0;        // Print flag for AST with types.
int warns = 0;                         // GLOBAL DECLARATION => Counter for all warnings in the program
int errs = 0;                          // GLOBAL DECLARATION => Counter for all errors in the program

/* ==================================================
   main() function for C- compiler!
   ================================================== */
int main(int argc, char *argv[]) {

   char * fext = (char *)"c-";      // File extension tag. 
   char * file;                     // File itself.

   // Exactly two arguments './c- <file>', check if file can be opened.
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

      // Detect passed arguments.
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

      // No file was passed in arguments and there are more than 2 arguments.
      if( strstr(file, fext) == NULL )
      {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs =+ 1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);
      }

      // File was passed but doesn't exist or can be opened and there are more than 2 arguments.
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
   /* ================================================== */

   initErrorProcessing();                 // Generate map for syntax analysis.
   yyparse();                             // Fetch the symbols & Lexical/Syntax Analysis.
   
   SymbolTable ST;                        // Create the symbol table object.
   rAST = createRoutineAST();             // Create and initialize the routine symable table!
   ST = traverseRoutineAST(rAST, &ST);    // Traverse through the routine AST and update the main AST.
   if( errs == 0  ) {
      traverseAST(AST, &ST);              // Traverse the entire AST (program) & Semantic Analysis.
      checkMain(&ST);                     // External function to check if program has a main() function, [LINKER ERROR].
   }                       

   // If program has no errors, print out the AST.
   if( errs == 0 ) {
      if( printAnnotatedTreeFlag == 1 ) { printAST(AST, 0, TYPES); }
      else if( printTreeFlag == 1 ) { printAST(AST, 0, NOTYPES); }
   }

   printf("Number of warnings: %d\n", warns);   // Print warning counter.
   printf("Number of errors: %d\n", errs);      // Print error counter.

   return 0;
}