#ifndef _YYERROR_H_
#define _YYERROR_H_

#define YYERROR_VERBOSE

#include "TokenData.h"           // For lastToken...
#include "AST.hpp"                 // You never know...
#include "parser.tab.h"          // For lnNum, errs, and warns...

// These variables do interface with my code!!!
extern TokenData * lastToken;
extern int lnNum;                   // Line number of last token scanned in your scanner (.l)
extern int errs;                    // Number of errors.
extern int warns;                   // Number of warnings.

void initErrorProcessing();            // WARNING: MUST be called before any errors occur (near top of main)!
void yyerror(const char *msg);         // Error routine called by Bison

#endif