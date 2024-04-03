#ifndef _AST_H
#define _AST_H
#include "TokenData.h"

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
   bool isFunction;
   bool isDeclared;
   bool isUsed;
   bool isStatic;
   bool isNested; // For Nested ASGN

};

Node * createNode(struct TokenData *, NodeType);
Node * addChild(Node *, Node *);
Node * addSib(Node *, Node *);
void printAST(Node *, int, bool);

void printNodeType(Node * current, bool types);
char * convertNTStr(Node * node);
char * convertDTStr(Node * node);
#endif