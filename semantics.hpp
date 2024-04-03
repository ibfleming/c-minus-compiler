#ifndef _SEMANTICS_H
#define _SEMANTICS_H

#include "scope.hpp"

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
Node * processCHSIGN(Node * node, SymbolTable * ST);
void processIf(Node * node, SymbolTable * ST);
void processWhile(Node * node, SymbolTable * ST);
Node * processAND(Node * node, SymbolTable * ST);
Node * processNOT(Node * node, SymbolTable * ST);
Node * processOR(Node * node, SymbolTable * ST);
Node * processQUES(Node *, SymbolTable *);
Node * processSizeOf(Node *, SymbolTable *);

void is_this_func_main(Node *, SymbolTable *);
void checkInitializers(Node *, SymbolTable *);
void checkMain(SymbolTable *);
void checkPassedParams(Node *, Node *, map<int, Node*>);

/* ======================================================================== */

#endif