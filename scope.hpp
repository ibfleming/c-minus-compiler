#ifndef _SCOPE_H
#define _SCOPE_H

#include "symbolTable.hpp"

using namespace std;

class SymbolTable::Scope {
   private:
      string name;                               // Name of the scope
      map<string, Node *> symbols;               // Map of symbols: symbol name points to the Node * of that symbol
      static bool debugFlag;                     // Debugging mode
   public:
      Scope(string newName);                     // Constructor (newName is the name of the scope)
      ~Scope();                                  // Destructor
      string scopeName();                        // Returns name of the scope
      void debug(bool state);                    // Change debugging mode
      void print(void (*printFunc)(Node *));     // Print the symbols
      bool insert(string symbol, Node * node);   // Adds symbol (and Node) to the current scope
      Node * lookup(string symbol);              // Returns Node ptr of the symbol in the scope if it exists
      void checkUnusedVars();
      void checkUnusedGlobalVars();  
      Node * lookupScope(string symbol);
      void checkIfDeclared(string symbol);
};
#endif