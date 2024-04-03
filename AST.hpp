#ifndef _AST_H
#define _AST_H

#include "TokenData.h"
#include <map>

#define MAXCHILDREN 3

// Print Type for AST
typedef enum PT {
   withTypes,
   withoutTypes,
   isAugmented
} printType;

// Node Type for Nodes
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
   TypeNT,
   RET_SYM
} NodeType;

// Data Type for Nodes
typedef enum DT {
   IntDT,
   CharDT,
   BoolDT,
   UndefinedDT,
   VoidDT
} DataType;

typedef enum RT {
   LocalRT,          // Local scope
   GlobalRT,         // Global scope
   StaticRT,         // Static type (Global scope)
   ParamRT,          // Parameter type (Local scope)
   NoneRT
} RefType;

typedef struct AST
{
   struct AST * child[MAXCHILDREN];          // Children node list of node.
   struct AST * sibling;                     // Node sibling pointer of node.
   std::map<int, struct AST *> ParmList;     // If this node is function, store the declared parameter nodes in this map.

   int parmC;                 // Parameter counter for function node.S
   int childC;                // Child node counter for this node.
   int siblingLoc;            // Sibling counter/location connected to this node.

   int tknClass;              // Token Class (from Flex).
   int lineNum;               // Line number where token was read.
   char *literal;             // Literal Token Read from yyval.

   NodeType nodeType;         // Node type for the node. 
   DataType dataType;         // Data type for the node.

   // Union :: Stores the data read from Constant tokens.
   union data{
      int Int;                // Integers
      char Char;              // Single-character
      char *String;           // Multi-character (string)
   } data;

   // Semantic Analysis 
   bool isArray;                    // If node is an array, int a[x] => a.
   bool isIndexed;                  // If node is an indexed array, a[0].
   bool isInit;                     // Is node initialized?
   bool isConst;                    // Is node a constant?
   bool isUsed;                     // Has this node been used somewhere in the program?
   bool isStatic;                   // Is this node a declared static variable?
   bool isVisited;                  // Has this node been visited in the tree traversal?
   bool hasReturn;                  // Does this function node have a return node.
   bool isAddressed;

   // (HW6) Track size, location, and reference type
   RefType refType;           // Reference type
   int size;                  // Size of node
   int location;              // Location/address in memory
   
} Node;

// Function defintions for AST
Node * createNode(TokenData *, NodeType);
Node * createRoutineNode(char *, NodeType);
Node * addChild(Node *, Node *);
Node * addSib(Node *, Node *);

void printAST(Node *, int, printType);
void printASTType(Node *, printType);

void printAugmentedInfo(Node *);
char * convertRTStr(Node *);
char * convertNTStr(Node *);
char * convertDTStr(Node *);

#endif