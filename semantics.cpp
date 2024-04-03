/* ========================================================================
   INCLUDES
   ======================================================================== */
#include "semantics.h"
#include "scope.h"
#include "parser.tab.h"
#include "routines.h"

/* ======================================================================== */
extern int warns; // Warning counter, initialized to 0
extern int errs; // Error counter, initialized to 0
/* ======================================================================== */

// DEBUGGING
bool smDebug = false;
bool smTable = false;

/* ========================================================================
   GLOBALS
   ======================================================================== */
Node * parentScope = NULL;            // Set to the function node after insertion
bool inFunction = false;         // Are we in a function?
bool hasMain = false;            // Does this program have a main()?
bool checkInit = false;
bool inLoop = false;
int nStmts = 0;
int fStmts = 0;
int i = 0;

/* ========================================================================
   NEW TRAVERSAL METHOD
   ======================================================================== */
void traverseAST(Node * AST, SymbolTable * ST) {
   if( AST != NULL ) {
      if( AST->nodeType == FuncNT ) {
         if( string(AST->literal).compare("main") == 0 ) {
            AST->isUsed = true;
            if( !checkIfDeclaredAlreadyGlobal(AST, ST) ) {
               for(int i = 0; i < AST->childC; i++ ) {
                  if( AST->child[0] == NULL && AST->child[1]->nodeType == CompoundNT ) {
                     hasMain = true;
                  }
               } 
            }
         }
         nStmts = 0;
         fStmts += 1;
         inFunction = false;
         parentScope = AST;
         if( string(AST->literal).compare("main") != 0 ) checkIfDeclaredAlreadyGlobal(AST, ST);
         ST->insertGlobal(string(AST->literal), AST);
         ST->enter(string(AST->literal));
         traverseScope(AST, ST);
         if( parentScope->hasReturn == false ) {
            if( parentScope->dataType != VoidDT ) {
               printf("WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n", 
               AST->lineNum, convertDTStr(parentScope), parentScope->literal);
               warns += 1;
            }
         }
      }
      
      if( isSymbol(AST) && AST->isVisited == false ) {
         AST->isVisited = true;
         checkIfDeclaredAlreadyGlobal(AST, ST);
         AST->isInit = true;
         ST->insert(string(AST->literal), AST);
         if ( AST->child[0] != NULL ) {
            if( AST->child[0]->nodeType == IdNT ) {
               printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n",
               AST->lineNum, AST->child[i]->literal);
               errs += 1;
            }
            if( AST->isArray == true && AST->child[0]->isArray == false ) {
               printf("ERROR(%d): Initializer for variable '%s' requires both operands be arrays or not but variable is an array and rhs is not an array.\n",
               AST->lineNum, AST->literal);
               errs += 1;
            }
            if( AST->child[0]->isConst == true ) {
               if( AST->dataType != AST->child[0]->dataType ) {
                  printf("ERROR(%d): Initializer for variable '%s' of type %s is of type %s.\n",
                  AST->lineNum, AST->literal, convertDTStr(AST), convertDTStr(AST->child[0]));
                  errs += 1;
               }
            }
         }
      }

      /*if( AST->visited == false ) {
         printf("\tSymbol: %s [%d]\n", AST->literal, AST->lineNum);
      }*/

      for( int i = 0; i < AST->childC; i++ ) {
         traverseAST(AST->child[i], ST);
      }
      traverseAST(AST->sibling, ST);
   }
}
/* ========================================================================
   TRAVERSE THROUGH THE SCOPE ONLY
   ======================================================================== */
void traverseScope(Node * AST, SymbolTable * ST) {
   if( AST != NULL ) {
      if( AST->nodeType == CompoundNT ) {
         if( nStmts == 0 ) {
            nStmts = 1;
         }
         else {
            ST->enter("Compound");
         }
      }

      if( AST->nodeType == ToNT ) {
         ST->enter("For");
         for( int i = 0; i < AST->childC; i++ ) {
            if( AST->child[i] != NULL ) {
               if( AST->child[i]->nodeType == CompoundNT ) {
                  AST->isVisited = true;
                  nStmts = 0;
               }
            }
         }
      }

      if( isSymbol(AST) ) {
         AST->isVisited = true;
         if( AST->nodeType == ParmNT || AST->nodeType == ParmArrNT ) {
            if( parentScope != NULL ) {
               parentScope->ParmList.insert({parentScope->parmC, AST});
               //printf("%s\n", Parms[parentScope->parmC]->literal);
               parentScope->parmC++; 
            }
         }
         //printf("\tSymbol: %s [%d] \n", AST->literal, AST->lineNum);
         checkIfDeclaredAlready(AST, ST);
         ST->insert(string(AST->literal), AST);
         if ( AST->child[0] != NULL ) {
            if( AST->child[0]->nodeType == IdNT ) {
               printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n",
               AST->lineNum, AST->literal);
               errs += 1;
            }
            if( AST->isArray == true && AST->child[0]->isArray == false ) {
               printf("ERROR(%d): Initializer for variable '%s' requires both operands be arrays or not but variable is an array and rhs is not an array.\n",
               AST->lineNum, AST->literal);
               errs += 1;
            }
            if( AST->child[0]->isConst == true ) {
               if( AST->dataType != AST->child[0]->dataType ) {
                  printf("ERROR(%d): Initializer for variable '%s' of type %s is of type %s.\n",
                  AST->lineNum, AST->literal, convertDTStr(AST), convertDTStr(AST->child[0]));
                  errs += 1;
               }
            }
         }
      } else {
         semanticAnalysis(AST, ST);
      }

      /*if( isScope(AST) ) {
         printf("\tSymbol: %s [%d] \n", AST->literal, AST->lineNum);
      }*/

      for( int i = 0; i < AST->childC; i++ ) {
         traverseScope(AST->child[i], ST);
      }

      if( AST->nodeType == FuncNT ) {
         if( ST->depth() == 2 ) {
            ST->checkUnusedVars();
            ST->leave();
         }
         return;
      } else {
         if( AST->sibling != NULL ) {
            if( AST->nodeType == ToNT ) {
               if( ST->depth() > 2 ) {
                  ST->checkUnusedVars();
                  ST->leave();
               }
               traverseScope(AST->sibling, ST);
            }
            else if( AST->nodeType == CompoundNT ) { 
               if( ST->depth() > 2) {
                  ST->checkUnusedVars();
                  ST->leave();
               }
               traverseScope(AST->sibling, ST);
            } else {
               traverseScope(AST->sibling, ST);
            }  
         }
         else if( AST->nodeType == CompoundNT ) {
            if( ST->depth() > 2 ) {
               ST->checkUnusedVars();
               ST->leave();
            }
         }
         else if ( AST->nodeType == FuncNT ) {
            ST->checkUnusedVars();
            ST->leave();
            return;
         }
      } 
   }
}
/* ========================================================================
   SEMANTIC ANALYSIS
   ======================================================================== */
void semanticAnalysis(Node * node, SymbolTable * ST) {
   if( node->nodeType == IdNT ) {
      if( node->isVisited == false ) {
         node->isVisited = true;
         Node * id = fetchSymbol(node, ST);
         if( id != NULL ) {
            if( id->isStatic == true ) id->isInit = true;
            checkIfInit(id, node, ST);
            id->isInit = true;
            id->isUsed = true;
         }
      }
   }
   if( node->nodeType == ArrNT ) { 
      if( node->isVisited == false ) {
         node->isVisited = true;
         checkInit = true;
         Node * id = processArray(node, ST);
         checkInit = false;
         if( id != NULL ) {
            checkIfInit(id, node, ST);
            id->isInit = true;
            id->isUsed = true;
            if( id->nodeType == FuncNT ) {
               printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, id->literal);
               errs += 1;
            }
            id->isIndexed = false;
         }
      }
   }
   if( node->nodeType == ReturnNT && node->isVisited == false ) {
      node->isVisited = true;
      processReturn(node, ST);
   }
   if( node->nodeType == CallNT && node->isVisited == false ) {
      processCall(node, ST);
   }
   if( node->nodeType == AssignNT && node->isVisited == false ) {
      processAssign(node, ST);
   }
   if( node->nodeType == OpNT && node->isVisited == false ) {
      processOp(node, ST);
   }
   if( node->nodeType == SignNT && node->isVisited == false ) {
      processCHSIGN(node, ST);
   }
   if( node->nodeType == IfNT ) {
      processIf(node, ST);
   }
   if( node->nodeType == IterNT ) {
      processWhile(node, ST);
   }
   if( node->nodeType == QuesNT ) {
      processQUES(node, ST);
   }
   if( node->nodeType == AndNT && node->isVisited == false ) {
      processAND(node, ST);
   }
   if( node->nodeType == SizeOfNT ) {
      processSizeOf(node, ST);
   }
   if( node->nodeType == NotNT && node->isVisited == false ) {
      processNOT(node, ST);
   }
   if( node->nodeType == OrNT && node->isVisited == false ) {
      processOR(node, ST);
   }
   if( node->nodeType == RangeNT ) {
      processRange(node, ST);
   }
}
/* ========================================================================
   SEMANTIC ANALYSIS FUNCTIONS
   ======================================================================== */

/* ==================================================
   PROCESS CHSIGN              
   ================================================== */
Node * processCHSIGN(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      node->isVisited = true;
      Node * id = node->child[0];
      if( id != NULL ) {
         if( id->nodeType == IdNT ) {
            id->isVisited = true;
            Node * sym = processLHS(id, ST);
            if( sym != NULL ) {
               sym->isUsed == true;
               checkIfInit(sym, node, ST);
               sym->isInit = true;
               if( sym->isArray == true ) {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if( sym->dataType != IntDT ) {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n", 
                  node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         else if ( id->nodeType == SignNT )
         {
            Node * sym = processCHSIGN(id, ST);
            if( sym != NULL ) {
               //
            }
         }
         
      }
   }
   return NULL;
}

/* ==================================================
   PROCESS ARRAY              
   ================================================== */
Node * processArray(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * id = node->child[0];
      Node * index = node->child[1];

      Node * symbol;
      if( id != NULL ) {
         id->isVisited = true;
         symbol = processLHS(id, ST);
         if( symbol != NULL ) {
            node->dataType = symbol->dataType;
            if( checkInit == true ) checkIfInit(symbol, node, ST);
            symbol->isInit = true;
            if( symbol->isArray == false ) {
               printf("ERROR(%d): Cannot index nonarray \'%s\'.\n", node->lineNum, symbol->literal);
               errs += 1;
            }
         }
         else {
            printf("ERROR(%d): Cannot index nonarray \'%s\'.\n", node->lineNum, id->literal);
            errs += 1;            
         }
      }
      if( index != NULL ) {
         if( symbol != NULL ) symbol->isIndexed = true;
         
         if( index->isConst == true ) {
            if( index->dataType != IntDT ) {
               printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(index));
               errs += 1;
            }
         }
         else if( index->nodeType == IdNT ) {
            index->isVisited = true;
            Node * idx = processLHS(index, ST);
            if( idx != NULL ) {
               idx->isUsed = true;
               if( checkInit == true ) checkIfInit(idx, node, ST);
               idx->isInit = true;
               if( idx->dataType != IntDT ) {
                  printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(idx));
                  errs += 1;
               }
               if( idx->isArray == true ) {
                  printf("ERROR(%d): Array index is the unindexed array '%s'.\n", node->lineNum, idx->literal);
                  errs += 1;
               }
            }
         }
         else if( index->nodeType == ArrNT ) {
            Node * array = processArray(index, ST);
            if( array != NULL ) {
               array->isUsed = true;
               if( array->dataType != IntDT ) {
                  printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, symbol->literal, convertDTStr(array));
                  errs += 1;
               }
            }
         }
         else if ( index->nodeType == OpNT ) {
            index->isVisited = true;
            processOp(index, ST);
         }
         
      }
      if( symbol != NULL ) return symbol;
   }
   return NULL;
}
/* ==================================================
   PROCESS LHS              
   ================================================== */
Node * processLHS(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      if( node->isConst == true ) {
         return node;
      }
      else if ( node->nodeType == IdNT )
      {
         if( isSymbolDeclared(node, ST) ) {
            Node * lsym = fetchSymbol(node, ST);
            return lsym;
         }
      }
      else if( node->nodeType == ArrNT ) {
         Node * lsym = processArray(node, ST);
         return lsym;
      }
      else if( node->nodeType == CallNT ) {
         Node * lsym = processCall(node, ST);
         return lsym;
      }
      else if ( node->nodeType == OpNT )
      {
         Node * lsym = processOp(node, ST);
         return lsym; // RETURN NODE OP TO RETAIN DT!
      }
   }
   return NULL;
}

/* ==================================================
   PROCESS RHS              
   ================================================== */
Node * processRHS(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      if(node->isConst == true ) {
         return node;
      }
      else if ( node->nodeType == IdNT )
      {
         if( isSymbolDeclared(node, ST) ) {
            Node * rsym = fetchSymbol(node, ST);
            return rsym;
         }
      }
      else if( node->nodeType == ArrNT ) {
         Node * rsym = processArray(node, ST);
         if( rsym != NULL ) rsym->isInit = true;
         return rsym;
      }
      else if ( node->nodeType == CallNT ) {
         Node * rsym = processCall(node, ST);
         return rsym;
      }
      else if ( node->nodeType == OpNT ) {
         Node * rsym = processOp(node, ST);
         return rsym;
      } 
      else if ( node->nodeType == AndNT ) {
         Node * rsym = processAND(node, ST);
         return rsym;
      }
      else if ( node->nodeType == OrNT )
      {
         Node * rsym = processOR(node, ST);
         return rsym;
      }
      
      
   }
   return NULL;
}
/* ==================================================
   PROCESS OPERATOR              
   ================================================== */
Node * processOp(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * lhs = node->child[0];
      Node * rhs = node->child[1];
      Node * lsym = NULL;
      Node * rsym = NULL;

      if( node->tknClass == LESS || node->tknClass == LEQ || node->tknClass == GREAT || 
          node->tknClass == GEQ || node->tknClass == EQL || node->tknClass == NEQ ) 
      {
         // PROCESS LHS!
         if( lhs != NULL ) {
            lhs->isVisited = true;
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) {
               lsym->isUsed = true;
               checkIfInit(lsym, node, ST);
            }
         }
         // PROCESS RHS
         if( rhs != NULL ) {
            rhs->isVisited = true;
            if( rhs->nodeType == AssignNT ) {
               rhs->isVisited = true;
               rsym = processAssign(rhs, ST);
            }
            else if ( rhs->nodeType == OpNT ) {
               rhs->isVisited = true;
               rsym = processOp(rhs, ST);
            }
            else {
               rsym = processRHS(rhs, ST);
               if( rsym != NULL ) {
                  rsym->isUsed = true;
               }
            }
         }  
         // ERROR CHECK!
         if( lsym != NULL && rsym != NULL ) {
            if( lsym->dataType != rsym->dataType ) {
               printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n",
               node->lineNum, node->literal, convertDTStr(lsym), convertDTStr(rsym));
               errs += 1;
            }
            if( lsym->isArray == true && lsym->isIndexed == false  ) {
               if( rsym->isArray == false && rsym->isConst == false ) {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n",
                  node->lineNum, node->literal);
                  errs += 1;
               }
            }
            if( lsym->isArray == false ) {
               if( rsym->isArray == true ) {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n",
                  node->lineNum, node->literal);
                  errs += 1;
               }
            }

            if( lsym != NULL ) { lsym->isIndexed = false; }
            if( rsym != NULL ) { rsym->isIndexed = false; }
            // SYMBOL RETURN PLACEHOLDER, RETURNS JUST THE BOOL VALUE!
            Node * tmp = createNode(NULL, RET_SYM);
            tmp->literal = lsym->literal;
            tmp->lineNum = node->lineNum;
            tmp->tknClass = lsym->tknClass;
            tmp->dataType = BoolDT;

            return tmp;
         }
      }
      else if( node->tknClass == ADD || node->tknClass == SUB || node->tknClass == MUL || 
               node->tknClass == DIV || node->tknClass == MOD ) 
      {
         // PROCESS LHS!
         if( lhs != NULL ) {
            lhs->isVisited = true;
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) {
               lsym->isUsed = true;
               if( checkInit == true ) checkIfInit(lsym, node, ST);
               lsym->isInit = true;
            }
         }
         // PROCESS RHS
         if( rhs != NULL ) {
            rhs->isVisited = true;
            if( rhs->nodeType == AssignNT ) {
               rhs->isVisited = true;
               rsym = processAssign(rhs, ST);
               
            }
            else if ( rhs->nodeType == OpNT ) {
               rhs->isVisited = true;
               rsym = processOp(rhs, ST);
            }
            else {
               rsym = processRHS(rhs, ST);
               if( rsym != NULL ) {
                  rsym->isUsed = true;
                  if( checkInit == true ) checkIfInit(rsym, node, ST);
                  rsym->isInit = true;
               }
            }
         }     
         if( lsym != NULL ) {
            if( lsym->dataType != IntDT ) {
               printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n",
               node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         } 
         if (rsym != NULL ) {
            if( rsym->dataType != IntDT ) {
               printf("ERROR(%d): '%s' requires operands of type int but rhs is of type %s.\n",
               node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
         if( lsym != NULL && rsym != NULL ) {
            if( (lhs->nodeType != ArrNT && lsym->isArray == true) || (rhs->nodeType != ArrNT && rsym->isArray == true ) ) 
            {
               printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
               errs += 1;
            }
         }

         if( lsym != NULL ) { lsym->isIndexed = false; }
         if( rsym != NULL ) { rsym->isIndexed = false; }

         return lsym;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS ASSIGNMENT              
   ================================================== */
Node * processAssign(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * lhs = node->child[0];
      Node * rhs = node->child[1];
      Node * lsym = NULL;
      Node * rsym = NULL;

      // <=
      if( node->tknClass == ASGN ) {
         // PROCESS LHS
         if( lhs != NULL ) {
            lhs->isVisited = true;
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) {
               lsym->isUsed = true;
               node->dataType = lsym->dataType;
               if( lsym->nodeType == FuncNT ) {
                  printf("ERROR(%d): Cannot use function \'%s\' as a variable!\n", node->lineNum, lsym->literal);
                  errs += 1;
               }
            }
         }
         // PROCESS RHS
         if( rhs != NULL) {
            rhs->isVisited = true;
            if( rhs->nodeType == AssignNT ) {
               rsym = processAssign(rhs, ST);
               
            }
            else if ( rhs->nodeType == OpNT ) {
               checkInit = true;
               rsym = processOp(rhs, ST);
               checkInit = false;
               if( rsym != NULL ) {
                  checkIfInit(rsym, node, ST);
               }
            }
            else {
               checkInit = true;
               rsym = processRHS(rhs, ST);
               checkInit = false;
               checkIfInit(rsym, node, ST);
               if( rsym != NULL ) {
                  rsym->isUsed = true;
               }
            }
         }
         // ERROR CHECK!
         if( lsym != NULL && rsym != NULL ) {
            lsym->isInit = true;
            //printf("lhs: %s\n", lsym->literal);
            //printf("rhs: %s\n", rsym->literal);
            if( lsym->isArray == true && lsym->isIndexed == false ) {
               if( rsym->isArray == false ) {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
            if( lsym->isArray == false ) {
               if( rsym->isArray == true && rsym->isIndexed == false ) {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n", node->lineNum, node->literal);
                  errs += 1;                  
               }
            }
            if( lsym->dataType != rsym->dataType ) {
               printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n",
               node->lineNum, node->literal, convertDTStr(lsym), convertDTStr(rsym));
               errs += 1;
            }
            if( lsym != NULL ) { lsym->isIndexed = false; }
            if( rsym != NULL ) { rsym->isIndexed = false; }
            rsym->isInit = true;
            return lsym;
         }
      }

      if( node->tknClass == ADDASS || node->tknClass == SUBASS ||
          node->tknClass == MULASS || node->tknClass == DIVASS )
      {
         if( lhs != NULL ) {
            lhs->isVisited = true;
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) {
               lsym->isUsed = true;
               lsym->isInit = true;
               node->dataType = lsym->dataType;
               if( lsym->nodeType == FuncNT ) {
                  printf("ERROR(%d): Cannot use function \'%s\' as a variable!\n", node->lineNum, lsym->literal);
                  errs += 1;
               }
            }
         }
         if( rhs != NULL ) {
            rhs->isVisited = true;
            if( rhs->nodeType == AssignNT ) {
               rsym = processAssign(rhs, ST);
               
            }
            else if ( rhs->nodeType == OpNT ) {
               checkInit = true;
               rsym = processOp(rhs, ST);
               checkInit = false;
               if( rsym != NULL ) {
                  checkIfInit(rsym, node, ST);
               }
            }
            else {
               checkInit = true;
               rsym = processRHS(rhs, ST);
               checkInit = false;
               checkIfInit(rsym, node, ST);
               if( rsym != NULL ) {
                  rsym->isUsed = true;
               }
            }
         }

         if( lsym != NULL ) {
            if( lsym->dataType != IntDT ) {
               printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n", 
               node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
         if( rsym != NULL ) {
            if( rsym->dataType != IntDT ) {
               printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n", 
               node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }

         if( lsym != NULL ) { lsym->isIndexed = false; }
         if( rsym != NULL ) { rsym->isIndexed = false; }

         return lsym;
      }

      if( node->tknClass == INC || node->tknClass == DEC ) {
         if( lhs != NULL ) {
            lhs->isVisited = true;
            // Ordering here is wacky for the order by which uninit vars appear for desired output.
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) { 
               lsym->isUsed = true; 
               checkIfInit(lsym, node, ST);
               if( lhs->nodeType != ArrNT && lsym->isArray == true ) {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if( lsym->dataType != IntDT ) {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                  node->lineNum, node->literal, convertDTStr(lsym));
                  errs += 1;
               } 
            }
            else {
               return NULL;
            }
            checkInit = true;
            lsym = processLHS(lhs, ST);
            checkInit = false;
            if( lsym != NULL ) { lsym->isIndexed = false; lsym->isInit = true; }
            return lsym;
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS RETURN           
   ================================================== */
void processReturn(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      if( parentScope != NULL ) {
         parentScope->hasReturn = true;
         for( int i = 0; i < node->childC; i++ ) {
            Node * val = node->child[i];
            if( val != NULL ) {
               if( val->isConst == true ) {
                  if( parentScope->dataType != node->child[i]->dataType ) {
                     printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but returns type %s.\n",
                     node->lineNum, parentScope->literal, parentScope->lineNum, convertDTStr(parentScope), convertDTStr(val));
                     errs += 1;
                  }
               }
               else if ( val->nodeType == IdNT ) {
                  val->isVisited = true;
                  Node * id = fetchSymbol(val, ST);
                  if( id != NULL ) {
                     id->isUsed = true;
                     if( id->isArray == true ) {
                        printf("ERROR(%d): Cannot return an array.\n", node->lineNum);
                        errs += 1;
                     }
                  }
                  else {
                     printf("ERROR(%d): Symbol \'%s\' is not declared.\n", node->lineNum, val->literal);
                     errs += 1;
                  }
               }
               else if ( val->nodeType == CallNT )
               {
                  val->isVisited = true;
                  Node * call = processCall(val, ST);
                  if( call != NULL ) {
                     call->isUsed = true;
                     //
                  }
               }
               else if( val->nodeType == AssignNT ) {
                  val->isVisited = true;
                  Node * id = processAssign(val, ST);
                  if( id != NULL ) {
                     if( id->isArray == true ) {
                        printf("ERROR(%d): Cannot return an array.\n", node->lineNum);
                        errs += 1;
                     }
                  }
               }
            }
         }
         if( node->childC == 0 && parentScope->dataType != VoidDT ) {
            printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but returns type has no value.\n",
            node->lineNum, parentScope->literal, parentScope->lineNum, convertDTStr(parentScope));
            errs += 1;
         }
         if( node->childC >= 1 && parentScope->dataType == VoidDT ) {
            printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has a value.\n",
            node->lineNum, parentScope->literal, parentScope->lineNum);
            errs += 1;
         }
      }
   }
}
/* ==================================================
   PROCESS CALL              
   ================================================== */
Node * processCall(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      node->isVisited = true;
      int parmCount = 0;
      map<int, Node *> passedParms;
      isSymbolDeclared(node, ST);
      Node * function = fetchFunction(node, ST);
      if( function != NULL ) {
         //printf("%s\n", function->literal);
         function->isUsed = true;
         if( node->childC != 0 ) {
            Node * parm = node->child[0];
            while( parm != NULL ) {
               if( parm->isConst == true ) {
                  parm->isVisited = true;
                  passedParms.insert({parmCount, parm});
               }
               else if( parm->nodeType == IdNT ) {
                  parm->isUsed = true;
                  parm->isVisited = true;
                  Node * tmp = fetchSymbol(parm, ST);
                  if( tmp != NULL ) {
                     if( tmp->isArray == true ) {
                        tmp->isIndexed = false;
                     }
                     if( tmp->nodeType == FuncNT ) {
                        printf("ERROR(%d): Cannot use function '%s' as a variable.\n",
                        node->lineNum, tmp->literal);
                        errs += 1;
                     }
                     tmp->isUsed = true;
                     checkIfInit(tmp, node, ST);
                     tmp->isInit = true;
                     passedParms.insert({parmCount, tmp});
                  }
                  else {
                     printf("ERROR(%d): Symbol \'%s\' is not declared.\n", node->lineNum, parm->literal);
                     errs += 1;
                  }
               }
               else if ( parm->nodeType == ArrNT ) {
                  parm->isVisited = true;
                  Node * tmp = processArray(parm, ST);
                  if( tmp != NULL ) {
                     tmp->isUsed = true;
                  }
                  passedParms.insert({parmCount, tmp});
               }
               else if ( parm->nodeType == CallNT ) {
                  parm->isVisited = true;
                  Node * tmp = processCall(parm, ST);
                  passedParms.insert({parmCount, tmp});
               }
               else if ( parm->nodeType == OpNT ) {
                  parm->isVisited = true;
                  Node * tmp = processOp(parm, ST);
                  passedParms.insert({parmCount, parm}); // Just pass the node to retain its DT!
               }
               else if ( parm->nodeType == AssignNT ) {
                  parm->isVisited = true;
                  Node * tmp = processAssign(parm, ST);
                  if( tmp != NULL ) {
                     if( tmp->isArray == true ) {
                        tmp->isIndexed = true;
                     }
                  }
                  passedParms.insert({parmCount, tmp});
               }
               else if ( parm->nodeType == AndNT )
               {
                  parm->isVisited = true;
                  Node * tmp = processAND(parm, ST);
                  passedParms.insert({parmCount, parm});
               }
               
               parmCount++;
               parm = parm->sibling;
            }
            if( parmCount > function->parmC ) {
               printf("ERROR(%d): Too many parameters passed for function '%s' declared on line %d.\n", 
               node->lineNum, function->literal, function->lineNum);
               errs += 1;
               //printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
               if( function->parmC != 0 && function->ParmList.empty() == false ) {
                  checkPassedParams(node, function, passedParms);
               }
            }
            else if ( parmCount < function->parmC ) {
               printf("ERROR(%d): Too few parameters passed for function '%s' declared on line %d.\n",
               node->lineNum, function->literal, function->lineNum);
               errs += 1;
               //printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
               if( function->parmC != 0 && function->ParmList.empty() == false ) {
                  checkPassedParams(node, function, passedParms);
               }
            }
            else if (parmCount == function->parmC )
            {
               //printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
               if( function->parmC && function->ParmList.empty() == false ) {
                  checkPassedParams(node, function, passedParms);
               }
            }
         }
         if ( parmCount < function->parmC ) {
            printf("ERROR(%d): Too few parameters passed for function '%s' declared on line %d.\n",
            node->lineNum, function->literal, function->lineNum);
            errs += 1;
            //printf("(%d) Parameters #%d, Func: #%d:\n", node->lineNum, parmCount, function->parmC);
            if( function->parmC != 0 && function->ParmList.empty() == false ) {
               checkPassedParams(node, function, passedParms);
            }
         }
         return function;
      }
      else {
         Node * function = fetchSymbol(node, ST);
         if( function != NULL ) {
            if( function->nodeType != FuncNT ) {
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
void processWhile(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      if( node->child[0] != NULL ) {
         if( node->child[0]->isConst == true ) {
            if( node->child[0]->dataType != BoolDT ) {
               printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", node->lineNum, convertDTStr(node->child[0]));
               errs += 1;     
            }
            return;
         }
         if( node->child[0]->nodeType == IdNT ) {
            node->child[0]->isVisited = true;
            Node * condition = fetchSymbol(node->child[0], ST);
            if( condition != NULL ) {
               checkIfInit(condition, node, ST);
               if( condition->dataType != BoolDT ) {
                  printf("ERROR(%d): Expecting Boolean test condition in while statement but got type %s.\n", node->lineNum, convertDTStr(condition));
                  errs += 1;
               }
               if( condition->isArray == true ) {
                  printf("ERROR(%d): Cannot use array as test condition in while statement.\n", node->lineNum);
                  errs += 1;
               }
            }
            return;
         }
         if( node->child[0]->nodeType == CallNT ) {
            node->child[0]->isVisited = true;
            Node * condition = processCall(node->child[0], ST);
            if( condition != NULL ) {
               if( condition->dataType != BoolDT ) {
                  printf("ERROR(%d): Expecting Boolean test condition in while statement but got type %s.\n", node->lineNum, convertDTStr(condition));
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
   PROCESS IF              
   ================================================== */
void processIf(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      if( node->child[0] != NULL ) {
         if( node->child[0]->isConst == true ) {
            if( node->child[0]->dataType != BoolDT ) {
               printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", node->lineNum, convertDTStr(node->child[0]));
               errs += 1;     
            }
            return;
         }
         if( node->child[0]->nodeType == IdNT ) {
            Node * condition = fetchSymbol(node->child[0], ST);
            if( condition != NULL ) {
               if( condition->dataType != BoolDT ) {
                  printf("ERROR(%d): Expecting Boolean test condition in if statement but got type %s.\n", node->lineNum, convertDTStr(condition));
                  errs += 1;
               }
            }
            return;
         }
         if( node->child[0]->nodeType == AssignNT ) {
            node->child[0]->isVisited = true;
            Node * condition = processAssign(node->child[0], ST);
            if( condition != NULL ) {
               if( condition->dataType != BoolDT ) {
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
Node * processAND(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * lhs = node->child[0];
      Node * rhs = node->child[1];
      Node * lsym = NULL;
      Node * rsym = NULL;

      if( lhs != NULL ) {
         lhs->isVisited = true;
         lsym = processLHS(lhs, ST);
         if( lsym != NULL ) {
            lsym->isUsed = true;
            lsym->isInit = true;
            if( lsym->dataType != BoolDT ) {
               printf("ERROR(%d): '%s' requires operands of type bool but lhs is of type %s.\n",
               node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
      }
      if( rhs != NULL) {
         rhs->isVisited = true;
         rsym = processRHS(rhs, ST);
         if( rsym != NULL ) {
            rsym->isUsed = true;
            rsym->isInit = true;
            if( rsym->dataType != BoolDT ) {
               printf("ERROR(%d): '%s' requires operands of type bool but rhs is of type %s.\n",
               node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
      }
      if( lsym != NULL && rsym != NULL ) {
         if( lsym->isArray == true || rsym->isArray == true ) {
               printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
               errs += 1;
         }
         
         if( lsym != NULL ) { lsym->isIndexed = false; }
         if( rsym != NULL ) { rsym->isIndexed = false; }
         return lsym;
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS OR              
   ================================================== */
Node * processOR(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * lhs = node->child[0];
      Node * rhs = node->child[1];
      Node * lsym = NULL;
      Node * rsym = NULL;

      if( lhs != NULL ) {
         lhs->isVisited = true;
         lsym = processLHS(lhs, ST);
         if( lsym != NULL ) {
            lsym->isUsed = true;
            lsym->isInit = true;
            if( lsym->dataType != BoolDT ) {
               printf("ERROR(%d): '%s' requires operands of type bool but lhs is of type %s.\n",
               node->lineNum, node->literal, convertDTStr(lsym));
               errs += 1;
            }
         }
      }
      if( rhs != NULL) {
         rhs->isVisited = true;
         rsym = processRHS(rhs, ST);
         if( rsym != NULL ) {
            rsym->isUsed = true;
            rsym->isInit = true;
            if( rsym->dataType != BoolDT ) {
               printf("ERROR(%d): '%s' requires operands of type bool but rhs is of type %s.\n",
               node->lineNum, node->literal, convertDTStr(rsym));
               errs += 1;
            }
         }
      }
      if( lsym != NULL && rsym != NULL ) {
         return lsym;
      }      
   }
   return NULL;
}
/* ==================================================
   PROCESS NOT              
   ================================================== */
Node * processNOT(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * val = node->child[0];
      if( val != NULL ) {
         if( val->nodeType == IdNT ) {
            Node * sym = fetchSymbol(val, ST);
            if( sym != NULL ) {
               sym->isUsed = true;
               if( sym->isArray == true ) {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if( sym->dataType != BoolDT ) {
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
Node * processQUES(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      Node * val = node->child[0];
      if( val != NULL ) {
         if( val->nodeType == IdNT ) {
            Node * sym = fetchSymbol(val, ST);
            if( sym != NULL ) {
               sym->isUsed = true;
               if( sym->isArray == true ) {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
               if( sym->dataType != IntDT ) {
                  printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n",
                  node->lineNum, node->literal, convertDTStr(sym));
                  errs += 1;
               }
               return sym;
            }
         }
         else if ( val->nodeType == QuesNT ) 
         {
            val->isVisited == true;
            Node * sym = processQUES(val, ST);
            if( sym != NULL ) {
               sym->isUsed = true;
               return sym;
            }
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS SIZE OF              
   ================================================== */
Node * processSizeOf(Node * node, SymbolTable * ST)  {
   if( node != NULL ) {
      Node * val = node->child[0];
      if( val != NULL ) {
         if( val->nodeType == IdNT ) {
            Node * sym = fetchSymbol(val, ST);
            if( sym != NULL ) {
               sym->isUsed = true;
               checkIfInit(sym, node, ST);
               if( sym->isArray == false ) {
                  printf("ERROR(%d): The operation '%s' only works with arrays.\n", node->lineNum, node->literal);
                  errs += 1;
               }
            }
         }
      }
   }
   return NULL;
}
/* ==================================================
   PROCESS RANGE              
   ================================================== */
void processRange(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      for( int i = 0; i < node->childC; i++ ) {
         if( node->child[i] != NULL ) {
            Node * sym = processLHS(node->child[i], ST);
            if( sym != NULL ) {
               if( sym->isArray == true && sym->isIndexed == false ) {
                  printf("ERROR(%d): Cannot use array in position %d in range of for statement.\n", 
                  node->lineNum, i + 1);
                  errs += 1;
               }
               if( sym->dataType != IntDT ) {
                  printf("ERROR(%d): Expecting type int in position %d in range of for statement but got type %s.\n",
                  node->lineNum, i + 1, convertDTStr(sym));
                  errs += 1;
               }
               if( i <= 1 && sym->nodeType == FuncNT ) {
                  printf("ERROR(%d): Cannot use function '%s' as a variable.\n", node->lineNum, sym->literal);
                  errs += 1;
               }
            }
         }
      }
   }
}
/* ==================================================
   CHECK MAIN              
   ================================================== */
   void checkMain(SymbolTable * ST) {
      ST->checkUnusedGlobalVars();
      if( hasMain == false ) {
         printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n");
         errs += 1;
      }
   }
/* ==================================================
   CHECK PASSED PARMS              
   ================================================== */
void checkPassedParams(Node * node, Node * function, map<int, Node*> passedParms) {
   for(int i = 0; i < passedParms.size(); i++ ) {
      //printf("%s\n", function->ParmList[i]->literal);
      //printf("%s\n", passedParms[i]->literal);
      if( function->ParmList[i] != NULL && passedParms[i] != NULL ) {
         if( function->ParmList[i]->dataType != passedParms[i]->dataType ) {
            printf("ERROR(%d): Expecting type %s in parameter %d of call to '%s' declared on line %d but got type %s.\n",
            node->lineNum, convertDTStr(function->ParmList[i]), i + 1, function->literal, function->lineNum, convertDTStr(passedParms[i]));
            errs += 1;
         }
         if( passedParms[i]->isIndexed == false && passedParms[i]->isArray == true ) {
            if( function->ParmList[i]->isArray == false ) {
               printf("ERROR(%d): Not expecting array in parameter %d of call to '%s' declared on line %d.\n",
               node->lineNum, i + 1, function->literal, function->lineNum);
               errs += 1;       
            }     
         }
         if( passedParms[i]->isArray == false ) {
            if( function->ParmList[i]->isArray == true ) {
               printf("ERROR(%d): Expecting array in parameter %d of call to '%s' declared on line %d.\n",
               node->lineNum, i + 1, function->literal, function->lineNum);
               errs += 1;               
            }
         }
      }
   }
}