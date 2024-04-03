#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <climits>

#include "AST.hpp"
#include "TokenData.h"
#include "parser.tab.h"

extern int warns;
extern int errs;

using namespace std;

Node *createNode(TokenData *tokenData, NodeType nType)
{
   Node *t = new Node();
   if (t == NULL)
   {
      printf("OUT OF MEMORY ERROR :: Line (%d)\n", tokenData->lineNum);
      return NULL;
   }

   if (tokenData != NULL)
   {

      // Transfer over vital information
      t->tknClass = tokenData->tknClass;
      t->lineNum = tokenData->lineNum;
      t->literal = strdup(tokenData->tknStr);
      t->data.Int = INT_MIN;
      t->data.Char = '\0';
      t->data.String = NULL;

      // For specific token classes...
      switch (t->tknClass)
      {
      case NUMCONST:
         t->data.Int = tokenData->nVal;
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
         break;
      }
   }
   else
   {
      t->tknClass = -1;
      t->lineNum = -1;
      t->literal = NULL;
      t->data.Int = INT_MIN;
      t->data.Char = '\0';
      t->data.String = NULL;
   }

   for (int i = 0; i < MAXCHILDREN; i++)
   {
      t->child[i] = NULL;
   }
   t->sibling = NULL;
   t->ParmList = map<int, Node *>();

   t->parmC = 0;
   t->childC = 0;
   t->siblingLoc = 0;

   t->nodeType = nType;
   t->dataType = UndefinedDT;

   // Semantic Analysis
   t->isArray = false;              // If node is an array, int a[x] => a.
   t->isIndexed = false;            // If node is an indexed array, a[0].
   t->isInit = false;               // Is node initialized?
   t->isConst = false;              // Is node a constant?
   t->isUsed = false;               // Has this node been used somewhere in the program?
   t->isStatic = false;             // Is this node a declared static variable?
   t->isVisited = false;            // Has this node been visited in the tree traversal?
   t->hasReturn = false;            // Does this function node have a return node.
   t->isAddressed = false;          // Has this node's memory been addressed?
   t->isLib = false;                // Is this node a routine I/O?
   t->is_function_compound = false; // Is this compound node connected to the function?
   t->address = 0;                  // Address location of TM.
   t->isMain = false;
   t->is_loaded = false;
   t->is_decl = false;
   t->is_embedded = false;

   // (HW6) Track size, location, and reference type
   t->refType = NoneRT; // Reference type
   t->size = 1;         // Size of node
   t->location = 0;     // Location/address in memory
   t->break_address = 0;

   delete (tokenData);
   return t;
}

Node *createRoutineNode(char *name, NodeType nType)
{
   Node *t = new Node();
   if (t == NULL)
   {
      printf("OUT OF MEMORY ERROR FOR ROUTINE\n");
      return NULL;
   }

   for (int i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;

   t->sibling = NULL;
   t->childC = 0;
   t->siblingLoc = 0;

   t->tknClass = 0;
   t->lineNum = -1;
   t->literal = strdup(name);
   t->nodeType = nType;
   t->dataType = VoidDT;

   t->ParmList = map<int, Node *>(); // Stores index and Node * to Parameters
   t->parmC = 0;

   // Semantics
   t->isArray = false;
   t->isIndexed = false;
   t->isInit = false;
   t->isConst = false;
   t->isUsed = false;
   t->isVisited = false;
   t->isAddressed = false;
   t->isLib = true;
   t->is_function_compound = false;
   t->is_loaded = false;
   t->is_decl = false;
   t->is_embedded = false;

   // (HW6) Track size, location, and reference type
   t->refType = GlobalRT; // Reference type
   t->size = 1;           // Size of node
   t->location = 0;       // Location/address in memory

   return t;
}

Node *addChild(Node *parentNode, Node *childNode)
{
   if (parentNode == NULL)
   {
      return parentNode;
   }
   else
   {
      parentNode->child[parentNode->childC] = childNode;
      parentNode->childC += 1;
      return parentNode;
   }
}

Node *addSib(Node *parentNode, Node *siblingNode)
{
   /*if( siblingNode == NULL && errs == 0 ) {
      printf("ERROR(SYSTEM): never add a NULL to a sibling list.\n");
      exit(1);
   }*/
   if (parentNode == NULL && siblingNode != NULL)
   {
      // parentNode->siblingLoc = 0;
      return siblingNode;
   }
   else if (parentNode != NULL && siblingNode != NULL)
   {
      Node *tmp = parentNode;
      while (tmp->sibling != NULL)
      {
         tmp = tmp->sibling;
      }
      siblingNode->siblingLoc = tmp->siblingLoc + 1;
      tmp->sibling = siblingNode;
      tmp = siblingNode;
      int i = tmp->siblingLoc;
      while (tmp->sibling != NULL)
      {
         i += 1;
         tmp = tmp->sibling;
         tmp->siblingLoc = i;
      }
      return parentNode;
   }
   return parentNode;
}

void printAST(Node *AST, int tab, printType TYPE)
{
   if (AST == NULL)
   {
      printf("Tree is empty.\n");
      return;
   }
   else
   {
      Node *cur = AST;
      do
      {
         if (cur->siblingLoc > 0)
         {
            for (int i = 0; i < tab; i++)
            {
               printf(".   ");
            }
            printf("Sibling: %d  ", cur->siblingLoc);
         }
         printASTType(cur, TYPE);
         for (int x = 0; x < cur->childC; x++)
         {
            if (cur->child[x] != NULL)
            {
               for (int i = 0; i < tab + 1; i++)
               {
                  printf(".   ");
               }
               printf("Child: %d  ", x);
               printAST(cur->child[x], tab + 1, TYPE);
            }
         }
         cur = cur->sibling;
      } while (cur != NULL);
      return;
   }
}

void printASTType(Node *current, printType TYPE)
{
   if (TYPE == withoutTypes)
   {
      switch (current->nodeType)
      {
      case VarNT:
         printf("Var: %s of type %s ", current->literal, convertDTStr(current));
         break;
      case VarArrNT:
         printf("Var: %s is array of type %s ", current->literal, convertDTStr(current));
         break;
      case FuncNT:
         printf("Func: %s returns type %s ", current->literal, convertDTStr(current));
         break;
      case ParmNT:
         printf("Parm: %s of type %s ", current->literal, convertDTStr(current));
         break;
      case ParmArrNT:
         printf("Parm: %s is array of type %s ", current->literal, convertDTStr(current));
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
         if (current->data.Int == 1)
         {
            printf("Const true ");
         }
         else
         {
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
   else if (TYPE == withTypes)
   {
      switch (current->nodeType)
      {
      case VarNT:
         printf("Var: %s of type %s ", current->literal, convertDTStr(current));
         break;
      case VarArrNT:
         printf("Var: %s is array of type %s ", current->literal, convertDTStr(current));
         break;
      case FuncNT:
         printf("Func: %s returns type %s ", current->literal, convertDTStr(current));
         break;
      case ParmNT:
         printf("Parm: %s of type %s ", current->literal, convertDTStr(current));
         break;
      case ParmArrNT:
         printf("Parm: %s is array of type %s ", current->literal, convertDTStr(current));
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
         printf("Op: chsign of type %s ", convertDTStr(current));
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
         if (current->isArray)
         {
            printf("Const is array %s of type %s ", current->data.String, convertDTStr(current));
         }
         else
         {
            printf("Const %s of type %s ", current->data.String, convertDTStr(current));
         }
         break;
      case BoolConstNT:
         if (current->data.Int == 1)
         {
            printf("Const true of type %s ", convertDTStr(current));
         }
         else
         {
            printf("Const false of type %s ", convertDTStr(current));
         }
         break;
      case StaticNT:
         if (current->isArray == true)
         {
            printf("Var: %s is array of type %s ", current->literal, convertDTStr(current));
         }
         else
         {
            printf("Var: %s of type %s ", current->literal, convertDTStr(current));
         }
         break;
      default:
         printf("ERROR :: printNodeType() :: Unknown Node Type\n");
         break;
      }
   }
   else if (TYPE == isAugmented)
   {
      switch (current->nodeType)
      {
      case VarNT:
         if (current->isStatic == true)
         {
            printf("Var: %s of static type %s ", current->literal, convertDTStr(current));
         }
         else
         {
            printf("Var: %s of type %s ", current->literal, convertDTStr(current));
         }
         printAugmentedInfo(current);
         break;
      case VarArrNT:
         if (current->isStatic == true)
         {
            printf("Var: %s of static array of type %s ", current->literal, convertDTStr(current));
         }
         else
         {
            printf("Var: %s of array of type %s ", current->literal, convertDTStr(current));
         }
         printAugmentedInfo(current);
         break;
      case FuncNT:
         printf("Func: %s returns type %s ", current->literal, convertDTStr(current));
         printAugmentedInfo(current);
         break;
      case ParmNT:
         printf("Parm: %s of type %s ", current->literal, convertDTStr(current));
         printAugmentedInfo(current);
         break;
      case ParmArrNT:
         printf("Parm: %s of array of type %s ", current->literal, convertDTStr(current));
         printAugmentedInfo(current);
         break;
      case CompoundNT:
         printf("Compound ");
         printAugmentedInfo(current);
         break;
      case IfNT:
         printf("If ");
         break;
      case IterNT:
         printf("While ");
         break;
      case ToNT:
         printf("For ");
         printAugmentedInfo(current);
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
         if (current->isArray == true)
         {
            printf("Assign: %s of array of type %s ", current->literal, convertDTStr(current));
         }
         else
         {
            printf("Assign: %s of type %s ", current->literal, convertDTStr(current));
         }
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
         printf("Op: chsign of type %s ", convertDTStr(current));
         break;
      case SizeOfNT:
         printf("Op: sizeof of type %s ", convertDTStr(current));
         break;
      case QuesNT:
         printf("Op: ? of type %s ", convertDTStr(current));
         break;
      case IdNT:
         if (current->isArray == true)
         {
            if (current->isStatic == true)
            {
               printf("Id: %s of static array of type %s ", current->literal, convertDTStr(current));
            }
            else
            {
               printf("Id: %s of array of type %s ", current->literal, convertDTStr(current));
            }
         }
         else
         {
            if (current->isStatic == true)
            {
               printf("Id: %s of static type %s ", current->literal, convertDTStr(current));
            }
            else
            {
               printf("Id: %s of type %s ", current->literal, convertDTStr(current));
            }
         }
         printAugmentedInfo(current);
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
         printf("Const %s of array of type %s ", current->data.String, convertDTStr(current));
         printAugmentedInfo(current);
         break;
      case BoolConstNT:
         if (current->data.Int == 1)
         {
            printf("Const true of type %s ", convertDTStr(current));
         }
         else
         {
            printf("Const false of type %s ", convertDTStr(current));
         }
         break;
      case StaticNT:
         if (current->isArray == true)
         {
            printf("Var: %s of static array of type %s ", current->literal, convertDTStr(current));
         }
         else
         {
            printf("Var: %s of static type %s ", current->literal, convertDTStr(current));
         }
         printAugmentedInfo(current);
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
void printAugmentedInfo(Node *node)
{
   printf("[mem: %s loc: %d size: %d] ", convertRTStr(node), node->location, node->size);
}

char *convertRTStr(Node *node)
{
   char *tmp;
   if (node->refType == LocalRT)
      tmp = (char *)"Local";
   if (node->refType == GlobalRT)
      tmp = (char *)"Global";
   if (node->refType == StaticRT)
      tmp = (char *)"LocalStatic";
   if (node->refType == ParamRT)
      tmp = (char *)"Parameter";
   if (node->refType == NoneRT)
      tmp = (char *)"None";
   return tmp;
}

char *convertDTStr(Node *node)
{
   char *tmp;
   if (node->dataType == IntDT)
      tmp = (char *)"int";
   if (node->dataType == CharDT)
      tmp = (char *)"char";
   if (node->dataType == BoolDT)
      tmp = (char *)"bool";
   if (node->dataType == VoidDT)
      tmp = (char *)"void";
   if (node->dataType == UndefinedDT)
      tmp = (char *)"undefined";
   return tmp;
}

char *convertNTStr(Node *node)
{
   char *tmp;
   switch (node->nodeType)
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