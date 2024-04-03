#include "memloc.hpp"

extern int goffset;  // Global frame (R0)
extern int foffset;  // Local frame (R1)

Node * function_node;
bool bypass_function_compound_stmt = false;
bool inside_embedded_compound_stmt = false;
//int foffset_old;

void setAddressLocations(Node * node, SymbolTable * ST) {
   if( node != NULL ) {
      // No need to set refTypes (inside semantic.cpp)
      if( node->nodeType == FuncNT ) {
         bypass_function_compound_stmt = false;
         function_node = node;
         foffset = 0;
         node->size = -2;
         foffset += node->size;
      } 
      else if( node->nodeType == CompoundNT || node->nodeType == ToNT ) {
         node->size = foffset;
         inside_embedded_compound_stmt = true;
         int foffset_old = foffset;
         for( int i = 0; i < MAXCHILDREN; i++ ) {
            miniTraverse(node->child[i], ST);
         }
         foffset = foffset_old;
         inside_embedded_compound_stmt = false;
      }
      else if( isSymbol(node) ) {
         if( inside_embedded_compound_stmt == true ) { 
            if( node->nodeType == VarNT || node->nodeType == VarArrNT ) {
               node->isAddressed = true;
               node->refType = LocalRT;
               node->location = foffset;
               if( node->isArray == true ) { node->location -= 1; }
               foffset -= node->size;
            }
         }
         else if( node->nodeType == VarNT || node->nodeType == VarArrNT ) {
            
            if( node->refType == LocalRT )   // In the frame.
            { 
               node->location = foffset;
               if( node->isArray == true ) { node->location -= 1; }
               foffset -= node->size;
            }
            else if ( node->refType == GlobalRT )  // In global space.
            {
               // Is it initialized? <var>:<constant>
               if( node->child[0] != NULL ) {
                  // Only for String constants...
                  if( node->child[0]->nodeType == StringConstNT ) {
                     node->child[0]->location = goffset - 1;
                     goffset -= node->child[0]->size;
                     node->child[0]->isAddressed = true;
                  }
                  node->location = goffset;
                  if( node->isArray == true ) { node->location -= 1; }
                  goffset -= node->size;
               }
               // Just a plain ole' declaration.
               else if ( node->child[0] == NULL ) {
                  node->location = goffset;
                  if( node->isArray == true ) { node->location -= 1; }
                  goffset -= node->size;
               }
            }
         }
         else if( node->nodeType == ParmNT || node->nodeType == ParmArrNT ) {
            // Always in frame memory space.
            if( function_node != NULL ) { function_node->size -= 1; }
            node->location = foffset;
            foffset -= node->size;
         }
         else if( node->nodeType == StaticNT ) {
            // Always in global memory space.
            if( node->child[0] != NULL ) {
               if( node->child[0]->nodeType == StringConstNT ) {
                  node->child[0]->location = goffset - 1;
                  goffset -= node->child[0]->size;
                  node->child[0]->isAddressed = true;
               }
               node->location = goffset;
               if( node->isArray == true ) { node->location -= 1; }
               goffset -= node->size;
            }
            else {
               node->location = goffset;
               if( node->isArray == true ) { node->location -= 1; }
               goffset -= node->size;
            }
         }
      }
      else if( isConstant(node) ) {
         if( node->nodeType == StringConstNT ) { 
            node->location = goffset - 1;
            goffset -= node->size;
         }
      }
   }
}  

void miniTraverse(Node * node, SymbolTable * ST) {
   if( node == NULL ) return;
   setAddressLocations(node, ST);
   miniTraverse(node->sibling, ST);
}

/* 
void setAddressLocations(Node * node) {
   if( node != NULL ) {
      if( node->nodeType == FuncNT ) {
         node->refType = GlobalRT;
         node->size = -2;
         for( Node * cursor = node->child[0]; cursor != NULL; cursor = cursor->sibling ) {
         node->size -= cursor->size;
      }
      int foffset_OLD = foffset;
      foffset = -2;
      for( int i = 0; i < MAXCHILDREN; i++ ) {
         //walkAST(node->child[i]);
      }
      foffset = foffset_OLD;
   }
   if( node->nodeType == ParmNT || node->nodeType == ParmArrNT ) {
      node->refType = ParamRT;
      //walkAST(node->child[0]);
      node->location = foffset;
      foffset -= node->size;
   }
   if( node->nodeType == VarNT || node->nodeType == VarArrNT || node->nodeType == StaticNT ) {
      for( int i = 0; i < MAXCHILDREN; i++ ) {
         //walkAST(node->child[0]);
      }
      if( node->refType == StaticRT || node->refType == GlobalRT || node->nodeType == StaticNT ) {
         node->location = goffset;
         if( node->isArray == true ) { node->location -= 1; }
         goffset -= node->size;
         } else {
            node->refType = LocalRT;
            node->location = foffset;
            if( node->isArray == true ) { node->location -= 1; }
            foffset -= node->size;
         }
         if( node->nodeType == CompoundNT || node->nodeType == ToNT ) {
            node->size = foffset;
            for(Node * cursor = node->child[0]; cursor != NULL; cursor = cursor->sibling ) {
               if( cursor->refType != GlobalRT ) { node->size -= cursor->size; }
            }
            int foffset_OLD = foffset;
            for( int i = 0; i < MAXCHILDREN; i++ ) {
               //walkAST(node->child[i]);
            }
            foffset = foffset_OLD;
         }
         if( node->nodeType == IdNT ) {
            //
         }
         if( node->nodeType == StringConstNT ) {
            node->refType = GlobalRT;
            node->location = goffset - 1;
            goffset -= node->size;
         }
         for( int i = 0; i < MAXCHILDREN; i++ ) {
            //walkAST(node->child[i]);
         }
      }
   }
}
*/

/*
void placeNode(Node *node) {
    if(node->isDecl) {
        if(node->nodeType == ntFunc) {
            node->referenceType = rtGlobal;
            node->size = -2;
            for(Node *cursor = node->child[0]; cursor != NULL; cursor = cursor->sibling) {
                node->size -= cursor->size;
            }
            node->location = 0;
            int foffsetOld = foffset;
            foffset = -2;
            for(int i = 0; i < AST_MAX_CHILDREN; i++) {
                walkAST(node->child[i]);
            }
            foffset = foffsetOld;
        } else if(node->nodeType == ntParm || node->nodeType == ntParmArray) {
            node->referenceType = rtParameter;
            walkAST(node->child[0]);
            node->location = foffset;
            foffset -= node->size;
        } else if(node->nodeType == ntVar || node->nodeType == ntVarArray || node->nodeType == ntStaticVar) {
            for(int i = 0; i < AST_MAX_CHILDREN; i++) {
                walkAST(node->child[i]);
            }
            if(node->referenceType == rtStatic || node->referenceType == rtGlobal || node->nodeType == ntStaticVar) {
                node->location = goffset;
                if(node->isArray) { node->location -= 1; }
                goffset -= node->size;
            } else {
                node->referenceType = rtLocal;
                node->location = foffset;
                if(node->isArray) {
                    node->location -= 1;
                }
                foffset -= node->size;
            }
        }
    } else {
        if(node->nodeType == ntCompound || node->nodeType == ntCompoundwFunc || node->nodeType == ntTo || node->nodeType == ntTowComp) {
            node->size = foffset;
            for(Node *cursor = node->child[0]; cursor != NULL ; cursor = cursor->sibling) {
                if(cursor->referenceType != rtGlobal) { node->size -= cursor->size; }
            }
            node->location = 0;
            int foffsetOld = foffset;
            for(int i = 0; i < AST_MAX_CHILDREN; i++) {
                walkAST(node->child[i]);
            }
            foffset = foffsetOld;
        } else if(node->nodeType == ntID) {
            if(node->entry->following->node == NULL) {
                printf("failed\n");
            }
            node->referenceType = node->entry->following->node->referenceType;
            node->location = node->entry->following->node->location;
            node->size = node->entry->following->node->size;
        } else if(node->nodeType == ntStringConst) {
            node->referenceType = rtGlobal;
            node->location = goffset - 1;
            goffset -= node->size;
        } else {
            for(int i = 0; i < AST_MAX_CHILDREN; i++) {
                walkAST(node->child[i]);
            }
        }
    }
}
*/