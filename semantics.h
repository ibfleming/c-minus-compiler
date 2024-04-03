#ifndef _SEMANTICS_H
#define _SEMANTICS_H

#include "scope.h"

/* ======================================================================== */
void traverseAST(Node * AST, SymbolTable * ST);
void traverseScope(Node * AST, SymbolTable * ST);
/* ======================================================================== */
void semanticAnalysis(Node * node, SymbolTable * ST);
/* ======================================================================== */

Node * processArray(Node * node, SymbolTable * ST);
Node * processLHS(Node * node, SymbolTable * ST);
Node * processRHS(Node * node, SymbolTable * ST);
Node * processOp(Node * node, SymbolTable * ST); 
Node * processAssign(Node * node, SymbolTable * ST);
void processReturn(Node * node, SymbolTable * ST);
Node * processCall(Node * node, SymbolTable * ST);
void processRange(Node *, SymbolTable *);
void checkPassedParams(Node * node, Node * function, map<int, Node*> passedParms);
Node * processCHSIGN(Node * node, SymbolTable * ST);
void processIf(Node * node, SymbolTable * ST);
void processWhile(Node * node, SymbolTable * ST);
Node * processAND(Node * node, SymbolTable * ST);
Node * processNOT(Node * node, SymbolTable * ST);
Node * processOR(Node * node, SymbolTable * ST);
Node * processQUES(Node *, SymbolTable *);
Node * processSizeOf(Node *, SymbolTable *);
void checkMain(SymbolTable * ST);

//bool checkIfDeclaredScope(Node *, Node*, SymbolTable *);
//bool checkDuplicateScope(Node *, SymbolTable *);         // Check duplicates in Global and current scope!
//bool checkDuplicate(Node *, SymbolTable *);              // Check duplicates in Global and all current scopes!
//bool checkIfInit(Node *, Node *, SymbolTable *);
//Node * fetchSymbol(Node *, SymbolTable *);
//Node * processAssign(Node *, SymbolTable *);
//Node * processOps(Node *, SymbolTable *);
//Node * processLHS(Node *, SymbolTable *);
//Node * processRHS(Node *, SymbolTable *);
//Node * processArray(Node *, SymbolTable *);
//Node * processCHSIGN(Node *, SymbolTable *);
//void printNotArray(Node *, Node *);
//void printArrayLHSRHS(Node *, Node *, Node *);
//void printRequiresDT(Node *, Node *, Node *);
//Node * processAND(Node *, SymbolTable *);
//Node * processOR(Node *, SymbolTable *);
//Node * processNOT(Node *, SymbolTable *);
//Node * processCall(Node *, SymbolTable *);
/* ======================================================================== */

#endif