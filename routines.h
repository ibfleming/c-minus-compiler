#ifndef _ROUTINES_H
#define _ROUTINES_H

#include "scope.h"

/* ==================================================
   ROUTINE I/O LIBRARY DEFITIONS              
   ================================================== */

Node * createRoutineAST();
SymbolTable traverseRoutineAST(Node * rAST, SymbolTable * ST);

/* ==================================================
   SEMANTIC HELPER DEFINITIONS
   - Place here for better organization.             
   ================================================== */

bool checkIfInit(Node * node, Node * parent, SymbolTable * ST);
void checkIfDeclaredAlready(Node * node, SymbolTable * ST);
bool checkIfDeclaredAlreadyGlobal(Node * node, SymbolTable * ST);
bool isSymbolDeclared(Node * node, SymbolTable * ST);
Node * fetchFunction(Node * node, SymbolTable * ST);
Node * fetchSymbol(Node * node, SymbolTable * ST);
bool isScope(Node * node);
bool isSymbol(Node * node);
void printData(Node * node);
void printNothing(Node * node);

/* ================================================== */

#endif