#include "AST.hpp"
#include "symbolTable.hpp"
#include "semantics.hpp"
#include "routines.hpp"
#include "code_gen.hpp"
#include "yyerror.hpp"
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>

#define TYPES true    // Flag for AST print() with data types.
#define NOTYPES false // Flag for AST print() without data types.
bool MY_DEBUG = false;

extern FILE *yyin;
extern int yydebug;

FILE *code;
Node *AST;                      // AST of entire program.
Node *rAST;                     // AST of I/O routines.
int printTreeFlag = 0;          // Print flag for AST without types.
int printAnnotatedTreeFlag = 0; // Print flag for AST with types.
int printAugmentedTreeFlag = 0; // Print flag for augmented AST.
int gc_flag = 0;

int warns = 0; // GLOBAL DECLARATION => Counter for all warnings in the program
int errs = 0;  // GLOBAL DECLARATION => Counter for all errors in the program
int foffset = 0;
int goffset = 0;

/* ==================================================
   main() function for C- compiler!
   ================================================== */
int main(int argc, char *argv[])
{

   char *test_type = strdup(argv[argc - 1]);
   char *file;
   char *fext = (char *)"c-"; // File extension tag.

   // Exactly two arguments './c- <file>', check if file can be opened.
   if (argc == 2)
   {
      FILE *fptr = fopen(argv[argc - 1], "r");

      file = argv[argc - 1];
      *strchr(file, '.') = '\0';
      strcat(file, ".tm");

      code = fopen(file, "w+");

      if (fptr)
      {
         yyin = fptr;
         gc_flag = 1;
      }
      else
      {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs = +1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);
      }

      char temp[32];
      strcpy(temp, test_type);
      char *lastSlash = strrchr(temp, '/');
      if (lastSlash != NULL)
      {
         size_t length = lastSlash - temp;
         strncpy(test_type, temp, length);
         test_type[length] = '\0';
      }
   }

   if (argc > 2)
   {
      file = argv[argc - 1];

      // Detect passed arguments.
      for (int i = 1; i < argc - 1; i++)
      {
         if (argv[i][0] == '-')
         {
            switch (argv[i][1])
            {
            case 'M':
               printAugmentedTreeFlag = 1;
               break;
            case 'p':
               printTreeFlag = 1;
               break;
            case 'P':
               printAnnotatedTreeFlag = 1;
               break;
            case '#':
               MY_DEBUG = true;
               break;
            case 'd':
               yydebug = 1;
               break;
            default:
               printf("ERROR(ARGLIST): Not a valid parameter option!\n");
               exit(1);
            }
         }
      }

      // No file was passed in arguments and there are more than 2 arguments.
      if (strstr(file, fext) == NULL)
      {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs = +1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);
      }

      // Arguments are error-free, open file and check it for errors.
      file = argv[argc - 1];
      FILE *fptr = fopen(file, "r");
      if (fptr)
      {
         yyin = fptr;
      }
      else
      {
         printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", file);
         errs = +1;
         printf("Number of warnings: %d\n", warns);
         printf("Number of errors: %d\n", errs);
         exit(1);
      }
   }
   /* ================================================== */

   initErrorProcessing(); // Generate map for syntax analysis.
   yyparse();             // Fetch the symbols & Lexical/Syntax Analysis.

   SymbolTable ST;                     // Create the symbol table object.
   rAST = createRoutineAST(NULL);      // // Create and initialize the routine symable table!
   ST = traverseRoutineAST(rAST, &ST); // Traverse through the routine AST and update the main AST.
   if (errs == 0)
   {
      traverseAST(AST, &ST);       // Traverse the entire AST (program) & Semantic Analysis.
      checkMain(&ST);              // External function to check if program has a main() function, [LINKER ERROR].
      AST = createRoutineAST(AST); // Add Routines to AST!
   }

   // If program has no errors, print out the AST.
   if (errs == 0)
   {
      if (printAnnotatedTreeFlag == 1)
      {
         printAST(AST, 0, withTypes);
      }
      if (printTreeFlag == 1)
      {
         printAST(AST, 0, withoutTypes);
      }
      if (printAugmentedTreeFlag == 1)
      {
         printAST(AST, 0, isAugmented); // Will print any memory errors...
         printf("Offset for end of global space: %d\n", goffset);
      }
      if (gc_flag == 1)
      {
         fix_memory_loops(AST);
         generate_code(AST, rAST); // Generate the code in a .tm file.
         // printAST(AST, 0, isAugmented); // Will print memory fixes...
         if (strcmp(test_type, "UnitTests") == 0)
         {
            printf("Number of warnings: %d\n", warns); // Print warning counter.
            printf("Number of errors: %d\n", errs);    // Print error counter.
         }
      }
   }
   else
   {
      printf("Number of warnings: %d\n", warns); // Print warning counter.
      printf("Number of errors: %d\n", errs);    // Print error counter.
   }

   fflush(code);
   fcloseall();

   return 0;
}