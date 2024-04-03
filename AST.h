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
   StaticNT
} NodeType;

typedef enum DT {
   IntDT,
   CharDT,
   BoolDT,
   Void
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
   char *tknStr; // Literal Token Read from yyval

   NodeType nodeType;
   DataType dataType;

   union{
      int nVal;
      char cVal;
      char *strVal;
   } data;

};

Node * createNode(struct TokenData *, NodeType);
Node * addChild(Node *, Node *);
Node * addSib(Node *, Node *);
void printAST(Node *, int);
void printNodeType(Node *);
char * printDataType(DataType);

#endif