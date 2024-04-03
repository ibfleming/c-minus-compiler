#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
   t->tknStr = strdup(tokenData->tknStr);
   t->nodeType = nType;
   t->dataType = Void;

   switch (t->tknClass) {
      case ID:
         t->data.strVal = strdup(tokenData->strVal);
         break;
      case NUMCONST:
         t->data.nVal = tokenData->nVal;
         break;
      case CHARCONST:
         t->data.cVal = tokenData->cVal;
         break;
      case STRINGCONST:
         t->data.strVal = strdup(tokenData->strVal);
         break;
      case BOOLCONST:
         t->data.nVal = tokenData->nVal;
         break;
      default:
         t->data.strVal = strdup(tokenData->strVal);
         break;
   }

   // Debug
   //printf("Created Node %s\n", t->tknStr);
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
      /*
      if( childNode == NULL ) {
         printf("Added NULL child to parent %s\n", parentNode->tknStr);
      }
      else {
         printf("Added %s child to parent %s\n", childNode->tknStr, parentNode->tknStr);
      }
      */
      return parentNode;
   }
}

Node * addSib(Node * parentNode, Node * siblingNode)
{
   if( parentNode == NULL )
   {
      //printf("Started Row with %s\n", siblingNode->tknStr);
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
      //printf("Added %s as sibling to %s\n", siblingNode->tknStr, tmp->tknStr);
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

void printAST(Node * AST, int tab)
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
         printNodeType(cur);
         for(int x = 0; x < cur->childC; x++)
         {
            if( cur->child[x] != NULL )
            {
               for(int i = 0; i < tab + 1; i++) {
                  printf(".   ");
               }
               printf("Child: %d  ", x);
               printAST(cur->child[x], tab + 1);
            }
         }
         cur = cur->sibling;
      } while( cur != NULL );
      return;
   }
}

char * printDataType(DataType dataType)
{
   char * tmp;
   if( dataType == IntDT ) tmp = (char *)"int";
   if( dataType == CharDT ) tmp = (char *)"char";
   if( dataType == BoolDT ) tmp = (char *)"bool";
   if( dataType == Void ) tmp = (char *)"void";

   return tmp;
}

void printNodeType(Node * current)
{
   switch(current->nodeType) 
   {
      case VarNT:
         printf("Var: %s of type %s ", current->data.strVal, printDataType(current->dataType));
         break;
      case VarArrNT:
         printf("Var: %s of array of type %s ", current->data.strVal, printDataType(current->dataType));
         break;
      case FuncNT:
         printf("Func: %s returns type %s ", current->data.strVal, printDataType(current->dataType));
         break;
      case ParmNT:
         printf("Parm: %s of type %s ", current->data.strVal, printDataType(current->dataType));
         break;
      case ParmArrNT:
         printf("Parm: %s of array of type %s ", current->data.strVal, printDataType(current->dataType));
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
         printf("Assign: %s ", current->data.strVal);
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
         printf("Op: %s ", current->data.strVal);
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
         printf("Id: %s ", current->data.strVal);
         break;
      case ArrNT:
         printf("Op: [ ");
         break;
      case CallNT:
         printf("Call: %s ", current->data.strVal);
         break;
      case NumConstNT:
         printf("Const %d ", current->data.nVal);
         break;
      case CharConstNT:
         printf("Const \'%c\' ", current->data.cVal);
         break;
      case StringConstNT:
         printf("Const %s ", current->data.strVal);
         break;
      case BoolConstNT:
         if( current->data.nVal == 1 ) {
            printf("Const true ");
         }
         else {
            printf("Const false ");
         }
         break;
      case StaticNT:
         printf("Var: %s of static type %s ", current->data.strVal, printDataType(current->dataType));
         break;
      default:
         printf("ERROR :: printNodeType() :: Unknown Node Type\n");
         break;
   }
   printf("[line: %d]", current->lineNum);
   printf("\n");
   return;
}