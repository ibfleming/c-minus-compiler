#include "semantics.h"
#include "scope.h"
#include "parser.tab.h"

//////////////////////////////////////////////////
extern int warns; // Warning counter, initialized to 0
extern int errs; // Error counter, initialized to 0
//////////////////////////////////////////////////

// DEBUGGING
bool smDebug = false;

//////////////////////////////////////////////////
//                   GLOBALS                    // 
//////////////////////////////////////////////////
Node * parentScope = NULL;            // Set to the function node after insertion
Node * parentComp = NULL;
int nestedASGN = 0;              // Nested ASGN statements a <= b <= c
int nestedOP = 0;                 
int cStmts = 0;                  // Counter for compound statements
int inFor = 0;               // Whether or not we are in a For scope
bool inFunction = false;         // Are we in a function?
bool hasMain = false;            // Does this program have a main()?
int i = 0;
//////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        SEMANTIC ANALYSIS                                         //
//////////////////////////////////////////////////////////////////////////////////////////////////////
void semanticAnalysis(Node * AST, SymbolTable * ST) {

   if( AST == NULL ) return;
   Node * curr = AST;

   //////////////////////////////////////////////////
   //                SCOPE INSERTION               // 
   //////////////////////////////////////////////////
   if( isScope(curr) ) {
      if( ST->depth() == 1 && curr->nodeType == FuncNT ) {
         /* 
         If we encounter a scope symbol. If we are in Global and the scope is
         a Function then enter. Cannot enter any any scope other than a function in Global.
         */
        // Check if there is a main() function with no parameters!
         if( string(curr->literal).compare(string("main")) == 0 && curr->child[0] == NULL ) { hasMain = true; }
         if( ST->lookupGlobal(string(curr->literal)) != NULL ) { 
            checkDuplicate(curr, ST);
            string dupl = string(curr->literal) + "(" + to_string(i) + ")";
            ST->insertGlobal(dupl, curr);
            ST->enter(dupl);
            inFunction = true;
            parentScope = curr; 
         }
         else {
            ST->insertGlobal(string(curr->literal), curr);
            ST->enter(string(curr->literal));
            inFunction = true;
            parentScope = curr;
         }

      }
      else {
         if( curr->nodeType == CompoundNT && inFunction == true ) {
            if( cStmts == 0 ) {
               // Increment for the initial compound statement but do not enter, it's a part of the Function scope
               cStmts = 1;
            }
            else if ( cStmts >= 1 && inFor == 0 )
            {
               // If its not the initial compound, enter and increment.
               ST->enter(string(curr->literal));
               cStmts++;
               parentComp = curr;
            }
            else if ( inFor > 0 )
            {
               cStmts++;
            }
            
         }
         else if ( curr->nodeType == ToNT && inFunction == true )
         {
            // If we detect a For loop within a Function, treat it as if its a Compound
            ST->enter(string(curr->literal));
            parentComp = curr;
            inFor++;

            // Set the Var in the For loop to being  Used and is Initialized
            if( curr->child[0] != NULL ) {
               curr->child[0]->isInit = true;
               curr->child[0]->isUsed = false;
            }
         }
      }
   }
   //////////////////////////////////////////////////
   //               SYMBOL INSERTION               // 
   //////////////////////////////////////////////////
   if( isSymbol(curr) ) {
      if( ST->depth() == 1 ) {
         // Insert if this symbol is not declared already.
         if( checkDuplicate(curr, ST) == false ) {
            ST->insertGlobal(string(curr->literal), curr);
            curr->isInit = true;
            curr->isUsed = true;
            for (int i = 0; i < curr->childC; i++)
            {
               if( curr->child[0] != NULL ) {
                  if( curr->child[0]->nodeType == IdNT ) {
                     if( checkIfDeclared(curr->child[0], ST, true) ) {
                        Node * tmp = fetchSymbol(curr->child[0], ST);
                        tmp->isUsed = true;
                     }
                  }
               }
            }
         }
      }
      else if ( ST->depth() > 1 ) {
         // Insert if this symbol is not declared already.
         if( checkDuplicateScope(curr, ST) == false ) {
            ST->insert(string(curr->literal), curr);
            for (int i = 0; i < curr->childC; i++)
            {
               if( curr->child[0] != NULL ) {
                  if( curr->child[0]->nodeType == IdNT ) {
                     if( checkIfDeclared(curr->child[0], ST, true) ) {
                        Node * tmp = fetchSymbol(curr->child[0], ST);
                        tmp->isUsed = true;
                     }
                  }
               }
            }
            
         }
      }
      // PRINT TABLE PER SYMBOL INSERTION
      if( smDebug ) ST->print(printData);
      //
   }
   //////////////////////////////////////////////////////////////////////////////////////////////////////
   //                         ANALYZE OTHER SYMBOLS (NON-SYMBOLS AND SCOPES)                           //
   //////////////////////////////////////////////////////////////////////////////////////////////////////
   
   //////////////////////////////////////////////////
   //                STANDALONE VARS               // 
   //////////////////////////////////////////////////
   if( curr->nodeType == IdNT && curr->sibling != NULL && curr->isNested == false ) {
      if( checkIfDeclared(curr, ST, true) ) {
            Node * symbol = fetchSymbol(curr, ST);
            checkIfInit(symbol, curr, ST);
            symbol->isUsed = true;
            symbol->isInit = true;
      }
   }
   //////////////////////////////////////////////////
   //              STANDALONE ARRAYS               // 
   //////////////////////////////////////////////////
   if( curr->nodeType == ArrNT && curr->sibling != NULL ) {
      Node * array = processArray(curr, ST);
      array->isUsed = true;
      array->isInit = true;
   }
   //////////////////////////////////////////////////
   //                    CALLNT                    // 
   //////////////////////////////////////////////////
   if( curr->nodeType == CallNT && curr->isNested == false ) {
      processCall(curr, ST);
   }
   //////////////////////////////////////////////////
   //                 ASSIGNMENTS                  // 
   //////////////////////////////////////////////////   
   if( curr->nodeType == AssignNT && curr->isNested == false) {
      processAssign(curr, ST);
   }
   //////////////////////////////////////////////////
   //                  OPERATORS                   // 
   //////////////////////////////////////////////////   
   if( curr->nodeType == OpNT && curr->isNested == false ) {
      processOps(curr, ST);
   }
   //////////////////////////////////////////////////
   //                    CHSIGN                    // 
   ////////////////////////////////////////////////// 
   if( curr->nodeType == SignNT && curr->isNested == false ) {
      processCHSIGN(curr, ST);
   }
   //////////////////////////////////////////////////
   //                    SIZEOF                    // 
   //////////////////////////////////////////////////
   if( curr->nodeType == SizeOfNT ) {  // CAN BE A FUNCTION?
      Node * ID = curr->child[0];
      if( ID->nodeType == IdNT ) {
         if( checkIfDeclared(ID, ST, true) ) {
            ID = fetchSymbol(ID, ST);
            checkIfInit(ID, curr, ST);
            ID->isUsed = true;
            if( ID->isArray == false ) {
               printf("ERROR(%d): The operation '%s' only works with arrays.\n", curr->lineNum, curr->literal);
               errs += 1;
            }
         }
      }
   }
   //////////////////////////////////////////////////
   //                   QUESTION                   // 
   //////////////////////////////////////////////////
   if( curr->nodeType == QuesNT ) {
      Node * ID = curr->child[0];
      if( ID->nodeType == IdNT ) {
         if( checkIfDeclared(ID, ST, true) ) {
            ID = fetchSymbol(ID, ST);
            ID->isUsed = true;
            if( ID->dataType != IntDT ) {
               printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n", curr->lineNum, curr->literal, convertDTStr(ID));
               errs += 1;
            }
            printNotArray(ID, curr);
         }
      }
   }
   //////////////////////////////////////////////////
   //                     AND                      // 
   //////////////////////////////////////////////////
   if( curr->nodeType == AndNT && curr->isNested == false ) {
      processAND(curr, ST);
   }
   //////////////////////////////////////////////////
   //                     OR                       // 
   //////////////////////////////////////////////////
   if( curr->nodeType == OrNT && curr->isNested == false) {
      processOR(curr, ST);
   }
   //////////////////////////////////////////////////
   //                     NOT                      // 
   //////////////////////////////////////////////////
   if( curr->nodeType == NotNT && curr->isNested == false ) {
      processNOT(curr, ST);     
   }
   //////////////////////////////////////////////////
   //                    RANGE                     // 
   //////////////////////////////////////////////////
   if( curr->nodeType == RangeNT ) {
      for(int i = 0; i < curr->childC; i++ ) {
         Node * child = curr->child[i];
         if( child != NULL ) {
            if( child->nodeType == IdNT ) {
               checkIfDeclared(child, ST, false);
            }
            if( child->nodeType == OpNT) {
               child->isNested = true;
               processOps(child, ST);
            }
         }
      }
   }
   //////////////////////////////////////////////////
   //                    RETURN                    // 
   //////////////////////////////////////////////////
   if( curr->nodeType == ReturnNT && inFunction == true ) {
      Node * child = curr->child[0];
      if( child != NULL ) {
         if( child->nodeType == IdNT ) {
            if( checkIfDeclared(child, ST, true) ) {
               child = fetchSymbol(child, ST);
               child->isUsed = true;
               if( child->isArray == true ) {
                  printf("ERROR(%d): Cannot return an array.\n", curr->lineNum);
                  errs += 1;
               }
            }
         }
         if( child->nodeType == OpNT ) {
            child->isNested = true;
            processOps(child, ST);
         }
      }
   }
   //////////////////////////////////////////////////
   // RECURSIVE CALLING THROUGH CHILDREN & SIBLING // 
   //////////////////////////////////////////////////
   //printf("cStmts: %d, [%d]\n", cStmts, curr->lineNum);
   if( curr != NULL ) {
      for( int i = 0; i < curr->childC; i++ ) {
         if( curr->child[i] != NULL ) semanticAnalysis(curr->child[i], ST);
      }

      if( curr->nodeType == ToNT ) {
         if( curr->child[2] != NULL ) {
            if(curr->child[2]->nodeType != CompoundNT ) {
            ST->checkUnusedVars();
            if( smDebug ) ST->print(printData);
            ST->leave();
            inFor--;
            }
         }
      }

      if( curr->sibling != NULL ) semanticAnalysis(curr->sibling, ST);
   }
   //////////////////////////////////////////////////
   //           EOF/END OF SCOPE CHECKING          // 
   //////////////////////////////////////////////////
   if( curr->nodeType == CompoundNT ) {
      if( cStmts > 1 && inFor >= 1 ) {
         if( ST->depth() != 1 ) {
            ST->checkUnusedVars();
            if( smDebug ) ST->print(printData); 
            ST->leave();
            inFor--;
            cStmts--;
         }
      } else if( cStmts > 1 && inFor == 0 ) {
         if( ST->depth() != 1 ) {
            ST->checkUnusedVars();
            if( smDebug ) ST->print(printData); 
            ST->leave();
            cStmts--;
         }         
      } else if( cStmts == 1 ) {
         if( ST->depth() != 1 ) {
            ST->checkUnusedVars();
            if( smDebug ) ST->print(printData);
            ST->leave();
            cStmts = 0;
            inFunction = false;
         }
      }
   }
   if ( curr->nodeType == FuncNT || curr->nodeType == VarNT || curr->nodeType == VarArrNT ) {
      if( curr->sibling == NULL && cStmts == 0 )
      {
            // If current FuncNT node equals the last FuncNT declared then EOF. Error check.
         if( string(curr->literal) == string(parentScope->literal) )
         {
            ST->checkUnusedGlobalVars(); // Check if Global declarations were used or not.
            if( !hasMain ) { printf("ERROR(LINKER): A function named 'main()' must be defined.\n"); errs += 1; }
         }
      }
   }
   //////////////////////////////////////////////////
}
//////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                  SEMANTIC ANALYSIS FUNCTIONS                                     //
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////
//                 PROCESS CALL                 // 
//////////////////////////////////////////////////
Node * processCall(Node * node, SymbolTable * ST) {
   Node * func = fetchSymbol(node, ST);
   
   if( func != NULL ) {
      func->isInit = true;
      if( func->nodeType != FuncNT ) {
         printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", node->lineNum, func->literal);
         errs += 1;
      }
   }
   bool decl = checkIfDeclared(node, ST, true);
   Node * child = node->child[0];
   while( child != NULL ) {
      if( child->nodeType == IdNT ) {
         child->isNested = true;
         if( checkIfDeclared(child, ST, true) ) {
            Node * tmp = fetchSymbol(child, ST);
            checkIfInit(tmp, node, ST);
            tmp->isUsed = true;
            tmp->isInit = true;
            if( tmp->nodeType == FuncNT ) {
               printf("ERROR(%d): Cannot use function \'%s\' as a variable.\n", node->lineNum, tmp->literal);
               errs += 1;
            }
         }
      }
      if( child->nodeType == OpNT ) {
         child->isNested = true;
         processOps(child, ST);
      }
      if( child->nodeType == ArrNT ) {
         checkIfDeclared(child->child[0], ST, true);
         Node * tmp = processArray(child, ST);
         tmp->isUsed = true;
      }
      if( child->nodeType == SignNT ) {
         child->isNested = true;
         processCHSIGN(child, ST);
      }
      if( child->nodeType == CallNT ) {
         child->isNested = true;
         processCall(child, ST);
      }
      if( child->nodeType == AndNT ) {
         child->isNested = true;
         processAND(child, ST);
      }
      if( child->nodeType == OrNT ) {
         child->isNested = true;
         processOR(child, ST);
      }
      if( child->nodeType == NotNT ) {
         child->isNested = true;
         processNOT(child, ST);
      }
      if( child->nodeType == AssignNT ) {
         child->isNested = true;
         processAssign(child, ST);
      }
      if( child->sibling != NULL ) {
         child = child->sibling;
      } else {
         child = NULL;
      }
   }
   if( decl == true ) return node;
   return NULL;
}
//////////////////////////////////////////////////
//                PROCESS CHSIGN                // 
//////////////////////////////////////////////////
Node * processCHSIGN(Node * node, SymbolTable * ST)
{
   Node * ID = node->child[0];
   if( ID->nodeType == IdNT ) {
      if( checkIfDeclared(ID, ST, true) ) {
         ID = fetchSymbol(ID, ST);
         checkIfInit(ID, node, ST);
         ID->isInit = true;
         ID->isUsed = true;
         if( ID->dataType != IntDT ) {
            printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n", node->lineNum, node->literal, convertDTStr(ID));
            errs += 1;
         }
         if( ID->isArray == true ) {
            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
            errs += 1;
         }
      }
   }
   else if( ID->isConst == true ) {
      if( ID->dataType != IntDT ) {
         printf("ERROR(%d): Unary '%s' requires an operand of type int but was given type %s.\n", node->lineNum, node->literal, convertDTStr(ID));
         errs += 1;
      }      
   }
   else if( ID->nodeType == SignNT ) {
      ID = processCHSIGN(ID, ST);
   }
   else if( ID->nodeType == OpNT ) {
      ID = processOps(ID, ST);
   }
   return ID;
} 
//////////////////////////////////////////////////
//                 PROCESS LHS                  //
//////////////////////////////////////////////////
Node * processLHS(Node * node, SymbolTable * ST) 
{
   if( node != NULL ) {
      if( node->isConst == true ) {
         return node;
      }
      switch ( node->nodeType )
      {
      ///////////////////////
      //      LHS: ID      //
      ///////////////////////
      case IdNT:
         if( checkIfDeclared(node, ST, false) ) {
            Node * ID = fetchSymbol(node, ST);
            ID->isUsed = true; // Maybe incorrect!
            if( ID->nodeType == FuncNT ) {
               printf("ERROR(%d): Cannot use function '%s' as a variable.\n", node->lineNum, ID->literal);
               errs += 1;               
            }
            return ID;
         } else {
            //printf("processLHS(): Symbol \'%s\' not declared. Returning NULL.\n", node->literal);
            return NULL;
         }
      ///////////////////////
      //     LHS: ARRAY    //
      ///////////////////////
      case ArrNT:
         if( node != NULL ) {
            Node * ID = node->child[0];
            if( checkIfDeclared(ID, ST, false) ) {
               ID = fetchSymbol(ID, ST);
               ID->isInit = true;
               ID = processArray(node, ST);
            }
            else {
               printf("ERROR(%d): Cannot index nonarray '%s'.\n", node->lineNum, ID->literal);
               errs += 1;
               checkIfDeclared(ID, ST, true);
            }
            Node * index = node->child[1];
            if( index != NULL ) {
               if( index->nodeType == ArrNT ) { // INDEX FOR ARRAY IS ANOTHER ARRAY!
                  Node * indexArr = index->child[0];
                  if( indexArr != NULL && indexArr->nodeType == IdNT ) {
                     indexArr = fetchSymbol(indexArr, ST);
                     indexArr->isInit = true;
                     indexArr->isUsed = true;
                     if( indexArr->dataType != IntDT ) {
                        printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, ID->literal, convertDTStr(indexArr));
                        errs += 1;
                     }
                  }
               }
            }
            return ID;
         }
      ///////////////////////
      //     LHS: CALL     //
      ///////////////////////
      case CallNT:
         return processCall(node, ST);
      case SignNT:
      return processCHSIGN(node, ST);
      default:
         //printf("Returning NULL for process LHS!\n");
         return NULL;
      }
   }
   return NULL;
}
//////////////////////////////////////////////////
//                 PROCESS RHS                  //
//////////////////////////////////////////////////
Node * processRHS(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      if( node->isConst == true ) {
         return node;
      }
      switch( node->nodeType ) {
         ///////////////////////
         //      RHS: ID      //
         ///////////////////////
         case IdNT:
            if( checkIfDeclared(node, ST, false) ) {
               Node * ID = fetchSymbol(node, ST);
               ID->isUsed = true;
               return ID;
            }
            else {
               return NULL;
            }
            break;
         ///////////////////////
         //     RHS: ARRAY    //
         ///////////////////////
         case ArrNT:
            if( node->child[0]->nodeType == ArrNT ) {
               Node * arr = processArray(node->child[0], ST);
               node->dataType = arr->dataType;
               arr->isUsed= true;
               return arr;
            }
            else {
               Node * arr = fetchSymbol(node->child[0], ST);
               arr = processArray(node, ST);
               arr->isUsed = true;
               return arr;
            }
            break;
         ///////////////////////
         //     RHS: CALL     //
         ///////////////////////
         case CallNT:
            return processCall(node, ST);
         ///////////////////////
         //      RHS: OR      //
         ///////////////////////
         case OrNT:
            return NULL;
         ///////////////////////
         //     RHS: AND      //
         ///////////////////////
         case AndNT:
            return NULL;
         ///////////////////////
         //     RHS: NOT      //
         ///////////////////////
         case NotNT:
            return NULL;
         default:
            //printf("Returning NULL for process RHS!\n");
            return NULL;
      }
   }
   return NULL;
}
//////////////////////////////////////////////////
//                 PROCESS ARRAY                //
//////////////////////////////////////////////////
// Might need to implement Init check to function
Node * processArray(Node * node, SymbolTable * ST) {
   if( node->nodeType == ArrNT ) {
      Node * ID = node->child[0];
      Node * index = node->child[1];
      if( ID->nodeType == IdNT ) {
         if( checkIfDeclared(ID, ST, false) == true ) {
            ID = fetchSymbol(ID, ST);
            if( ID->isStatic == true ) { ID->isInit = true; }
            if( node->sibling != NULL ) { checkIfInit(ID, node, ST); }
            //checkIfInit(ID, node, ST);
            node->dataType = ID->dataType;
            if( ID->nodeType == FuncNT ) {
               printf("ERROR(%d): Cannot use function '%s' as a variable.\n", node->lineNum, ID->literal);
               errs += 1;
            }
            if( ID->isArray == false ) {     // CHECK IF ID IS AN ARRAY!
               printf("ERROR(%d): Cannot index nonarray '%s'.\n", node->lineNum, ID->literal);
               errs += 1;
            }
         }
         else if( checkIfDeclared(ID, ST, false) == false ) {
            printf("ERROR(%d): Cannot index nonarray '%s'.\n", node->lineNum, ID->literal);
            errs += 1;   
         }
         if( index != NULL ) {
            ID->isIndexed = true;
            if( index->nodeType == IdNT ) {
               if( checkIfDeclared(index, ST, false) ) {
                  index = fetchSymbol(index, ST);
                  index->isUsed = true;
                  if( index->dataType != IntDT ) {
                     printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, ID->literal, convertDTStr(index));
                     errs += 1;
                  }
                  if( index->isArray == true ) {
                     printf("ERROR(%d): Array index is the unindexed array '%s'.\n", node->lineNum, index->literal);
                     errs += 1;   
                  }
               }
            }
            else if( index->nodeType == ArrNT ) {
               processArray(index, ST);
            }
            else if( index->nodeType == OpNT ) {
               processOps(index, ST);
            }
            else if( index->isConst == true ) {
               if( index->dataType != IntDT ) {
                  printf("ERROR(%d): Array '%s' should be indexed by type int but got type %s.\n", node->lineNum, ID->literal, convertDTStr(index));
                  errs += 1;
               }            
            }
         }
         return ID;
      }      
   }
   return NULL;
}
//////////////////////////////////////////////////
//                  PROCESS OPS                 //
//////////////////////////////////////////////////
Node * processOps(Node * node, SymbolTable * ST) {

   Node * lhs = node->child[0];
   Node * rhs = node->child[1];

   //printf("LHS: %s %d\n", lhs->literal, node->lineNum);
   //printf("RHS: %s %d\n", rhs->literal, node->lineNum);

   if( lhs != NULL && rhs != NULL ) 
   {
      ////////////////////////////////////////
      //     OPERATORS (EQL. DT & ARRAY)    //
      ////////////////////////////////////////
      if( node->tknClass == EQL || node->tknClass == NEQ || node->tknClass == LESS || 
          node->tknClass == GREAT || node->tknClass == LEQ || node->tknClass == GEQ )
      {
         ///////////////////////
         //    CHECK DECL.    // => Will print declaration error.
         ///////////////////////
         bool lhsD, rhsD;
         if( lhs->isConst == false && lhs->nodeType == IdNT ) { lhsD = checkIfDeclared(lhs, ST, true); }
         if( rhs->isConst == false && rhs->nodeType == IdNT ) { rhsD = checkIfDeclared(rhs, ST, true); }
         if( lhsD == false && lhs->nodeType == IdNT ) {}//printf("LHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         if( rhsD == false && rhs->nodeType == IdNT ) {} //printf("RHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);

         ///////////////////////
         //    PROCESS RHS    //
         ///////////////////////
         Node * rsym;
         if( rhs->nodeType == AssignNT ) {
            rhs->isNested = true;
            rsym = processAssign(rhs, ST);
            if( rsym != NULL ) {
               rsym->isUsed = true;
               // SINCE END OF ASSIGN SETS INDEXED TO FALSE SET IT AGAIN AFTER IF ITS AN ARRAY!
               if( rsym->isArray == true ) {   
                  rsym->isIndexed = true;
               }
               }
         } else if( rhs->nodeType == OpNT ) {
               rhs->isNested = true;
               rsym = processOps(rhs, ST);
               if( rsym != NULL ) {
                  rsym->isUsed = true;
                  if( rsym->isArray == true ) {   
                     rsym->isIndexed = true;
                  }
               }
         } else {
            rsym = processRHS(rhs, ST);
            if( rsym != NULL ) {
               checkIfInit(rsym, node, ST);
            }
         }
         ///////////////////////
         //    PROCESS LHS    //
         ///////////////////////
         Node * lsym = processLHS(lhs, ST);
         if( lsym != NULL && lsym->isConst == false) {
            lsym->isUsed = true;
         }
         ////////////////////////////////////////
         //     ERROR CHECK FOR BOTH SIDES     //
         ////////////////////////////////////////
         if( lsym != NULL ) {
            if( rsym != NULL ) {
               rsym->isInit = true;
               //printf("RHS: %s %s %d\n", rsym->literal, convertNTStr(rsym), node->lineNum );
               ////////////////////////////////////////
               //           DATATYPE CHECK           //
               ////////////////////////////////////////
               printRequiresDT(lsym, rsym, node);
               ////////////////////////////////////////
               //        ARRAY ERROR CHECKING        //
               ////////////////////////////////////////
               printArrayLHSRHS(lsym, rsym, node);

               // Might be that these Vars be used again so make sure they aren't Indexed which is logical.
               if( rsym->isIndexed == true) rsym->isIndexed = false;
            }
            // Might be that these Vars be used again so make sure they aren't Indexed which is logical.
            if( lsym->isIndexed == true) lsym->isIndexed = false;
            return lsym;
         }
      }
      ////////////////////////////////////////
      //    OPERATORS (INT & NONARRAY)      //
      ////////////////////////////////////////
      if( node->tknClass == ADD || node->tknClass == SUB || node->tknClass == MUL ||
         node->tknClass == DIV || node->tknClass == MOD )
      {
         ///////////////////////
         //    CHECK DECL.    // => Will print declaration error.
         ///////////////////////
         bool lhsD, rhsD;
         if( lhs->isConst == false && lhs->nodeType == IdNT ) { lhsD = checkIfDeclared(lhs, ST, true); }
         if( rhs->isConst == false && rhs->nodeType == IdNT ) { rhsD = checkIfDeclared(rhs, ST, true); }
         if( lhsD == false && lhs->nodeType == IdNT ) {}//printf("LHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         if( rhsD == false && rhs->nodeType == IdNT ) {}//printf("RHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         ///////////////////////
         //    PROCESS LHS    //
         ///////////////////////
         Node * lsym;
         if( lhs->isConst == false ) {
            if( lhs->nodeType == OpNT ) {
               lhs->isNested = true;
               lsym = processOps(lhs, ST);
            }
            else if( lhs->nodeType == AssignNT ) {
               lhs->isNested = true;
               lsym = processAssign(lhs, ST);
            }
            else {
               lsym = processLHS(lhs, ST);
               if( lsym != NULL ) {
                  // Check if Init?
                  //checkIfInit(lsym, node, ST);
                  lsym->isInit = true;
               }
            }
            if( lhs != NULL ) {
               if( lsym->dataType != IntDT ) {
                  printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n", node->lineNum, node->literal, convertDTStr(lsym));
                  errs += 1;
               }
            }
         }
         ///////////////////////
         //    PROCESS RHS    //
         ///////////////////////
         Node * rsym = processRHS(rhs, ST);
         if( rsym != NULL ) {
            if( rsym->dataType != IntDT ) {
                  printf("ERROR(%d): '%s' requires operands of type int but rhs is of type %s.\n", node->lineNum, node->literal, convertDTStr(rsym));
                  errs += 1;
            }
            checkIfInit(rsym, node, ST);
         }
         ////////////////////////////////////////
         //     ERROR CHECK FOR BOTH SIDES     //
         ////////////////////////////////////////  
         if( lsym != NULL ) {
            lsym->isUsed = true;
            if( rsym != NULL ) {
               rsym->isUsed = true;
               //printf("\n");
               //printf("\tO.ASGN: %s, %s [%d]\n", convertNTStr(node->child[0]), convertNTStr(node->child[1]), node->lineNum);
               //printf("\tASGN: %s, %s, %s, %s [%d]\n", convertNTStr(lhs), convertNTStr(rhs), convertDTStr(lhs), convertDTStr(rhs), node->lineNum);
               //printf("%s %s\n", lhs->literal, rhs->literal);
               // ERROR CHECK STUFF!

               if( lsym->isArray == true || rsym->isArray == true ) {
                  printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
                  errs += 1;               
               }
            }
            return lsym;
         }  
      }


   }
   return NULL;
}
//////////////////////////////////////////////////
//              PROCESS ASSIGNMENTS             //
//////////////////////////////////////////////////
Node * processAssign(Node * node, SymbolTable * ST) {
   
   //printf("HERE! [%d]\n", node->lineNum);
   Node * lhs = node->child[0];
   Node * rhs = node->child[1];

   if( lhs != NULL && rhs != NULL ) 
   {
      ////////////////////////////////////////
      //          ASSIGNMENT (ASGN)         //
      ////////////////////////////////////////
      if( node->tknClass == ASGN ) {
         ///////////////////////
         //    CHECK DECL.    // => Will print declaration error.
         ///////////////////////
         bool lhsD, rhsD;
         if( lhs->isConst == false && lhs->nodeType == IdNT ) { lhsD = checkIfDeclared(lhs, ST, true); }
         if( rhs->isConst == false && rhs->nodeType == IdNT ) { rhsD = checkIfDeclared(rhs, ST, true); }
         if( lhsD == false && lhs->nodeType == IdNT ) {}//printf("LHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         if( rhsD == false && rhs->nodeType == IdNT ) {}//printf("RHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         ///////////////////////
         //    PROCESS LHS    //
         ///////////////////////
         Node * lsym;
         if( lhs->isConst == false ) {
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) {
               node->dataType = lsym->dataType;
               if( lhs->literal == rhs->literal ) {
                  lsym->isInit = false;
               } else {
                  lsym->isInit = true;
               }
            }
            if( lhs->nodeType == FuncNT ) return lsym;
         }
         ///////////////////////
         //    PROCESS RHS    //
         ///////////////////////
         Node * rsym;
         if( rhs->nodeType == AssignNT ) {
            rhs->isNested = true;
            rsym = processAssign(rhs, ST);
            if( rsym != NULL && rsym->isArray == true ) rsym->isIndexed = true;
          }
         else if( rhs->nodeType == OpNT ) {
            rhs->isNested  = true;
            rsym = processOps(rhs, ST);
            if( rsym != NULL && rsym->isArray == true ) rsym->isIndexed = true;
         }
         else if( rhs->nodeType == CallNT ) {
            rhs->isNested = true;
            rsym = processRHS(rhs, ST);
         }
         else {
            rsym = processRHS(rhs, ST);
         }
         if( rsym != NULL && rsym->isConst == false ) {
            bool init = checkIfInit(rsym, node, ST);
            if( init == false ) rsym->isInit = true;
         }
         ////////////////////////////////////////
         //     ERROR CHECK FOR BOTH SIDES     //
         //////////////////////////////////////// 
         if( lsym != NULL ) {
            lsym->isUsed = true;
            if( rsym != NULL ) {
               rsym->isUsed = true;
               //if( lsym->literal == rsym->literal && node->child[1]->nodeType == ArrNT ) { rsym->isInit = false; }
               //if( node->child[1]->nodeType != ArrNT && !checkIfInit(rsym, node, ST) ) { rsym->isInit = true; }

               ////////////////////////////////////////
               //           DATATYPE CHECK           //
               ////////////////////////////////////////
               if( lsym->dataType != UndefinedDT || rsym->dataType == UndefinedDT ) printRequiresDT(lsym, rsym, node);
               ////////////////////////////////////////
               //        ARRAY ERROR CHECKING        //
               ////////////////////////////////////////
               printArrayLHSRHS(lsym, rsym, node);

               // Might be that these Vars be used again so make sure they aren't Indexed which is logical.
               if( rsym->isIndexed == true) rsym->isIndexed = false;
            }
            if( lsym->isIndexed == true) lsym->isIndexed = false;
            return lsym;
         }

      }
      ////////////////////////////////////////
      //      ASSIGNMENTS (INT, NO ARR)     //
      ////////////////////////////////////////
      if( node->tknClass == ADDASS || node->tknClass == SUBASS || node->tknClass == MULASS ||
          node->tknClass == DIVASS ) 
      {
         ///////////////////////
         //    CHECK DECL.    // => Will print declaration error.
         ///////////////////////
         bool lhsD, rhsD;
         if( lhs->isConst == false && lhs->nodeType == IdNT ) { lhsD = checkIfDeclared(lhs, ST, true); }
         if( rhs->isConst == false && rhs->nodeType == IdNT ) { rhsD = checkIfDeclared(rhs, ST, true); }
         if( lhsD == false && lhs->nodeType == IdNT ) {} //printf("LHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         if( rhsD == false && rhs->nodeType == IdNT ) {} //printf("RHS SYMBOL IS NOT DECLARED! [%d]\n", node->lineNum);
         ///////////////////////
         //    PROCESS RHS    //
         ///////////////////////
         Node * rsym;
         if( rhs->nodeType == AssignNT ) {
            rhs->isNested = true;
            rsym = processAssign(rhs, ST);
            if( rsym != NULL && rsym->isArray == true ) rsym->isIndexed = true;
          }
         else if( rhs->nodeType == OpNT ) {
            rhs->isNested  = true;
            rsym = processOps(rhs, ST);
            if( rsym != NULL && rsym->isArray == true ) rsym->isIndexed = true;
         }
         else if( rhs->nodeType == CallNT ) {
            rhs->isNested = true;
            rsym = processRHS(rhs, ST);
         }
         else {
            rsym = processRHS(rhs, ST);
            //checkifInit(rsym, node, ST);
         }
         ///////////////////////
         //    PROCESS LHS    //
         ///////////////////////
         Node * lsym;
         if( lhs->isConst == false ) {
            lsym = processLHS(lhs, ST);
            if( lsym != NULL ) {
               lsym->isInit = true;
               //node->dataType = lsym->dataType;
            }
            if( lhs->nodeType == FuncNT ) return lsym;
         }
         ////////////////////////////////////////
         //     ERROR CHECK FOR BOTH SIDES     //
         //////////////////////////////////////// 
         if( lsym != NULL ) {
            lsym->isUsed = true;
            if( rsym != NULL ) {
               rsym->isUsed = true;
               //if( lsym->literal == rsym->literal && node->child[1]->nodeType == ArrNT ) { rsym->isInit = false; }
               //if( node->child[1]->nodeType != ArrNT && !checkIfInit(rsym, node, ST) ) { rsym->isInit = true; return lsym; }

               if( lsym->dataType != IntDT ) {
                  printf("ERROR(%d): '%s' requires operands of type int but lhs is of type %s.\n", node->lineNum, node->literal, convertDTStr(lsym));
                  errs += 1;
               }
               if( rsym->dataType != IntDT ) {
                  printf("ERROR(%d): '%s' requires operands of type int but rhs is of type %s.\n", node->lineNum, node->literal, convertDTStr(rsym));
                  errs += 1;
               }

               // Might be that these Vars be used again so make sure they aren't Indexed which is logical.
               //if( rsym->isIndexed == true) rsym->isIndexed = false;
            }
            //if( lsym->isIndexed == true) lsym->isIndexed = false;
            return lsym;
         }

      }
   }
   ////////////////////////////////////////
   //       INCREMENT & DECREMENT        //
   ////////////////////////////////////////
   else if( lhs != NULL ) {
      if( node->tknClass == INC || node->tknClass == DEC ) {
         if( lhs->nodeType == IdNT ) {
            if( checkIfDeclared(lhs, ST, true) == true ) {
               Node * sym = processLHS(lhs, ST);
               checkIfInit(sym, node, ST);
               sym->isInit = true;
               printNotArray(sym, node);
               return sym;
            }
            else {
               return NULL;
            }
         }
      }   
   }
   return NULL;
}
//////////////////////////////////////////////////
//                 PROCESS AND                  //
//////////////////////////////////////////////////
Node * processAND(Node * node, SymbolTable * ST) {
   Node * lhs = node->child[0];
   Node * rhs = node->child[1];

   if( lhs != NULL && rhs != NULL ) {
      if( rhs->nodeType == IdNT) {
         if( checkIfDeclared(rhs, ST, true) ) {
            rhs = fetchSymbol(rhs, ST);
         }
      }
      if( lhs->nodeType == IdNT ) {
         if( checkIfDeclared(lhs, ST, true) ) {
            lhs = fetchSymbol(lhs, ST);
         }
      }
      if ( lhs->dataType != BoolDT ) 
      {
         printf("ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", node->lineNum, node->literal, convertDTStr(node), convertDTStr(lhs));
         errs += 1;
      }
      if ( rhs->dataType != BoolDT && rhs->dataType != UndefinedDT ) 
      {
         printf("ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", node->lineNum, node->literal, convertDTStr(node), convertDTStr(rhs));
         errs += 1;
      }
      if( lhs->isArray == true || rhs->isArray == true ) {
         printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
         errs += 1;               
      }
      if( lhs != NULL ) {
         return lhs;
      }  
   }
   return NULL;
}
//////////////////////////////////////////////////
//                 PROCESS OR                   //
//////////////////////////////////////////////////
Node * processOR(Node * node, SymbolTable * ST) {
   Node * lhs = node->child[0];
   Node * rhs = node->child[1];

   if( lhs != NULL && rhs != NULL ) {
      if( rhs->nodeType == IdNT) {
         if( checkIfDeclared(rhs, ST, true) ) {
            rhs = fetchSymbol(rhs, ST);
            if( lhs->literal != rhs->literal ) checkIfInit(lhs, node, ST);
            if( rhs->dataType != BoolDT ) {
               printf("ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", 
               node->lineNum, node->literal, convertDTStr(node), convertDTStr(rhs));
               errs += 1;            
            }
         }
      }
      if( lhs->nodeType == IdNT ) {
         if( checkIfDeclared(lhs, ST, true) ) {
            lhs = fetchSymbol(lhs, ST);
            lhs->isInit = true;
            if( lhs->dataType != BoolDT ) {
               printf("ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", 
               node->lineNum, node->literal, convertDTStr(node), convertDTStr(lhs));
               errs += 1;
            }
         }
      }
      if( lhs->isArray == true || rhs->isArray == true ) {
         printf("ERROR(%d): The operation '%s' does not work with arrays.\n", node->lineNum, node->literal);
         errs += 1;               
      }
      if( lhs != NULL ) {
         return lhs;
      }  
   }
   return NULL;
}
//////////////////////////////////////////////////
//                 PROCESS NOT                  //
//////////////////////////////////////////////////
Node * processNOT(Node * node, SymbolTable * ST) {
   Node * child = node->child[0];
   if( child != NULL ) {
      if( child->nodeType == IdNT ) {
         if( checkIfDeclared(child, ST, true) ) {
            child = fetchSymbol(child, ST);
            child->isUsed = true;
            if( child->dataType != BoolDT ) {
               printf("ERROR(%d): Unary '%s' requires an operand of type bool but was given type %s.\n", node->lineNum, node->literal, convertDTStr(child));
               errs += 1;
            }
            printNotArray(child, node);
         }
         return child;
      }
      else if( child->isConst == true ) {
         return child;
      }
   }
   return NULL;
}
//////////////////////////////////////////////////
//          CHECK IF SYMBOL IS DECLARED         //
//////////////////////////////////////////////////
// Returns true if it is a declared symbol, else false.
// I.E. There is a VarNT to that ID, etc.,  Sets DataTypes
bool checkIfDeclared(Node * node, SymbolTable * ST, bool print)
{
   string symbol = string(node->literal);
   Node * global = ST->lookupGlobal(symbol);
   Node * local = ST->lookup(symbol);
   if( global == NULL && local == NULL && print == true ) {
      printf("ERROR(%d): Symbol '%s' is not declared.\n", node->lineNum, symbol.c_str());
      errs += 1;
      return false;
   }
   if( global != NULL ) {
      // Set Datatype?
      node->dataType = global->dataType;
      return true;
   }
   else if ( local != NULL ) {
      // Set Datatype?
      node->dataType = local->dataType;
      return true;
   }
   return false;
}
//////////////////////////////////////////////////
//     CHECK IF SYMBOL IS DECLARED IN SCOPE     //
//////////////////////////////////////////////////
// Returns true if it is a declared symbol, else false.
// I.E. There is a VarNT to that ID, etc.,  Sets DataTypes
bool checkIfDeclaredScope(Node * node, Node * parent, SymbolTable * ST)
{
   string symbol = string(node->literal);
   string scopeName = string(parent->literal);
   Node * global = ST->lookupGlobal(symbol);
   Node * local = ST->lookupScopeName(symbol, scopeName);
   if( global == NULL && local == NULL ) {
      printf("ERROR(%d): Symbol '%s' is not declared.\n", node->lineNum, symbol.c_str());
      errs += 1;
      return false;
   }
   if( global != NULL ) {
      // Set Datatype?
      node->dataType = global->dataType;
   }
   else if ( local != NULL ) {
      // Set Datatype?
      node->dataType = local->dataType;
   }
   return true;
}
//////////////////////////////////////////////////
//         CHECK IF SYMBOL IS DUPLICATE         //
//////////////////////////////////////////////////
// Call this prior to symbol insertion. Returns true if
// symbol is already declared, else false. Apply only to isSymbol() NTs.
bool checkDuplicate(Node * node, SymbolTable * ST) {
   string symbol = string(node->literal);
   Node * global = ST->lookupGlobal(symbol);
   Node * local = ST->lookup(symbol);
   if( global != NULL ) {
      printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", node->lineNum, symbol.c_str(), global->lineNum);
      errs += 1;
      return true;
   }
   else if ( local != NULL )
   {
      printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", node->lineNum, symbol.c_str(), local->lineNum);
      errs += 1;
      return true;
   }   
   return false;
}
//////////////////////////////////////////////////
//    CHECK IF SYMBOL IS DUPLICATE IN SCOPE     //
//////////////////////////////////////////////////
// Call this prior to symbol insertion if we are no longer in the Global
// scope. Returns true if it is already declared in the current scope else false.
bool checkDuplicateScope(Node * node, SymbolTable * ST) {
   string symbol = string(node->literal);
   Node * global = ST->lookupGlobal(symbol);
   Node * scope = ST->lookupScope(symbol);
   if( node->isStatic == false ) {
      if( global != NULL ) {
         if( global->nodeType == FuncNT ) { return false; }
         printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", node->lineNum, symbol.c_str(), global->lineNum);
         errs += 1;
         return true;
      }
      else if ( scope != NULL )
      {
         if( scope->nodeType == FuncNT ) { return false; }
         printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", node->lineNum, symbol.c_str(), scope->lineNum);
         errs += 1;
         return true;
      } 
   }
   return false;
}
//////////////////////////////////////////////////
//             CHECK IF VAR IS INIT             //
//////////////////////////////////////////////////
// Call this prior to RHS ID assignment or Standalone IDs
// If it is Init, return true, if false, print warning
bool checkIfInit(Node * node, Node * parent, SymbolTable * ST) {
   Node * ID = fetchSymbol(node, ST);
   if( ID != NULL ) {
      if( ID->isInit == false ) {
         printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", parent->lineNum, node->literal);
         ID->isUsed = true;
         warns += 1;
         return false;
      }
   }
   return true;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//        RETURN THE NODE OF THE SYMBOL         //
//////////////////////////////////////////////////
Node * fetchSymbol(Node * node, SymbolTable * ST) {
   string symbol = string(node->literal);
   Node * global = ST->lookupGlobal(symbol);
   Node * local = ST->lookup(symbol);
   if( local != NULL ) {
      return local;
   }
   else if( global != NULL ) {
      return global;
   }
   return NULL; 
}
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//            SYMBOL/SCOPE FUNCTIONS            //
//////////////////////////////////////////////////
// Check if NodeType enters a scope
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

// Check if NodeType is a symbol to be inserted
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
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//                PRINT FUNCTIONS               //
//////////////////////////////////////////////////
void printData(Node * node) {
   printf("%s, %s", convertNTStr(node), convertDTStr(node));
}

void printNothing(Node * node) {}
//////////////////////////////////////////////////
//            PRINT ERROR FUNCTIONS             //
//////////////////////////////////////////////////
void printNotArray(Node * object, Node * parent) {
   if( object->isArray == true ) {
      printf("ERROR(%d): The operation '%s' does not work with arrays.\n", parent->lineNum, parent->literal);
      errs += 1;
   }
}

void printArrayLHSRHS(Node * lhs, Node * rhs, Node * parent) {
      // IF THEIR IDS ARE THE SAME AND ARE BOTH ARRAYS i.e. cc <= c[3]
      if( lhs->literal == rhs->literal ) {
         if( lhs->isArray == true && rhs->isArray == true ) {
            if( parent->child[0]->nodeType == IdNT && parent->child[1]->nodeType == ArrNT ) {
               lhs->isIndexed = false; // not necessary to do but still...
               printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", parent->lineNum, parent->literal);
               errs += 1;
            }
         }
      }
      if( lhs->isArray == false ) {
         if( rhs->isConst == false ) {
            if( rhs->isArray == true && rhs->isIndexed == false ) {
               printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n", parent->lineNum, parent->literal);
               errs += 1;
            }
         }
      }
      if( lhs->isArray == true ) {
         if( lhs->isIndexed == false ) {
            if( rhs->isArray == false ) {
               if( (rhs->isIndexed == true || rhs->isIndexed == false) && rhs->nodeType != StringConstNT ) {
                  printf("ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", parent->lineNum, parent->literal);
                  errs += 1;
               }
            }
         }
      }
}

void printRequiresDT(Node * lhs, Node * rhs, Node * parent) {
   if( lhs->dataType != rhs->dataType ) {
      printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n",
      parent->lineNum, parent->literal, convertDTStr(lhs), convertDTStr(rhs));
      errs += 1;
   }
} 