#include "routines.h"
#include "symbolTable.h"
#include "semantics.h"
using namespace  std;

extern int errs;
extern int warns;

/* ========================================================================
   CREATE THE ROUTINE LIBRARY AST
   ======================================================================== */
Node * createRoutineAST()
{
   Node * rAST;
   Node * tmp;
   rAST = createRoutineNode((char *)"input", FuncNT);
   rAST->dataType = IntDT;
   Node * inputb = createRoutineNode((char *)"inputb", FuncNT);
   inputb->dataType = BoolDT;
   rAST = addSib(rAST, inputb);
   Node * inputc = createRoutineNode((char *)"inputc", FuncNT);
   inputc->dataType = CharDT;
   rAST = addSib(rAST, inputc);
   tmp = createRoutineNode((char *)"output", FuncNT);
   tmp->parmC = 1;
   Node * parm1 = createRoutineNode((char *)"*dummy*", ParmNT);
   parm1->dataType = IntDT;
   parm1->isArray = false;
   parm1->isIndexed = false;
   tmp->ParmList.insert(make_pair(0, parm1));
   tmp = addChild(tmp, parm1);
   rAST = addSib(rAST, tmp);
   tmp = createRoutineNode((char *)"outputb", FuncNT);
   tmp->parmC = 1;
   Node * parm2 = createRoutineNode((char *)"*dummy*", ParmNT);
   parm2->dataType = BoolDT;
   parm2->isArray = false;
   parm2->isIndexed = false;
   tmp->ParmList.insert(make_pair(0, parm2));
   tmp = addChild(tmp, parm2);
   rAST = addSib(rAST, tmp);
   tmp = createRoutineNode((char *)"outputc", FuncNT);
   tmp->parmC = 1;
   Node * parm3 = createRoutineNode((char *)"*dummy*", ParmNT);
   parm3->dataType = CharDT;
   parm3->isArray = false;
   parm3->isIndexed = false;
   tmp->ParmList.insert(make_pair(0, parm3));
   tmp = addChild(tmp, parm3);
   rAST = addSib(rAST, tmp);
   rAST = addSib(rAST, createRoutineNode((char *)"outnl", FuncNT));
   return rAST;
}

/* ========================================================================
   TRAVERSE THE ROUTINE AST AND ADD TO GLOBALS
   ======================================================================== */
SymbolTable traverseRoutineAST(Node * rAST, SymbolTable * ST) {
   if( rAST != NULL ) {
      if( rAST->nodeType == FuncNT ) {
         rAST->isUsed = true;
         ST->insertGlobal(string(rAST->literal), rAST);
      }
      for( int i = 0; i < rAST->childC; i++ ) { traverseRoutineAST(rAST->child[i], ST); }
      traverseRoutineAST(rAST->sibling, ST);
      return *ST;
   }
   return *ST;
}

/* ========================================================================
   SEMANTIC HELPER FUNCTIONS
   ======================================================================== */

/* ==================================================
   CHECK IF SYMBOL IS INITIALIZED!             
   ================================================== */
// THIS CHECKS AND SETS isSymbol() to INIT AND IS USED!
bool checkIfInit(Node * node, Node * parent, SymbolTable * ST) {
   if( node != NULL && parent != NULL) {
      node->isUsed = true;
      if( isSymbol(node)) {
         //printf("init: %s\n", node->literal);
         if( node->isInit == false ) {
            printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", parent->lineNum, node->literal);
            node->isInit = true;
            warns += 1;
            return false;
         }
      }
   }
   return true; 
}
/* ==================================================
   IS SYMBOL DECLARED OR NOT?              
   ================================================== */
// Returns true if it is declared in the program, else false.
bool isSymbolDeclared(Node * node, SymbolTable * ST) {
   bool globalB = true;
   bool localB = true;
   if( node != NULL ) {
      if( node->nodeType == IdNT || node->nodeType == CallNT ) {
         // Check global first!
         Node * global = ST->lookupGlobal(node->literal);
         if( global == NULL ) { globalB = false; }
         Node * local = ST->lookup(node->literal);
         if( local == NULL ) { localB = false; }
         if( globalB == false && localB == false ) {
            printf("ERROR(%d): Symbol \'%s\' is not declared.\n", node->lineNum, node->literal);
            errs += 1;
            return false;
         }
         return true;
      }
   }
   return false;
}
/* ==================================================
   CHECK IF DECLARED              
   ================================================== */
void checkIfDeclaredAlready(Node * node, SymbolTable * ST)
{
   if( isSymbol(node) || isScope(node) ) {
      string symbol = string(node->literal);
      Node * local = ST->lookupScope(symbol);
      if( local == NULL ) {
         return;
      }
      if ( local != NULL ) {
         printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", node->lineNum, symbol.c_str(), local->lineNum);
         errs += 1;
      }
   }
}

/* ==================================================
   CHECK IF DECLARED GLOBAL                 
   ================================================== */
bool checkIfDeclaredAlreadyGlobal(Node * node, SymbolTable * ST)
{
   if( node != NULL ) {
      if( isSymbol(node) || isScope(node) ) {
         string symbol = string(node->literal);
         Node * global = ST->lookupGlobal(symbol);
         if( global == NULL ) {
            return false;
         }
         if ( global != NULL ) {
            printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", node->lineNum, symbol.c_str(), global->lineNum);
            errs += 1;
            return true;
         }
      }
   }
   return false;
}

/* ==================================================
   FETCH FUNCTION NODE                 
   ================================================== */
Node * fetchFunction(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * function = ST->lookupGlobal(node->literal);
      if( function != NULL ) { 
         node->dataType = function->dataType; 
         return function;
      }
   }
   return NULL;
}

/* ==================================================
   FETCH SYMBOL NODE                  
   ================================================== */
Node * fetchSymbol(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * symbol = ST->lookup(node->literal);
      if( symbol != NULL ) {
         node->dataType = symbol->dataType;
         return symbol;
      } else {
         Node * global = ST->lookupGlobal(node->literal);
         if( global != NULL ) {
            node->dataType = global->dataType;
            return global;
         }
      }
   }
   return NULL;
}
/* ==================================================
   IS NODE A SCOPE?                  
   ================================================== */
bool isScope(Node * node) {
   switch ( node->nodeType )
   {
   case FuncNT:
      return true;
   case CompoundNT:
      return true;
   case ToNT:
      return true;
   default:
      break;
   }
   return false;
}

/* ==================================================
   IS NODE A SYMBOL?                  
   ================================================== */
bool isSymbol(Node * node) {
   switch ( node->nodeType )
   {
   case VarNT:
      return true;
   case VarArrNT:
      return true;
   case ParmNT:
      node->isInit = true;
      return true;
   case ParmArrNT:
      node->isInit = true;
      return true;
   case StaticNT:
      node->isInit = true;
      return true;
   default:
      break;
   }
   return false;
}
/* ==================================================
   PRINT FUNCTIONS                 
   ================================================== */
void printData(Node * node) {
   printf("%s, %s", convertNTStr(node), convertDTStr(node));
}

void printNothing(Node * node) {} 