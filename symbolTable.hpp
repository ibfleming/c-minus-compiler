#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

#include "AST.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

class SymbolTable
{
private:
   class Scope;           // Scope class
   vector<Scope *> stack; // Stack containing Scope ptr classes
   bool debugFlag;        // Debugging mode
public:
   SymbolTable();                                // Constructor
   void debug(bool state);                       // Change debugging mode
   int depth();                                  // Depth of the scope stack
   void print(void (*printFunc)(Node *));        // Prints Symbol Table (all scopes)
   void enter(string name);                      // Enter a scope with the given name
   void leave();                                 // Leave the current scope (can't be global)
   void leaveScope(string name);                 // Leave a particular scope
   bool insert(string symbol, Node *node);       // Adds symbol (and Node) to the current scope
   bool insertGlobal(string symbol, Node *node); // Adds symbol (and Node) to the Global scope
   Node *lookup(string symbol);                  // Returns Node ptr of the symbol in the scope if it exists
   Node *lookupGlobal(string symbol);            // Returns Node ptr of the symbol in Global scope if it exists
   void checkUnusedVars();
   void checkUnusedGlobalVars();
   Node *lookupScope(string symbol);
   Node *lookupScopeName(string symbol, string scopeName);
   map<int, Node *> returnGlobalDecls();
};
#endif