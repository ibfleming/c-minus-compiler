#ifndef _SEMANTICS_H
#define _SEMANTICS_H

#include "scope.h"

void semanticAnalysis(Node *, SymbolTable *);
void printNothing(Node *);
void printData(Node *);
bool isScope(Node *);
bool isSymbol(Node *);
bool checkIfDeclared(Node *, SymbolTable *, bool print);
bool checkIfDeclaredScope(Node *, Node*, SymbolTable *);
bool checkDuplicate(Node *, SymbolTable *);              // Check duplicates in Global and all current scopes!
bool checkDuplicateScope(Node *, SymbolTable *);         // Check duplicates in Global and current scope!
bool checkIfInit(Node *, Node *, SymbolTable *);
Node * fetchSymbol(Node *, SymbolTable *);
Node * processAssign(Node *, SymbolTable *);
Node * processOps(Node *, SymbolTable *);
Node * processLHS(Node *, SymbolTable *);
Node * processRHS(Node *, SymbolTable *);
Node * processArray(Node *, SymbolTable *);
Node * processCHSIGN(Node *, SymbolTable *);
void printNotArray(Node *, Node *);
void printArrayLHSRHS(Node *, Node *, Node *);
void printRequiresDT(Node *, Node *, Node *);
Node * processAND(Node *, SymbolTable *);
Node * processOR(Node *, SymbolTable *);
Node * processNOT(Node *, SymbolTable *);
Node * processCall(Node *, SymbolTable *);

#endif