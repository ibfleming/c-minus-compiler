#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "AST.h"
#include "TokenData.h"
#include "parser.tab.h"

Node * createNode(struct TokenData * tokenData, NodeType nType)
{
   Node * t = (Node *) malloc(sizeof(Node));
   if( t == NULL )
   {
      printf("OUT OF MEMORY ERROR :: Line (%d)\n", tokenData->lineNum);
      return NULL;
   }

   for( int i = 0; i < MAXCHILDREN; i++ ) t->child[i] = NULL;
   
   t->sibling = NULL;
   t->childC = 0;
   t->siblingLoc = 0;

   t->tknClass = tokenData->tknClass;
   t->lineNum = tokenData->lineNum;
   t->literal = strdup(tokenData->tknStr);
   t->nodeType = nType;
   t->dataType = UndefinedDT;

   // Semantics
   bool isArray = false;
   bool isIndexed = false;
   bool isInit = false;
   bool isConst = false;
   bool isFunction = false;
   bool isDeclared = false;
   bool isUsed = false;
   bool isNested = false;

   // SPECIFIC CLASS CASES
   switch (t->tknClass) {
      case NUMCONST:
         t->data.Int= tokenData->nVal;
         break;
      case CHARCONST:
         t->data.Char = tokenData->cVal;
         break;
      case STRINGCONST:
         t->data.String = strdup(tokenData->strVal);
         break;
      case BOOLCONST:
         t->data.Int = tokenData->nVal;
         break;
      default:
         t->literal = strdup(tokenData->tknStr);
         break;
   }
   return t;
}

Node * addChild(Node * parentNode, Node * childNode)
{
   if( parentNode == NULL )
   {
      return parentNode;
   }
   else {
      parentNode->child[parentNode->childC] = childNode;
      parentNode->childC += 1;
      return parentNode;
   }
}

Node * addSib(Node * parentNode, Node * siblingNode)
{
   if( parentNode == NULL )
   {
      parentNode->siblingLoc = 0;
      return siblingNode;
   }
   else {
      Node * tmp = parentNode;
      while( tmp->sibling != NULL )
      {
         tmp = tmp->sibling;
      }
      siblingNode->siblingLoc = tmp->siblingLoc + 1;
      tmp->sibling = siblingNode;
      tmp = siblingNode;
      int i = tmp->siblingLoc;
      while( tmp->sibling != NULL )
      {
         i += 1;
         tmp = tmp->sibling;
         tmp->siblingLoc = i;
      }
      return parentNode;
   }
}

void printAST(Node * AST, int tab, bool types)
{
   if( AST == NULL )
   {
      printf("Tree is empty.\n");
      return;
   } 
   else
   {
      Node * cur = AST;
      do {
         if( cur->siblingLoc > 0 ) {
            for(int i = 0; i < tab; i++)
            {
               printf(".   ");
            }
            printf("Sibling: %d  ", cur->siblingLoc);
         }
         printNodeType(cur, types);
         for(int x = 0; x < cur->childC; x++)
         {
            if( cur->child[x] != NULL )
            {
               for(int i = 0; i < tab + 1; i++) {
                  printf(".   ");
               }
               printf("Child: %d  ", x);
               printAST(cur->child[x], tab + 1, types);
            }
         }
         cur = cur->sibling;
      } while( cur != NULL );
      return;
   }
}

void printNodeType(Node * current, bool types)
{
   if( types == false ) {
      switch(current->nodeType) 
      {
         case VarNT:
            printf("Var: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case VarArrNT:
            printf("Var: %s of array of type %s ", current->literal, convertDTStr(current));
            break;
         case FuncNT:
            printf("Func: %s returns type %s ", current->literal, convertDTStr(current));
            break;
         case ParmNT:
            printf("Parm: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case ParmArrNT:
            printf("Parm: %s of array of type %s ", current->literal, convertDTStr(current));
            break;
         case CompoundNT:
            printf("Compound ");
            break;
         case IfNT:
            printf("If ");
            break;
         case IterNT:
            printf("While ");
            break;       
         case ToNT:
            printf("For ");
            break;
         case RangeNT:
            printf("Range ");
            break;
         case ReturnNT:
            printf("Return ");
            break;
         case BreakNT:
            printf("Break ");
            break;
         case AssignNT:
            printf("Assign: %s ", current->literal);
            break;
         case OrNT:
            printf("Op: or ");
            break;
         case AndNT:
            printf("Op: and ");
            break;
         case NotNT:
            printf("Op: not ");
            break;
         case OpNT:
            printf("Op: %s ", current->literal);
            break;
         case SignNT:
            printf("Op: chsign ");
            break;
         case SizeOfNT:
            printf("Op: sizeof ");
            break;
         case QuesNT:
            printf("Op: ? ");
            break;       
         case IdNT:
            printf("Id: %s ", current->literal);
            break;
         case ArrNT:
            printf("Op: [ ");
            break;
         case CallNT:
            printf("Call: %s ", current->literal);
            break;
         case NumConstNT:
            printf("Const %d ", current->data.Int);
            break;
         case CharConstNT:
            printf("Const \'%c\' ", current->data.Char);
            break;
         case StringConstNT:
            printf("Const %s ", current->data.String);
            break;
         case BoolConstNT:
            if( current->data.Int == 1 ) {
               printf("Const true ");
            }
            else {
               printf("Const false ");
            }
            break;
         case StaticNT:
            printf("Var: %s of static type %s ", current->literal, convertDTStr(current));
            break;
         default:
            printf("ERROR :: printNodeType() :: Unknown Node Type\n");
            break;
      }
   }
   else if ( types == true )
   {
      switch(current->nodeType) 
      {
         case VarNT:
            printf("Var: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case VarArrNT:
            printf("Var: %s of array of type %s ", current->literal, convertDTStr(current));
            break;
         case FuncNT:
            printf("Func: %s returns type %s ", current->literal, convertDTStr(current));
            break;
         case ParmNT:
            printf("Parm: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case ParmArrNT:
            printf("Parm: %s of array of type %s ", current->literal, convertDTStr(current));
            break;
         case CompoundNT:
            printf("Compound ");
            break;
         case IfNT:
            printf("If ");
            break;
         case IterNT:
            printf("While ");
            break;       
         case ToNT:
            printf("For ");
            break;
         case RangeNT:
            printf("Range ");
            break;
         case ReturnNT:
            printf("Return ");
            break;
         case BreakNT:
            printf("Break ");
            break;
         case AssignNT:
            printf("Assign: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case OrNT:
            printf("Op: or of type %s ", convertDTStr(current));
            break;
         case AndNT:
            printf("Op: and of type %s ", convertDTStr(current));
            break;
         case NotNT:
            printf("Op: not of type %s ", convertDTStr(current));
            break;
         case OpNT:
            printf("Op: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case SignNT:
            printf("Op: chsign of type %s", convertDTStr(current));
            break;
         case SizeOfNT:
            printf("Op: sizeof of type %s ", convertDTStr(current));
            break;
         case QuesNT:
            printf("Op: ? of type %s ", convertDTStr(current));
            break;       
         case IdNT:
            printf("Id: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case ArrNT:
            printf("Op: [ of type %s ", convertDTStr(current));
            break;
         case CallNT:
            printf("Call: %s of type %s ", current->literal, convertDTStr(current));
            break;
         case NumConstNT:
            printf("Const %d of type %s ", current->data.Int, convertDTStr(current));
            break;
         case CharConstNT:
            printf("Const \'%c\' of type %s ", current->data.Char, convertDTStr(current));
            break;
         case StringConstNT:
            printf("Const %s of type %s ", current->data.String, convertDTStr(current));
            break;
         case BoolConstNT:
            if( current->data.Int == 1 ) {
               printf("Const true of type %s ", convertDTStr(current));
            }
            else {
               printf("Const false of type %s ", convertDTStr(current));
            }
            break;
         case StaticNT:
            printf("Var: %s of static type %s ", current->literal, convertDTStr(current));
            break;
         default:
            printf("ERROR :: printNodeType() :: Unknown Node Type\n");
            break;
      }
   }
   printf("[line: %d]", current->lineNum);
   printf("\n");
   return;
}

char * convertDTStr(Node * node) {
   char * tmp;
   if( node->dataType == IntDT ) tmp = (char *)"int";
   if( node->dataType == CharDT ) tmp = (char *)"char";
   if( node->dataType == BoolDT ) tmp = (char *)"bool";
   if( node->dataType == VoidDT ) tmp = (char *)"void";
   if( node->dataType == UndefinedDT ) tmp = (char *)"undefined";
   return tmp;
}

char * convertNTStr(Node * node) {
   char * tmp;
   switch ( node->nodeType )
   {
      case VarNT:
         tmp = (char *)"VarNT";
         return tmp;
      case VarArrNT:
         tmp = (char *)"VarArrNT";
         return tmp;
      case FuncNT:
         tmp = (char *)"FuncNT";
         return tmp;
      case ParmNT:
         tmp = (char *)"ParmNT";
         return tmp;
      case ParmArrNT:
         tmp = (char *)"ParmArrNT";
         return tmp;
      case CompoundNT:
         tmp = (char *)"CompoundNT";
         return tmp;
      case IfNT:
         tmp = (char *)"IfNT";
         return tmp;
      case IterNT:
         tmp = (char *)"IterNT";
         return tmp;       
      case ToNT:
         tmp = (char *)"ToNT";
         return tmp;
      case RangeNT:
         tmp = (char *)"RangeNT";
         return tmp;
      case ReturnNT:
         tmp = (char *)"ReturnNT";
         return tmp;
      case BreakNT:
         tmp = (char *)"BreakNT";
         return tmp;
      case AssignNT:
         tmp = (char *)"AssignNT";
         return tmp;
      case OrNT:
         tmp = (char *)"OrNT";
         return tmp;
      case AndNT:
         tmp = (char *)"AndNT";
         return tmp;
      case NotNT:
         tmp = (char *)"NotNT";
         return tmp;
      case OpNT:
         tmp = (char *)"OpNT";
         return tmp;
      case SignNT:
         tmp = (char *)"SignNT";
         return tmp;
      case SizeOfNT:
         tmp = (char *)"SizeOfNT";
         return tmp;
      case QuesNT:
         tmp = (char *)"QuesNT";
         return tmp;       
      case IdNT:
         tmp = (char *)"IdNT";
         return tmp;
      case ArrNT:
         tmp = (char *)"ArrNT";
         return tmp;
      case CallNT:
         tmp = (char *)"CallNT";
         return tmp;
      case NumConstNT:
         tmp = (char *)"NumConstNT";
         return tmp;
      case CharConstNT:
         tmp = (char *)"CharConstNT";
         return tmp;
      case StringConstNT:
         tmp = (char *)"StringConstNT";
         return tmp;
      case BoolConstNT:
         tmp = (char *)"BoolConstNT";
         return tmp;
      case StaticNT:
         tmp = (char *)"StaticNT";
         return tmp;
      default:
         printf("convertNTStr() :: Unknown NodeType!\n");
         return (char *)"Unknown";
   }
}