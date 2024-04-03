#ifndef _ROUTINES_H
#define _ROUTINES_H

#include "scope.hpp"

/* ==================================================
   ROUTINE I/O LIBRARY DEFITIONS
   ================================================== */

Node *createRoutineAST(Node *AST);
SymbolTable traverseRoutineAST(Node *rAST, SymbolTable *ST);

/* ==================================================
   SEMANTIC HELPER DEFINITIONS
   - Placed here for better organization.
   ================================================== */

bool checkIfInit(Node *node, Node *parent, SymbolTable *ST);
void checkIfDeclaredAlready(Node *node, SymbolTable *ST);
bool checkIfDeclaredAlreadyGlobal(Node *node, SymbolTable *ST);
bool isSymbolDeclared(Node *node, SymbolTable *ST);
Node *fetchFunction(Node *node, SymbolTable *ST);
Node *fetchSymbol(Node *node, SymbolTable *ST);
void exchangeNodeData(Node *receiver, Node *parent);
bool isScope(Node *node);
bool isSymbol(Node *node);
bool isConstant(Node *node);
void printData(Node *node);
void printNothing(Node *node);

/* ================================================== */

#endif