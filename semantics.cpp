/* ########################################################################
   semantics.cpp

   This "large" file is responsible for traversing through the program's
   abstract syntax tree detecting any semantic errors and warnings.

   ######################################################################## */

/* ========================================================================
   INCLUDES
   ======================================================================== */
#include "semantics.hpp"
#include "scope.hpp"
#include "routines.hpp"
#include "parser.tab.h"
using namespace std;

/* ========================================================================
   IMPORTANT VARIABLES
   ======================================================================== */
extern int warns; // Warning counter, initialized to 0
extern int errs;  // Error counter, initialized to 0
extern int goffset;
extern int foffset;
int old_foffset = 0;
/* ========================================================================
   GLOBALS
   ======================================================================== */
Node *functionScope = NULL; // Set to the function node after insertion.
Node *function_compound = NULL;
Node *embedded_compound_parent = NULL;
Node *embedded_for_parent = NULL;
bool inFunction = false;          // Are we in a function?
bool hasMain = false;             // Does this program have a main()?
bool check_for_bad_breaks = true; // Check for breaks outside of loops.
bool return_exists = false;       // If there's a return node in the scope.
int nStmts = 0;                   // Counter for compound statements we are in.
int fStmts = 0;                   // Counter for functions.
/* ========================================================================
   (1) [NEW] TRAVERSAL THROUGH THE AST ENCOUNTERING ONLY GLOBAL OBJECTS
   ======================================================================== */
void traverseAST(Node *AST, SymbolTable *ST)
{
   /* ========================================================================
      Depending on the node type, do something...
      ======================================================================== */
   if (AST != NULL)
   {
      /* ==================================================
         FUNCTIONS
         ================================================== */
      if (AST->nodeType == FuncNT)
      {
         functionScope = AST;
         function_compound = NULL;
         embedded_compound_parent = NULL;
         embedded_for_parent = NULL;
         check_for_bad_breaks = true;

         // MEMORY LOCATIONS/SIZING
         foffset = 0;
         old_foffset = 0;
         AST->size = -2;
         foffset += AST->size;
         for (Node *itr = AST->child[0]; itr != NULL; itr = itr->sibling)
         {
            AST->size -= itr->size;
         }
         //

         if (AST->dataType != VoidDT)
         {
            functionScope->hasReturn = true;
         }

         is_this_func_main(AST, ST);
         if (string(AST->literal).compare("main") != 0)
         {
            checkIfDeclaredAlreadyGlobal(AST, ST);
         }

         nStmts = 0;
         fStmts += 1;
         ST->insertGlobal(string(AST->literal), AST);
         ST->enter(string(AST->literal));
         traverseScope(AST, ST);

         if (functionScope->hasReturn == true)
         {
            if (return_exists == false)
            {
               printf("WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n",
                      AST->lineNum, convertDTStr(functionScope), functionScope->literal);
               warns += 1;
            }
         }
         return_exists = false;
      }

      /* ==================================================
         GLOBAL SYMBOL DECLARATIONS
         ================================================== */
      if (isSymbol(AST) && AST->isVisited == false)
      {
         AST->isVisited = true;
         AST->isInit = true;

         if (AST->nodeType == VarNT || AST->nodeType == VarArrNT)
         {
            AST->refType = GlobalRT;
            Node *init_val = AST->child[0];
            if (init_val != NULL)
            {
               if (init_val->nodeType == StringConstNT)
               {
                  init_val->location = goffset - 1;
                  goffset -= init_val->size;
               }
               AST->location = goffset;
               if (AST->isArray == true)
                  AST->location -= 1;
               goffset -= AST->size;
            }
            else
            {
               AST->location = goffset;
               if (AST->isArray == true)
                  AST->location -= 1;
               goffset -= AST->size;
            }
         }
         else if (AST->nodeType == StaticNT)
         {
            Node *init_val = AST->child[0];
            if (init_val != NULL)
            {
               if (init_val->nodeType == StringConstNT)
               {
                  init_val->location = goffset - 1;
                  goffset -= init_val->size;
               }
               AST->location = goffset;
               if (AST->isArray == true)
                  AST->location -= 1;
               goffset -= AST->size;
            }
            else
            {
               AST->location = goffset;
               if (AST->isArray == true)
                  AST->location -= 1;
               goffset -= AST->size;
            }
         }

         if (checkIfDeclaredAlreadyGlobal(AST, ST) == false)
         {
            ST->insert(string(AST->literal), AST);
         }
         checkInitializers(AST, ST);
      }
      /* ==================================================
         GLOBAL CONSTANTS
         ================================================== */

      /* ========================================================================
         Traverse through the AST...
         ======================================================================== */
      for (int i = 0; i < AST->childC; i++)
      {
         traverseAST(AST->child[i], ST);
      }
      traverseAST(AST->sibling, ST);

      /* ======================================================================== */
   }
}
/* ========================================================================
   (2) TRAVERSE THROUGH THE CURRENT SCOPE ONLY
   ======================================================================== */
void traverseScope(Node *AST, SymbolTable *ST)
{
   /* ========================================================================
      Depending on the node type, do something...
      ======================================================================== */
   if (AST != NULL)
   {
      /* ==================================================
         COMPOUND STATEMENTS (BEGIN...END)
         ================================================== */
      if (AST->nodeType == CompoundNT)
      {
         if (nStmts == 0)
         {
            if (function_compound == NULL)
            {
               function_compound = AST;
            }
            nStmts = 1;
            if (check_for_bad_breaks == true)
            {
               Node *tgt = AST->child[AST->childC - 1];
               if (tgt != NULL)
               {
                  if (tgt->nodeType == BreakNT)
                  {
                     printf("ERROR(%d): Cannot have a break statement outside of loop.\n", tgt->lineNum);
                     errs += 1;
                  }
                  else
                  {
                     Node *tmp = tgt->sibling;
                     while (tmp != NULL)
                     {
                        if (tmp->nodeType == BreakNT)
                        {
                           printf("ERROR(%d): Cannot have a break statement outside of loop.\n", tmp->lineNum);
                           errs += 1;
                        }
                        tmp = tmp->sibling;
                     }
                  }
                  check_for_bad_breaks = false;
               }
            }
            AST->size = foffset;
            for (Node *itr = AST->child[0]; itr != NULL; itr = itr->sibling)
            {
               if (itr->nodeType == VarNT || itr->nodeType == VarArrNT)
               {
                  AST->size -= itr->size;
               }
            }
         }
         else if (nStmts == 1)
         {
            if (embedded_compound_parent == NULL && embedded_for_parent == NULL)
            {
               // First embedded_compound_parent
               embedded_compound_parent = AST;
               AST->size = foffset;
               for (Node *itr = AST->child[0]; itr != NULL; itr = itr->sibling)
               {
                  if (itr->nodeType == VarNT || itr->nodeType == VarArrNT)
                  {
                     AST->size -= itr->size;
                  }
               }
               old_foffset = foffset;
            }
            else
            {
               AST->size = foffset;
               for (Node *itr = AST->child[0]; itr != NULL; itr = itr->sibling)
               {
                  if (itr->nodeType == VarNT || itr->nodeType == VarArrNT)
                  {
                     AST->size -= itr->size;
                  }
               }
            }
            ST->enter("Compound");
         }
      }
      /* ==================================================
         FOR LOOPS
         ================================================== */
      else if (AST->nodeType == ToNT)
      {
         if (embedded_for_parent == NULL)
         {
            // printf("for parent entered: %d\n", AST->lineNum);
            embedded_for_parent = AST;
            AST->size = foffset;
            for (Node *itr = AST->child[0]; itr != NULL; itr = itr->sibling)
            {
               if (itr->nodeType == VarNT || itr->nodeType == VarArrNT)
               {
                  AST->size -= itr->size;
               }
            }
            old_foffset = foffset;
            for (int i = 0; i < AST->childC; i++)
            {
               if (AST->child[i] != NULL)
               {
                  if (AST->child[i]->nodeType == CompoundNT)
                  {
                     nStmts = 0;
                  }
               }
            }
         }
         else
         {
            AST->size = foffset;
            for (Node *itr = AST->child[0]; itr != NULL; itr = itr->sibling)
            {
               if (itr->nodeType == VarNT || itr->nodeType == VarArrNT)
               {
                  AST->size -= itr->size;
               }
            }
            for (int i = 0; i < AST->childC; i++)
            {
               if (AST->child[i] != NULL)
               {
                  if (AST->child[i]->nodeType == CompoundNT)
                  {
                     AST->isVisited = true;
                     nStmts = 0;
                  }
               }
            }
         }
         ST->enter("For");
      }
      /* ==================================================
         LOCAL SYMBOL DECLARATIONS
         ================================================== */
      else if (isSymbol(AST))
      {
         AST->isVisited = true;

         if (AST->nodeType == VarNT || AST->nodeType == VarArrNT)
         {
            AST->refType = LocalRT;
            Node *init_val = AST->child[0];
            if (init_val != NULL)
            {
               if (init_val->nodeType == StringConstNT)
               {
                  init_val->isAddressed = true;
                  init_val->location = goffset - 1;
                  goffset -= init_val->size;
               }
               AST->location = foffset;
               if (AST->isArray == true)
               {
                  AST->location -= 1;
               }
               foffset -= AST->size;
            }
            else
            {
               AST->location = foffset;
               if (AST->isArray == true)
               {
                  AST->location -= 1;
               }
               foffset -= AST->size;
            }
         }
         else if (AST->nodeType == ParmNT || AST->nodeType == ParmArrNT)
         {
            AST->location = foffset;
            foffset -= AST->size;
            if (functionScope != NULL)
            {
               functionScope->ParmList.insert(make_pair(functionScope->parmC, AST));
               functionScope->parmC++;
            }
         }
         else if (AST->nodeType == StaticNT)
         {
            Node *init_val = AST->child[0];
            if (init_val != NULL)
            {
               if (init_val->nodeType == StringConstNT)
               {
                  init_val->isAddressed = true;
                  init_val->location = goffset - 1;
                  goffset -= init_val->size;
               }
               AST->location = goffset;
               if (AST->isArray == true)
                  AST->location -= 1;
               goffset -= AST->size;
            }
            else
            {
               AST->location = goffset;
               if (AST->isArray == true)
                  AST->location -= 1;
               goffset -= AST->size;
            }
         }

         checkIfDeclaredAlready(AST, ST);
         ST->insert(string(AST->literal), AST);
         checkInitializers(AST, ST);
      }
      /* ==================================================
         CONSTANTS
         ================================================== */
      if (AST->nodeType == StringConstNT && AST->isAddressed == false)
      {
         AST->isAddressed = true;
         AST->location = goffset - 1;
         goffset -= AST->size;
      }
      /* ==================================================
         SUMMON SEMANTIC ANALYSIS!!!
         ================================================== */
      else
      {
         semanticAnalysis(AST, ST);
      }

      /* ========================================================================
         Traverse through the scope...
         ======================================================================== */
      for (int i = 0; i < AST->childC; i++)
      {
         traverseScope(AST->child[i], ST);
      }

      if (AST->nodeType == FuncNT)
      {
         if (ST->depth() == 2)
         {
            ST->checkUnusedVars();
            ST->leave();
         }
         return;
      }
      else
      {
         if (AST->sibling != NULL)
         {
            if (AST->nodeType == ToNT)
            {
               if (ST->depth() > 2)
               {
                  ST->checkUnusedVars();
                  ST->leave();
                  if (AST == embedded_for_parent)
                  {
                     embedded_for_parent = NULL;
                     foffset = old_foffset;
                  }
               }
               else
               {
                  if (AST == embedded_for_parent)
                  {
                     embedded_for_parent = NULL;
                     foffset = old_foffset;
                  }
               }
               traverseScope(AST->sibling, ST);
            }
            else if (AST->nodeType == CompoundNT)
            {
               if (ST->depth() > 2)
               {
                  ST->checkUnusedVars();
                  ST->leave();
                  if (AST == embedded_compound_parent)
                  {
                     embedded_compound_parent = NULL;
                     foffset = old_foffset;
                  }
                  else if (AST->sibling->nodeType == CompoundNT)
                  {
                     foffset += AST->sibling->size;
                  }
               }
               traverseScope(AST->sibling, ST);
            }
            else
            {
               traverseScope(AST->sibling, ST);
            }
         }
         else if (AST->sibling == NULL)
         {
            if (AST->nodeType == ToNT)
            {
               if (ST->depth() > 2)
               {
                  ST->checkUnusedVars();
                  ST->leave();
                  if (AST == embedded_for_parent)
                  {
                     embedded_for_parent = NULL;
                     foffset = old_foffset;
                  }
                  else
                  {
                     foffset = old_foffset;
                  }
               }
            }
            else if (AST->nodeType == CompoundNT)
            {
               if (ST->depth() > 2)
               {
                  ST->checkUnusedVars();
                  ST->leave();
                  if (AST == embedded_compound_parent)
                  {
                     if (embedded_for_parent != NULL)
                     {
                        embedded_for_parent = NULL;
                     }
                     embedded_compound_parent = NULL;
                     foffset = old_foffset;
                  }
               }
            }
            else if (AST->nodeType == FuncNT)
            {
               ST->checkUnusedVars();
               ST->leave();
               return;
            }
         }
      }
      /* ======================================================================== */
   }
}
/* ========================================================================
   SEMANTIC ANALYSIS (Don't think a switch-case will work here)
   ======================================================================== */
void semanticAnalysis(Node *node, SymbolTable *ST)
{
   if (node->nodeType == IdNT)
   {
      if (node->isVisited == false)
      {
         node->isVisited = true;
         if (isSymbolDeclared(node, ST))
         {
            Node *id = fetchSymbol(node, ST);
            if (id != NULL)
            {
               if (id->isStatic == true)
                  id->isInit = true;
               checkIfInit(id, node, ST);
               id->isUsed = true;
            }
         }
      }
   }
   if (node->nodeType == ArrNT)
   {
      if (node->isVisited == false)
      {
         node->isVisited = true;
         Node *id = processArray(node, ST);
         if (id != NULL)
         {
            checkIfInit(id, node, ST);
            id->isUsed = true;
            id->isIndexed = false;
         }
      }
   }
   if (node->nodeType == ReturnNT && node->isVisited == false)
   {
      node->isVisited = true;
      processReturn(node, ST);
   }
   if (node->nodeType == CallNT && node->isVisited == false)
   {
      processCall(node, ST);
   }
   if (node->nodeType == AssignNT && node->isVisited == false)
   {
      processAssign(node, ST);
   }
   if (node->nodeType == OpNT && node->isVisited == false)
   {
      processOp(node, ST);
   }
   if (node->nodeType == SignNT && node->isVisited == false)
   {
      processCHSIGN(node, ST);
   }
   if (node->nodeType == IfNT)
   {
      processIf(node, ST);
   }
   if (node->nodeType == IterNT && node->isVisited == false)
   {
      processWhile(node, ST);
   }
   if (node->nodeType == QuesNT)
   {
      processQUES(node, ST);
   }
   if (node->nodeType == AndNT && node->isVisited == false)
   {
      processAND(node, ST);
   }
   if (node->nodeType == SizeOfNT)
   {
      processSizeOf(node, ST);
   }
   if (node->nodeType == NotNT && node->isVisited == false)
   {
      processNOT(node, ST);
   }
   if (node->nodeType == OrNT && node->isVisited == false)
   {
      processOR(node, ST);
   }
   if (node->nodeType == RangeNT && node->isVisited == false)
   {
      processRange(node, ST);
   }
}
/* ========================================================================
   SEMANTIC ANALYSIS FUNCTIONS
   ======================================================================== */
/* ==================================================
   PROCESS CHSIGN
   ================================================== */
Node *processCHSIGN(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      node->isVisited = true;
      Node *id = node->child[0];
      if (id != NULL)
      {
         if (id->isConst == true)
         {
            if (id->dataType != IntDT)
            {
               printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                      node->lineNum, node->literal, convertDTStr(id));
               errs += 1;
            }
         }
         if (id->nodeType == IdNT)
         {
            id->isVisited = true;
            Node *sym = processLHS(id, ST);
            if (sym != NULL)
            {
               sym->isUsed == true;
               checkIfInit(sym, node, ST);
               sym->isInit = true;
               if (sym->isArray == true)
               {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if (sym->dataType != IntDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS ARRAY
   ================================================== */
Node *processArray(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *id = node->child[0];
      Node *index = node->child[1];

      Node *symbol;
      if (id != NULL)
      {
         id->isVisited = true;
         isSymbolDeclared(id, ST);
         symbol = fetchSymbol(id, ST);
         if (symbol != NULL)
         {
            checkIfInit(symbol, node, ST);
            node->dataType = symbol->dataType;
            if (symbol->isArray == false)
            {
               printf("ERROR(%d): Cannot index nonarray \'%s\'.\n", node->lineNum, symbol->literal);
               errs += 1;
            }
            if (symbol->nodeType == FuncNT)
            {
               printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, symbol->literal);
               errs += 1;
            }
         }
         else
         {
            printf("ERROR(%d): Cannot index nonarray \'%s\'.\n", node->lineNum, id->literal);
            errs += 1;
         }
      }
      if (index != NULL)
      {
         if (symbol != NULL)
            symbol->isIndexed = true;

         if (index->isConst == true)
         {
            if (index->dataType != IntDT)
            {
               printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(index));
               errs += 1;
            }
         }
         else if (index->nodeType == IdNT)
         {
            index->isVisited = true;
            Node *idx = processLHS(index, ST);
            if (idx != NULL)
            {
               idx->isUsed = true;
               idx->isInit = true;
               if (idx->dataType != IntDT && idx->nodeType != FuncNT)
               {
                  printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(idx));
                  errs += 1;
               }
               if (idx->isArray == true)
               {
                  printf("ERROR(%d): Array index is the unindexed array '%s'.\n", node->lineNum, idx->literal);
                  errs += 1;
               }
               if (idx->nodeType == FuncNT)
               {
                  printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, idx->literal);
                  errs += 1;
               }
            }
         }
         else if (index->nodeType == ArrNT)
         {
            index->isVisited = true;
            if (index->child[0] != NULL)
            {
               Node *tmp = fetchSymbol(index->child[0], ST);
               if (tmp != NULL)
               {
                  tmp->isInit = true;
               }
            }
            Node *array = processArray(index, ST);
            if (array != NULL)
            {
               array->isUsed = true;
               if (array->dataType != IntDT)
               {
                  printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(array));
                  errs += 1;
               }
            }
         }
         else if (index->nodeType == OpNT)
         {
            index->isVisited = true;
            processOp(index, ST);
         }
         else if (index->nodeType == CallNT)
         {
            index->isVisited = true;
            Node *call = processCall(index, ST);
            if (call != NULL)
            {
               call->isUsed = true;
               if (call->dataType != IntDT)
               {
                  printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(call));
                  errs += 1;
               }
            }
         }
      }
      if (symbol != NULL)
         return symbol;
   }
   return NULL;
}
/* ==================================================
   PROCESS LHS
   ================================================== */
Node *processLHS(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      if (node->isConst == true)
      {
         return node;
      }
      else if (node->nodeType == IdNT)
      {
         if (isSymbolDeclared(node, ST))
         {
            Node *lsym = fetchSymbol(node, ST);
            return lsym;
         }
      }
      else if (node->nodeType == ArrNT)
      {
         Node *id = node->child[0];
         if (id != NULL)
         {
            id->isVisited = true;
            Node *symbol = fetchSymbol(id, ST);
            if (symbol != NULL)
            {
               symbol->isInit = true;
            }
         }
         Node *lsym = processArray(node, ST);
         return lsym;
      }
      else if (node->nodeType == CallNT)
      {
         Node *lsym = processCall(node, ST);
         return lsym;
      }
      else if (node->nodeType == OpNT)
      {
         Node *lsym = processOp(node, ST);
         return lsym;
      }
      else if (node->nodeType == SizeOfNT)
      {
         Node *lsym = processSizeOf(node, ST);
         return lsym;
      }
      else if (node->nodeType == AndNT)
      {
         Node *lsym = processAND(node, ST);
         return lsym;
      }
      else if (node->nodeType == OrNT)
      {
         Node *lsym = processOR(node, ST);
         return lsym;
      }
      else if (node->nodeType == NotNT)
      {
         Node *rsym = processNOT(node, ST);
         return rsym;
      }
   }
   return NULL;
}

/* ==================================================
   PROCESS RHS
   ================================================== */
Node *processRHS(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      if (node->isConst == true)
      {
         return node;
      }
      else if (node->nodeType == IdNT)
      {
         if (isSymbolDeclared(node, ST))
         {
            Node *rsym = fetchSymbol(node, ST);
            if (rsym->nodeType == FuncNT)
            {
               printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, rsym->literal);
               errs += 1;
            }
            return rsym;
         }
      }
      else if (node->nodeType == ArrNT)
      {
         Node *rsym = processArray(node, ST);
         return rsym;
      }
      else if (node->nodeType == CallNT)
      {
         Node *rsym = processCall(node, ST);
         return rsym;
      }
      else if (node->nodeType == OpNT)
      {
         node->isVisited = true;
         Node *rsym = processOp(node, ST);
         return rsym;
      }
      else if (node->nodeType == AndNT)
      {
         Node *rsym = processAND(node, ST);
         return rsym;
      }
      else if (node->nodeType == OrNT)
      {
         Node *rsym = processOR(node, ST);
         return rsym;
      }
      else if (node->nodeType == AssignNT)
      {
         Node *rsym = processAssign(node, ST);
         return rsym;
      }
      else if (node->nodeType == NotNT)
      {
         Node *rsym = processNOT(node, ST);
         return rsym;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS OPERATOR
   ================================================== */
Node *processOp(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *lhs = node->child[0];
      Node *rhs = node->child[1];
      Node *lsym = NULL;
      Node *rsym = NULL;

      if (node->tknClass == LESS || node->tknClass == LEQ || node->tknClass == GREAT ||
          node->tknClass == GEQ || node->tknClass == EQL || node->tknClass == NEQ)
      {
         // PROCESS LHS!
         if (lhs != NULL)
         {
            lhs->isVisited = true;
            lsym = processRHS(lhs, ST);
            if (lsym != NULL)
            {
               checkIfInit(lsym, node, ST);
            }
         }
         // PROCESS RHS
         if (rhs != NULL)
         {
            rhs->isVisited = true;
            if (rhs->nodeType == AssignNT)
            {
               rhs->isVisited = true;
               rsym = processAssign(rhs, ST);
            }
            else
            {
               rsym = processRHS(rhs, ST);
               if (rsym != NULL)
               {
                  rsym->isUsed = true;
               }
            }
            if (rsym != NULL)
            {
               checkIfInit(rsym, node, ST);
            }
         }
         // ERROR CHECK!
         if (lsym != NULL && rsym != NULL)
         {
            if (lsym->dataType != rsym->dataType)
            {
               printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n",
                      node->lineNum, node->literal, convertDTStr(lsym), convertDTStr(rsym));
               errs += 1;
            }
            if (lsym->isArray == true && lsym->isIndexed == false)
            {
               if (rsym->isArray == false && rsym->isConst == false)
               {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n",
                         node->lineNum, node->literal);
                  errs += 1;
               }
            }
            if (lsym->isArray == false)
            {
               if (rhs->nodeType != ArrNT && rsym->isArray == true)
               {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n",
                         node->lineNum, node->literal);
                  errs += 1;
               }
            }
            Node *ret = createNode(NULL, RET_SYM);
            ret->dataType = BoolDT;
            return ret;
         }
      }
      else if (node->tknClass == ADD || node->tknClass == SUB || node->tknClass == MUL ||
               node->tknClass == DIV || node->tknClass == MOD)
      {
         // PROCESS LHS!
         if (lhs != NULL)
         {
            lhs->isVisited = true;
            lsym = processRHS(lhs, ST);
            if (lsym != NULL)
            {
               checkIfInit(lsym, node, ST);
            }
         }
         // PROCESS RHS
         if (rhs != NULL)
         {
            rhs->isVisited = true;
            if (rhs->nodeType == AssignNT)
            {
               rhs->isVisited = true;
               rsym = processAssign(rhs, ST);
            }
            else
            {
               rsym = processRHS(rhs, ST);
               if (rsym != NULL)
               {
                  rsym->isUsed = true;
                  checkIfInit(rsym, node, ST);
                  rsym->isInit = true;
               }
            }
         }
         if (lsym != NULL)
         {
            if (lsym->dataType != IntDT)
            {
               printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
         if (rsym != NULL)
         {
            if (rsym->dataType != IntDT)
            {
               printf("ERROR(%d): '%s' requires operands of type int but rhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
         if (lsym != NULL && rsym != NULL)
         {
            if ((lhs->nodeType != ArrNT && lsym->isArray == true) || (rhs->nodeType != ArrNT && rsym->isArray == true))
            {
               printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
               errs += 1;
            }
         }
         Node *ret = createNode(NULL, RET_SYM);
         ret->dataType = IntDT;
         return ret;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS ASSIGNMENT
   ================================================== */
Node *processAssign(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *lhs = node->child[0];
      Node *rhs = node->child[1];
      Node *lsym = NULL;
      Node *rsym = NULL;

      /* ==================================================
         ASSIGNMENT <=
         ================================================== */
      if (node->tknClass == ASGN)
      {
         // PROCESS LHS
         if (lhs != NULL)
         {
            lhs->isVisited = true;
            lsym = processLHS(lhs, ST);
            if (lsym != NULL)
            {
               if (string(lhs->literal).compare(rhs->literal) == 0)
               {
                  checkIfInit(lsym, node, ST);
               } // i <= i; (allbad.c-)
               lsym->isUsed = true;
               node->dataType = lsym->dataType;
               if (lsym->nodeType == FuncNT)
               {
                  printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, lsym->literal);
                  errs += 1;
                  return lsym;
               }
            }
         }
         // PROCESS RHS
         if (rhs != NULL)
         {
            rhs->isVisited = true;
            if (rhs->nodeType == AssignNT)
            {
               rsym = processAssign(rhs, ST);
            }
            else
            {
               rsym = processRHS(rhs, ST);
               if (rsym != NULL)
               {
                  rsym->isUsed = true;
                  checkIfInit(rsym, node, ST);
                  if (rhs->nodeType != CallNT && rsym->nodeType == FuncNT)
                  {
                     return lsym;
                  }
               }
            }
         }
         if (lsym != NULL)
            lsym->isInit = true;
         // ERROR CHECK!
         if (lsym != NULL && rsym != NULL)
         {
            if (lsym->isArray == true && lhs->nodeType == IdNT)
            {
               if (rsym->isArray == false)
               {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               else if (rhs->nodeType == ArrNT)
               {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
            if (lsym->isArray == false)
            {
               if (rsym->isArray == true && (rhs->nodeType == IdNT || rhs->nodeType == StringConstNT))
               {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
            if (lsym->dataType != rsym->dataType)
            {
               printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n",
                      node->lineNum, node->literal, convertDTStr(lsym), convertDTStr(rsym));
               errs += 1;
            }

            if (lhs->isArray == true && lhs->nodeType != ArrNT)
            {
               node->isArray = true;
            }
            if (lsym->isArray == true)
               lsym->isIndexed = false;
            return lsym;
         }
      }
      /* ==================================================
         +=. -=, *=, /=
         ================================================== */
      if (node->tknClass == ADDASS || node->tknClass == SUBASS ||
          node->tknClass == MULASS || node->tknClass == DIVASS)
      {
         if (lhs != NULL)
         {
            lhs->isVisited = true;
            lsym = processLHS(lhs, ST);
            if (lsym != NULL)
            {
               lsym->isUsed = true;
               lsym->isInit = true;
               node->dataType = lsym->dataType;
            }
         }
         if (rhs != NULL)
         {
            rhs->isVisited = true;
            if (rhs->nodeType == AssignNT)
            {
               rsym = processAssign(rhs, ST);
            }
            else if (rhs->nodeType == OpNT)
            {
               rsym = processOp(rhs, ST);
            }
            else
            {
               rsym = processRHS(rhs, ST);
               if (rsym != NULL)
               {
                  rsym->isUsed = true;
                  checkIfInit(rsym, node, ST);
               }
            }
         }

         if (lsym != NULL)
         {
            if (lsym->dataType != IntDT)
            {
               printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
         if (rsym != NULL)
         {
            if (rsym->dataType != IntDT)
            {
               printf("ERROR(%d): '%s' requires operands of type int but rhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
         if (lsym != NULL && rsym != NULL)
         {
            if (lsym->isArray == true)
            {
               if (lhs->nodeType != ArrNT)
               {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
            else if (rsym->isArray == true)
            {
               if (rhs->nodeType != ArrNT)
               {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
         }
         return lsym;
      }
      /* ==================================================
         ++, --
         ================================================== */
      if (node->tknClass == INC || node->tknClass == DEC)
      {
         if (lhs != NULL)
         {
            lhs->isVisited = true;
            if (lhs->nodeType == ArrNT)
            {
               if (lhs->child[1]->nodeType == IdNT)
               {
                  Node *tmp = fetchSymbol(lhs->child[1], ST);
                  if (tmp != NULL)
                  {
                     checkIfInit(tmp, node, ST);
                  }
               }
            }
            lsym = processRHS(lhs, ST);
            if (lsym != NULL)
            {
               lsym->isUsed = true;
               checkIfInit(lsym, node, ST);
               if (lhs->nodeType != ArrNT && lsym->isArray == true)
               {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if (lsym->dataType != IntDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(lsym));
                  errs += 1;
               }
            }
            else
            {
               return NULL;
            }
            return lsym;
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS RETURN
   ================================================== */
void processReturn(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      return_exists = true;
      if (functionScope != NULL)
      {
         if (functionScope->hasReturn == true)
         {
            if (node->childC == 0)
            {
               printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but return has no value.\n",
                      node->lineNum, functionScope->literal, functionScope->lineNum, convertDTStr(functionScope));
               errs += 1;
               return;
            }
            else
            {
               for (int i = 0; i < node->childC; i++)
               {
                  Node *val = node->child[i];
                  if (val != NULL)
                  {
                     if (val->isConst == true)
                     {
                        if (functionScope->dataType != node->child[i]->dataType)
                        {
                           printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but returns type %s.\n",
                                  node->lineNum, functionScope->literal, functionScope->lineNum, convertDTStr(functionScope), convertDTStr(val));
                           errs += 1;
                        }
                        if (val->isArray == true)
                        {
                           printf("ERROR(%d): Cannot return an array.\n", node->lineNum);
                           errs += 1;
                        }
                     }
                     else if (val->nodeType == IdNT)
                     {
                        val->isVisited = true;
                        Node *id = fetchSymbol(val, ST);
                        checkIfInit(id, node, ST);
                        if (id != NULL)
                        {
                           id->isUsed = true;
                           if (functionScope->dataType != id->dataType)
                           {
                              printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but returns type %s.\n",
                                     node->lineNum, functionScope->literal, functionScope->lineNum, convertDTStr(functionScope), convertDTStr(val));
                              errs += 1;
                           }
                           if (id->isArray == true)
                           {
                              printf("ERROR(%d): Cannot return an array.\n", node->lineNum);
                              errs += 1;
                           }
                        }
                        else
                        {
                           isSymbolDeclared(val, ST);
                        }
                     }
                     else if (val->nodeType == CallNT)
                     {
                        val->isVisited = true;
                        Node *call = processCall(val, ST);
                        if (call != NULL)
                        {
                           call->isUsed = true;
                           //
                        }
                     }
                     else if (val->nodeType == AssignNT)
                     {
                        val->isVisited = true;
                        Node *id = processAssign(val, ST);
                        if (id != NULL)
                        {
                           if (id->isArray == true)
                           {
                              printf("ERROR(%d): Cannot return an array.\n", node->lineNum);
                              errs += 1;
                           }
                        }
                     }
                  }
               }
               return;
            }
         }
         else if (node->childC >= 1)
         {
            printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has a value.\n",
                   node->lineNum, functionScope->literal, functionScope->lineNum);
            errs += 1;
            return;
         }
      }
   }
}
/* ==================================================
   PROCESS CALL
   ================================================== */
Node *processCall(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      node->isVisited = true;
      int parmCount = 0;
      map<int, Node *> passedParms;
      isSymbolDeclared(node, ST);
      Node *function = fetchFunction(node, ST);
      if (function != NULL)
      {
         if (function->nodeType != FuncNT)
         {
            function->isUsed = true;
            printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", node->lineNum, function->literal);
            errs += 1;
            return function;
         }
         function->isUsed = true;
         if (node->childC != 0)
         {
            Node *parm = node->child[0];
            while (parm != NULL)
            {
               if (parm->isConst == true)
               {
                  parm->isVisited = true;
                  passedParms.insert(make_pair(parmCount, parm));
               }
               else if (parm->nodeType == IdNT)
               {
                  parm->isUsed = true;
                  parm->isVisited = true;
                  Node *tmp = fetchSymbol(parm, ST);
                  if (tmp != NULL)
                  {
                     checkIfInit(tmp, node, ST);
                     if (tmp->isArray == true)
                     {
                        tmp->isIndexed = false;
                     }
                     tmp->isUsed = true;
                     tmp->isInit = true;
                     passedParms.insert(make_pair(parmCount, tmp));
                     if (tmp->nodeType == FuncNT)
                     {
                        printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, tmp->literal);
                        errs += 1;
                     }
                  }
                  else
                  {
                     printf("ERROR(%d): Symbol \'%s\' is not declared.\n", node->lineNum, parm->literal);
                     errs += 1;
                  }
               }
               else if (parm->nodeType == ArrNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processArray(parm, ST);
                  if (tmp != NULL)
                  {
                     tmp->isUsed = true;
                  }
                  passedParms.insert(make_pair(parmCount, tmp));
               }
               else if (parm->nodeType == CallNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processCall(parm, ST);
                  passedParms.insert(make_pair(parmCount, tmp));
               }
               else if (parm->nodeType == OpNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processOp(parm, ST);
                  passedParms.insert(make_pair(parmCount, tmp)); // Just pass the node to retain its DT!
               }
               else if (parm->nodeType == AssignNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processAssign(parm, ST);
                  if (tmp != NULL)
                  {
                     if (tmp->isArray == true)
                     {
                        tmp->isIndexed = true;
                     }
                  }
                  passedParms.insert(make_pair(parmCount, tmp));
               }
               else if (parm->nodeType == AndNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processAND(parm, ST);
                  passedParms.insert(make_pair(parmCount, parm));
               }
               else if (parm->nodeType == SignNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processCHSIGN(parm, ST);
                  passedParms.insert(make_pair(parmCount, parm));
               }
               else if (parm->nodeType == NotNT)
               {
                  parm->isVisited = true;
                  Node *tmp = processNOT(parm, ST);
                  passedParms.insert(make_pair(parmCount, parm));
               }

               parmCount++;
               parm = parm->sibling;
            }
            if (parmCount > function->parmC)
            {
               printf("ERROR(%d): Too many parameters passed for function '%s' declared on line %d.\n",
                      node->lineNum, function->literal, function->lineNum);
               errs += 1;
               // printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
               if (function->parmC != 0 && function->ParmList.empty() == false)
               {
                  checkPassedParams(node, function, passedParms);
               }
            }
            else if (parmCount < function->parmC)
            {
               printf("ERROR(%d): Too few parameters passed for function '%s' declared on line %d.\n",
                      node->lineNum, function->literal, function->lineNum);
               errs += 1;
               // printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
               if (function->parmC != 0 && function->ParmList.empty() == false)
               {
                  checkPassedParams(node, function, passedParms);
               }
            }
            else if (parmCount == function->parmC)
            {
               // printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
               if (function->parmC && function->ParmList.empty() == false)
               {
                  checkPassedParams(node, function, passedParms);
               }
            }
         }
         if (node->childC == 0 && function->parmC > 0)
         {
            printf("ERROR(%d): Too few parameters passed for function '%s' declared on line %d.\n",
                   node->lineNum, function->literal, function->lineNum);
            errs += 1;
            // printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
            if (function->parmC != 0 && function->ParmList.empty() == false)
            {
               checkPassedParams(node, function, passedParms);
            }
         }
         return function;
      }
      else
      {
         Node *function = fetchSymbol(node, ST);
         if (function != NULL)
         {
            function->isUsed = true;
            if (function->nodeType != FuncNT)
            {
               printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", node->lineNum, function->literal);
               errs += 1;
            }
         }
      }
      return NULL;
   }
   return NULL;
}
/* ==================================================
   PROCESS WHILE
   ================================================== */
void processWhile(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      node->isVisited = true;
      if (node->child[0] != NULL)
      {
         if (node->child[0]->isConst == true)
         {
            if (node->child[0]->dataType != BoolDT)
            {
               printf("ERROR(%d): Expecting Boolean test condition in while statement but got type %s.\n", node->lineNum, convertDTStr(node->child[0]));
               errs += 1;
            }
            return;
         }
         if (node->child[0]->nodeType == IdNT)
         {
            node->child[0]->isVisited = true;
            isSymbolDeclared(node->child[0], ST);
            Node *condition = fetchSymbol(node->child[0], ST);
            if (condition != NULL)
            {
               checkIfInit(condition, node, ST);
               if (condition->dataType != BoolDT)
               {
                  printf("ERROR(%d): Expecting Boolean test condition in while statement but got type %s.\n", node->lineNum, convertDTStr(condition));
                  errs += 1;
               }
               if (condition->isArray == true)
               {
                  printf("ERROR(%d): Cannot use array as test condition in while statement.\n", node->lineNum);
                  errs += 1;
               }
            }
            return;
         }
         if (node->child[0]->nodeType == CallNT)
         {
            node->child[0]->isVisited = true;
            Node *condition = processCall(node->child[0], ST);
            if (condition != NULL)
            {
               if (condition->dataType != BoolDT)
               {
                  printf("ERROR(%d): Expecting Boolean test condition in while statement but got type %s.\n", node->lineNum, convertDTStr(condition));
                  errs += 1;
               }
            }
            return;
         }
         if (node->child[0]->nodeType == AssignNT)
         {
            node->child[0]->isVisited = true;
            Node *conditional = processAssign(node->child[0], ST);
            return;
         }
      }
   }
   return;
}
/* ==================================================
   PROCESS IF
   ================================================== */
void processIf(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      if (node->child[0] != NULL)
      {
         if (node->child[0]->isConst == true)
         {
            if (node->child[0]->dataType != BoolDT)
            {
               printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", node->lineNum, convertDTStr(node->child[0]));
               errs += 1;
            }
            return;
         }
         if (node->child[0]->nodeType == IdNT)
         {
            Node *condition = fetchSymbol(node->child[0], ST);
            checkIfInit(condition, node, ST);
            if (condition != NULL)
            {
               condition->isUsed = true;
               if (condition->dataType != BoolDT)
               {
                  printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", node->lineNum, convertDTStr(condition));
                  errs += 1;
               }
               if (condition->isArray == true)
               {
                  printf("ERROR(%d): Cannot use array as test condition in if statement.\n", node->lineNum);
                  errs += 1;
               }
            }
            return;
         }
         if (node->child[0]->nodeType == AssignNT)
         {
            node->child[0]->isVisited = true;
            Node *condition = processAssign(node->child[0], ST);
            if (condition != NULL)
            {
               if (condition->dataType != BoolDT)
               {
                  printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", node->lineNum, convertDTStr(condition));
                  errs += 1;
               }
            }
            return;
         }
      }
   }
   return;
}
/* ==================================================
   PROCESS AND
   ================================================== */
Node *processAND(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *lhs = node->child[0];
      Node *rhs = node->child[1];
      Node *lsym = NULL;
      Node *rsym = NULL;

      if (lhs != NULL)
      {
         lhs->isVisited = true;
         lsym = processLHS(lhs, ST);
         if (lsym != NULL)
         {
            checkIfInit(lsym, node, ST);
            // lsym->isUsed = true;
            // lsym->isInit = true;
            if (lsym->dataType != BoolDT)
            {
               printf("ERROR(%d): '%s' requires operands of type bool but lhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
      }
      if (rhs != NULL)
      {
         rhs->isVisited = true;
         rsym = processRHS(rhs, ST);
         if (rsym != NULL)
         {
            rsym->isUsed = true;
            rsym->isInit = true;
            if (rsym->dataType != BoolDT)
            {
               printf("ERROR(%d): '%s' requires operands of type bool but rhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
      }
      if (lsym != NULL && rsym != NULL)
      {
         if (lsym->isArray == true || rsym->isArray == true)
         {
            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
            errs += 1;
         }

         if (lsym != NULL)
         {
            lsym->isIndexed = false;
         }
         if (rsym != NULL)
         {
            rsym->isIndexed = false;
         }
         Node *ret = createNode(NULL, RET_SYM);
         ret->dataType = BoolDT;
         return ret;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS OR
   ================================================== */
Node *processOR(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *lhs = node->child[0];
      Node *rhs = node->child[1];
      Node *lsym = NULL;
      Node *rsym = NULL;

      if (lhs != NULL)
      {
         lhs->isVisited = true;
         lsym = processLHS(lhs, ST);
         if (lsym != NULL)
         {
            lsym->isUsed = true;
            if (lsym->nodeType == VarNT || lsym->nodeType == VarArrNT)
               checkIfInit(lsym, node, ST);
            if (lsym->dataType != BoolDT)
            {
               printf("ERROR(%d): '%s' requires operands of type bool but lhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
      }
      if (rhs != NULL)
      {
         rhs->isVisited = true;
         rsym = processRHS(rhs, ST);
         if (rsym != NULL)
         {
            rsym->isUsed = true;
            rsym->isInit = true;
            if (rsym->dataType != BoolDT)
            {
               printf("ERROR(%d): '%s' requires operands of type bool but rhs is of type %s.\n",
                      node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
      }
      if (lsym != NULL && rsym != NULL)
      {
         if (lsym->isArray == true || rsym->isArray == true)
         {
            if (lsym->isIndexed == false || rsym->isIndexed == false)
            {
               printf("ERROR(%d): The operation 'or' does not work with arrays.\n", node->lineNum);
               errs += 1;
            }
         }
         Node *ret = createNode(NULL, RET_SYM);
         ret->dataType = BoolDT;
         return ret;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS NOT
   ================================================== */
Node *processNOT(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *val = node->child[0];
      if (val != NULL)
      {
         if (val->isConst == true)
         {
            if (val->dataType != BoolDT)
            {
               printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n",
                      node->lineNum, node->literal, convertDTStr(val));
               errs += 1;
            }
            return val;
         }
         else if (val->nodeType == IdNT)
         {
            val->isVisited = true;
            Node *sym = fetchSymbol(val, ST);
            if (sym != NULL)
            {
               sym->isUsed = true;
               if (sym->isArray == true)
               {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if (sym->dataType != BoolDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         else if (val->nodeType == CallNT)
         {
            val->isVisited = true;
            Node *sym = fetchFunction(val, ST);
            if (sym != NULL)
            {
               sym->isUsed = true;
               if (sym->dataType != BoolDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         else if (val->nodeType == OpNT)
         {
            val->isVisited = true;
            Node *sym = processOp(val, ST);
            if (sym != NULL)
            {
               if (sym->dataType != BoolDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         else if (val->nodeType == NotNT)
         {
            val->isVisited = true;
            Node *sym = processNOT(val, ST);
            if (sym != NULL)
            {
               Node *ret = createNode(NULL, RET_SYM);
               ret->dataType = BoolDT;
               return ret;
            }
         }
         else if (val->nodeType == OrNT)
         {
            val->isVisited = true;
            Node *sym = processOR(val, ST);
            if (sym != NULL)
            {
               if (sym->dataType != BoolDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         if (val->nodeType == AndNT)
         {
            val->isVisited = true;
            Node *sym = processAND(val, ST);
            if (sym != NULL)
            {
               if (sym->dataType != BoolDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS QUEST
   ================================================== */
Node *processQUES(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *val = node->child[0];
      if (val != NULL)
      {
         if (val->nodeType == IdNT)
         {
            Node *sym = fetchSymbol(val, ST);
            if (sym != NULL)
            {
               sym->isUsed = true;
               if (sym->isArray == true)
               {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if (sym->dataType != IntDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         if (val->nodeType == QuesNT)
         {
            val->isVisited == true;
            Node *sym = processQUES(val, ST);
            if (sym != NULL)
            {
               sym->isUsed = true;
               return sym;
            }
         }
         if (val->nodeType == CallNT)
         {
            val->isVisited = true;
            Node *sym = fetchFunction(val, ST);
            if (sym != NULL)
            {
               sym->isUsed = true;
               if (sym->dataType != IntDT)
               {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                         node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
            }
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS SIZE OF
   ================================================== */
Node *processSizeOf(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      Node *val = node->child[0];
      if (val != NULL)
      {
         val->isVisited = true;
         if (val->nodeType == SizeOfNT)
         {
            printf("ERROR(%d): The operation '%s' only works with arrays.\n", node->lineNum, node->literal);
            errs += 1;
            return NULL;
         }
         else
         {
            Node *sym = processRHS(val, ST);
            if (sym != NULL)
            {
               sym->isUsed = true;
               checkIfInit(sym, node, ST);
               if (sym->isArray == false || val->nodeType == ArrNT)
               {
                  printf("ERROR(%d): The operation '%s' only works with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
         }
         Node *ret = createNode(NULL, RET_SYM);
         ret->dataType = IntDT;
         return ret;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS RANGE
   ================================================== */
void processRange(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      node->isVisited = true;
      for (int i = 0; i < node->childC; i++)
      {
         if (node->child[i] != NULL)
         {
            node->child[i]->isVisited = true;
            Node *sym = processLHS(node->child[i], ST);
            if (sym != NULL)
            {
               checkIfInit(sym, node, ST);
               sym->isUsed = true;
               if (sym->isArray == true && node->child[i]->nodeType != ArrNT)
               {
                  printf("ERROR(%d): Cannot use array in position %d in range of for statement.\n",
                         node->lineNum, i + 1);
                  errs += 1;
               }
               if (sym->dataType != IntDT)
               {
                  printf("ERROR(%d): Expecting type int in position %d in range of for statement but got type %s.\n",
                         node->lineNum, i + 1, convertDTStr(sym));
                  errs += 1;
               }
               if (sym->nodeType == FuncNT && node->child[i]->nodeType != CallNT)
               {
                  printf("ERROR(%d): Cannot use function '%s' as a variable.\n", node->lineNum, sym->literal);
                  errs += 1;
               }
            }
         }
      }
   }
}
/* ==================================================
   CHECK MAIN (Only called in main.c)
   ================================================== */
void checkMain(SymbolTable *ST)
{
   ST->checkUnusedGlobalVars();
   if (hasMain == false)
   {
      printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n");
      errs += 1;
   }
}
/* ==================================================
   CHECK PASSED PARMS
   ================================================== */
void checkPassedParams(Node *node, Node *function, map<int, Node *> passedParms)
{
   for (int i = 0; i < passedParms.size(); i++)
   {
      // printf("%s\n", function->ParmList[i]->literal);
      // printf("%s\n", passedParms[i]->literal);
      if (function->ParmList[i] != NULL && passedParms[i] != NULL)
      {
         if (function->ParmList[i]->dataType != passedParms[i]->dataType)
         {
            printf("ERROR(%d): Expecting type %s in parameter %d of call to '%s' declared on line %d but got type %s.\n",
                   node->lineNum, convertDTStr(function->ParmList[i]), i + 1, function->literal, function->lineNum, convertDTStr(passedParms[i]));
            errs += 1;
         }
         if (passedParms[i]->isArray == true && passedParms[i]->isIndexed == false)
         {
            if (function->ParmList[i]->isArray == false)
            {
               printf("ERROR(%d): Not expecting array in parameter %d of call to '%s' declared on line %d.\n",
                      node->lineNum, i + 1, function->literal, function->lineNum);
               errs += 1;
            }
         }
         if (passedParms[i]->isArray == false)
         {
            if (function->ParmList[i]->isArray == true)
            {
               printf("ERROR(%d): Expecting array in parameter %d of call to '%s' declared on line %d.\n",
                      node->lineNum, i + 1, function->literal, function->lineNum);
               errs += 1;
            }
         }
      }
   }
}
/* ==================================================
   IS THIS FUNCTION A MAIN?
   ================================================== */
void is_this_func_main(Node *node, SymbolTable *ST)
{
   if (node != NULL)
   {
      if (node->nodeType == FuncNT)
      {
         if (string(node->literal).compare("main") == 0)
         {
            node->isUsed = true;
            if (checkIfDeclaredAlreadyGlobal(node, ST) == false)
            {
               if (node->child[0] != NULL)
               {
                  if (node->child[0]->nodeType == ParmNT || node->child[0]->nodeType == ParmArrNT)
                  {
                     hasMain = false;
                  }
                  else
                  {
                     hasMain = true;
                  }
               }
               else
               {
                  hasMain = true;
               }
            }
         }
      }
   }
}
/* ==================================================
   CHECK INITIALIZERS
   ================================================== */
void checkInitializers(Node *node, SymbolTable *ST)
{
   Node *int_val = node->child[0];
   if (int_val != NULL)
   {
      if (int_val->nodeType == IdNT || int_val->nodeType == QuesNT || int_val->nodeType == SizeOfNT || int_val->nodeType == CallNT)
      {
         if (int_val->nodeType == IdNT)
         {
            if (string(node->literal).compare(int_val->literal) == 0)
            {
               printf("ERROR(%d): Symbol \'%s\' is not declared.\n", node->lineNum, node->literal);
               errs += 1;
            }
         }
         printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n",
                node->lineNum, node->literal);
         errs += 1;
      }
      if (node->isArray == true && int_val->isArray == false)
      {
         printf("ERROR(%d): Initializer for variable '%s' requires both operands be arrays or not but variable is an array and rhs is not an array.\n",
                node->lineNum, node->literal);
         errs += 1;
      }
      if (node->isArray == false && int_val->isArray == true)
      {
         printf("ERROR(%d): Initializer for variable '%s' requires both operands be arrays or not but variable is not array and rhs is an array.\n",
                node->lineNum, node->literal);
         errs += 1;
      }
      if (node->dataType != int_val->dataType && int_val->dataType != UndefinedDT)
      {
         printf("ERROR(%d): Initializer for variable '%s' of type %s is of type %s\n",
                node->lineNum, node->literal, convertDTStr(node), convertDTStr(int_val));
         errs += 1;
      }
      if (int_val->nodeType == OpNT)
      {
         for (int i = 0; i < int_val->childC; i++)
         {
            Node *tmp = int_val->child[i];
            if (tmp != NULL)
            {
               if (tmp->nodeType == IdNT)
               {
                  printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n",
                         node->lineNum, node->literal);
                  errs += 1;
                  break;
               }
               if (tmp->nodeType == OpNT)
               {
                  for (int i = 0; i < tmp->childC; i++)
                  {
                     if (tmp->child[i] != NULL)
                     {
                        if (tmp->child[i]->nodeType == QuesNT)
                        {
                           printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n",
                                  node->lineNum, node->literal);
                           errs += 1;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
}
/* ========================================================================
   END OF SEMANTIC ANALYSIS FUNCTIONS
   ======================================================================== */