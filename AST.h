#ifndef _AST_H
#define _AST_H
#include "TokenData.h"
#include <map>

#define MAXCHILDREN 3

typedef enum NT {
   VarNT,
   VarArrNT,
   FuncNT,
   ParmNT,
   ParmArrNT,
   CompoundNT,
   IfNT,
   IterNT,
   ToNT,
   RangeNT,
   ReturnNT,
   BreakNT,
   AssignNT,
   OrNT,
   AndNT,
   NotNT,
   OpNT,
   SignNT,
   SizeOfNT,
   QuesNT,
   IdNT,
   ArrNT,
   CallNT,
   NumConstNT,
   CharConstNT,
   StringConstNT,
   BoolConstNT,
   StaticNT,
   RET_SYM
} NodeType;

typedef enum DT {
   IntDT,
   CharDT,
   BoolDT,
   UndefinedDT,
   VoidDT
} DataType;

typedef struct AST Node;
struct AST
{
   Node * child[MAXCHILDREN];
   Node * sibling;
   std::map<int, Node *> ParmList;

   int childC;
   int siblingLoc;

   int tknClass; // Token Class (from Flex)
   int lineNum;  
   char *literal; // Literal Token Read from yyval

   NodeType nodeType;
   DataType dataType;

   union data{
      int Int;
      char Char;
      char *String;
   } data;

   // Semantics
   bool isArray;
   bool isIndexed; // If array is indexed
   bool isInit;
   bool isConst;
   bool isUsed;
   bool isStatic;
   bool isVisited;
   bool hasReturn;
   int parmC;

};

Node * createNode(struct TokenData *, NodeType);
Node * createRoutineNode(char * name, NodeType nType);
Node * addChild(Node *, Node *);
Node * addSib(Node *, Node *);
void printAST(Node *, int, bool);

void printNodeType(Node * current, bool types);
char * convertNTStr(Node * node);
char * convertDTStr(Node * node);
#endif