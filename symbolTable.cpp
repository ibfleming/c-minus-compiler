#include "scope.hpp"
#include "semantics.hpp"
using namespace std;

#define SCOPE_DEBUG false
#define SYMBOL_DEBUG false
bool debugOther = false;
map<int, Node *> g_decl;

using namespace std;

extern int warns;
extern int errs;

// #####################################################################################################
// #                                     SCOPE CLASS FUNCTIONS                                         #
// #####################################################################################################

SymbolTable::Scope::Scope(string newName)
{
   name = newName;
   debugFlag = SCOPE_DEBUG;
}

SymbolTable::Scope::~Scope() {}

string SymbolTable::Scope::scopeName()
{
   return name;
}

void SymbolTable::Scope::debug(bool state)
{
   debugFlag = state;
}

void SymbolTable::Scope::print(void (*printFunc)(Node *))
{
   printf("Scope: %-15s -----------------\n", name.c_str());
   for (map<string, Node *>::iterator it = symbols.begin(); it != symbols.end(); it++)
   {
      printf("%20s: ", (it->first).c_str());
      printFunc(it->second);
      printf("\n");
   }
}

bool SymbolTable::Scope::insert(string symbol, Node *node)
{
   if (symbols.find(symbol) == symbols.end())
   {
      if (debugFlag)
      {
         printf("DEBUG (Scope): Inserted \"%s\" in \"%s\" scope @ LINE [%d].\n", symbol.c_str(), name.c_str(), node->lineNum);
      }
      if (node == NULL)
      {
         printf("ERROR (SymbolTable): Attempting to save a NULL Node pointer for the symbol \"%s\".\n", symbol.c_str());
      }
      symbols[symbol] = node;
      return true;
   }
   else
   {
      if (debugFlag)
      {
         if (debugFlag)
            printf("DEBUG (Scope): Inserted \"%s\" in \"%s\" scope but the symbol is already there!\n", symbol.c_str(), name.c_str());
         return false;
      }
   }
   return false;
}

Node *SymbolTable::Scope::lookup(string symbol)
{
   if (symbols.find(symbol) != symbols.end())
   {
      if (debugFlag)
         printf("DEBUG (Scope): Found \"%s\" symbol in \"%s\" scope.\n", symbol.c_str(), name.c_str());
      return symbols[symbol];
   }
   else
   {
      if (debugFlag)
         printf("DEBUG (Scope): Did NOT find \"%s\" symbol in \"%s\" scope.\n", symbol.c_str(), name.c_str());
      return NULL;
   }
}

void SymbolTable::Scope::checkUnusedVars()
{
   for (map<string, Node *>::iterator it = symbols.begin(); it != symbols.end(); it++)
   {
      if (it->second->isUsed == false)
      {
         if (it->second->nodeType == ParmNT || it->second->nodeType == ParmArrNT)
         {
            printf("WARNING(%d): The parameter \'%s\' seems not to be used.\n", it->second->lineNum, it->second->literal);
            warns += 1;
         }
         else if (it->second->nodeType == FuncNT)
         {
            printf("WARNING(%d): The function \'%s\' seems not to be used.\n", it->second->lineNum, it->second->literal);
            warns += 1;
         }
         else
         {
            printf("WARNING(%d): The variable \'%s\' seems not to be used.\n", it->second->lineNum, it->second->literal);
            warns += 1;
         }
      }
   }
}

map<int, Node *> SymbolTable::Scope::returnGlobalDecls()
{
   int i = 0;
   for (map<string, Node *>::iterator it = symbols.begin(); it != symbols.end(); it++)
   {
      if (it->second->nodeType == VarNT || it->second->nodeType == VarArrNT || it->second->nodeType == StaticNT)
      {
         g_decl[i] = it->second;
      }
      i++;
   }
   return g_decl;
}

bool SymbolTable::Scope::debugFlag; // Initialize to emit errors.

// #####################################################################################################
// #                                     SYMBOLTABLE CLASS FUNCTIONS                                   #
// #####################################################################################################

SymbolTable::SymbolTable()
{
   debugFlag = SYMBOL_DEBUG;
   enter((string) "Global");
}

void SymbolTable::debug(bool state)
{
   debugFlag = state;
}

int SymbolTable::depth()
{
   return stack.size();
}

void SymbolTable::print(void (*printFunc)(Node *))
{
   printf("============  Symbol Table  ============\n");
   for (std::vector<Scope *>::iterator it = stack.begin(); it != stack.end(); it++)
   {
      (*it)->print(printFunc);
   }
   printf("========================================\n");
}

void SymbolTable::enter(string name)
{
   if (debugOther)
      printf("DEBUG (SymbolTable): Entered scope \"%s\".\n", name.c_str());
   stack.push_back(new Scope(name));
}

void SymbolTable::leave()
{
   if (debugOther)
      printf("DEBUG (SymbolTable): Leaving scope \"%s\".\n", (stack.back()->scopeName()).c_str());
   if (stack.size() > 1)
   {
      delete stack.back();
      stack.pop_back();
   }
   else
   {
      printf("ERROR (SymbolTable): You cannot leave Global scope. Number of scopes: %d.\n", (int)stack.size());
   }
}

bool SymbolTable::insert(string symbol, Node *node)
{
   if (debugOther)
   {
      printf("DEBUG (SymbolTable): Inserted \"%s\" symbol in \"%s\" scope @ LINE [%d].", symbol.c_str(), (stack.back()->scopeName()).c_str(), node->lineNum);
      if (node == NULL)
         printf(" WARNING: The inserted pointer is NULL!");
      printf("\n");
   }
   return (stack.back())->insert(symbol, node);
}

bool SymbolTable::insertGlobal(string symbol, Node *node)
{
   if (debugFlag)
   {
      printf("DEBUG (Scope): Insert the Global symbol \"%s\" @ LINE [%d].", symbol.c_str(), node->lineNum);
      if (node == NULL)
         printf(" WARNING: The inserted pointer is NULL!");
      printf("\n");
   }
   return stack[0]->insert(symbol, node);
}

Node *SymbolTable::lookup(string symbol)
{
   Node *data = NULL;
   string name;

   for (vector<Scope *>::reverse_iterator it = stack.rbegin(); it != stack.rend(); it++)
   {
      data = (*it)->lookup(symbol);
      name = (*it)->scopeName();
      if (data != NULL)
         break;
   }
   if (debugFlag)
   {
      printf("DEBUG (SymbolTable): Symbol \"%s\" ", symbol.c_str());
      if (data)
         printf("found in the \"%s\" scope.\n", name.c_str());
      else
         printf("is not in any of the scopes!\n");
   }
   return data;
}

Node *SymbolTable::lookupGlobal(string symbol)
{
   Node *data = stack[0]->lookup(symbol);
   if (debugFlag)
      printf("DEBUG (SymbolTable): Symbol \"%s\" in the Globals and %s.\n", symbol.c_str(), (data ? "found it" : "did NOT find it"));
   return data;
}

void SymbolTable::leaveScope(string name)
{
   for (vector<Scope *>::iterator it = stack.begin(); it != stack.end();)
   {
      if ((*it)->scopeName() == name)
      {
         printf("DEBUG (SymbolTable): Leaving the scope \"%s\" as specified!\n", (*it)->scopeName().c_str());
         delete *it;
         it = stack.erase(it);
      }
      else
      {
         ++it;
      }
   }
}

void SymbolTable::checkUnusedVars()
{
   vector<Scope *>::reverse_iterator it = stack.rbegin();
   if ((*it)->scopeName() != string("Global"))
   {
      (*it)->checkUnusedVars();
   }
}

void SymbolTable::checkUnusedGlobalVars()
{
   stack[0]->checkUnusedVars();
}

Node *SymbolTable::lookupScope(string symbol)
{
   vector<Scope *>::reverse_iterator it = stack.rbegin();
   return (*it)->lookup(symbol);
}

map<int, Node *> SymbolTable::returnGlobalDecls()
{
   return stack[0]->returnGlobalDecls();
}
// #####################################################################################################
// #                                     SYMBOLTABLE CLASS FUNCTIONS                                   #
// #####################################################################################################

Node *
SymbolTable::lookupScopeName(string symbol, string scopeName)
{
   for (vector<Scope *>::reverse_iterator it = stack.rbegin(); it != stack.rend(); it++)
   {
      if ((*it)->scopeName() == scopeName)
      {
         return (*it)->lookup(symbol);
      }
   }
   return NULL;
}