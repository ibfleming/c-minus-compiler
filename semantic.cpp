/*********************************************************************
 * @file semantic.cpp
 * 
 * @brief Source file for semantic analysis.
 *********************************************************************/

#include "semantic.hpp"

using namespace std;

namespace semantic {

#pragma region SymbolTable

bool SymbolTable::insertSymbol(node::Node* node) {
    string name = node->getString();
    if (symbols_.find(name) == symbols_.end()) {
        symbols_[name] = node;
        #if PENDANTIC_DEBUG
        cout << "Inserted symbol: " << name << endl;
        #endif
        return true;
    }
    #if PENDANTIC_DEBUG
    cout << "Symbol already exists: " << name << endl;
    #endif
    return false;
}

node::Node* SymbolTable::lookupSymbol(const node::Node* sym) {
    for (auto const& item : symbols_) {
        if ( item.first == sym->getString() ) {
            #if PENDANTIC_DEBUG
            cout << "Found symbol: " << sym->getString() << endl;
            #endif
            return item.second;
        }
    }
    #if PENDANTIC_DEBUG
    cout << "Symbol not found: " << sym->getString() << endl;
    #endif
    return nullptr;
}

void SymbolTable::printSymbols() {}

#pragma endregion SymbolTable

#pragma region Scope

void Scope::printScope() {
    cout << "+" << string(SPACE, '-') << "+" << endl;
    cout << "| SCOPE: \"" << name_ << "\"" << string(SPACE - 10 - name_.size(), ' ') << "|" << endl;
    cout << "+" << string(SPACE, '-') << "+" << endl;

    if (getTable()->getSize() == 0) {
        cout << "|";
        cout << setw(SPACE) << left << " NO SYMBOLS.";
        cout << "|" << endl;
    }
    else {
        for ( auto const& [key, val] : getTable()->getSymbols() ) {
            //string line = " (" + to_string(val->getLine()) + ")   ";
            string nodeType = types::pendaticNodeTypeToStr(val->getNodeType());
            string varType = types::varTypeToStr(val->getVarType());
            string data = " " + nodeType + " => " + key + " : " + varType;
            cout << "|";
            cout << setw(SPACE) << left << data;
            cout << "|" << endl;
        }
    }
    cout << "+" << string(SPACE, '-') << "+" << endl;
}

void Scope::checkUsedVariables(semantic::SemanticAnalyzer *analyzer) {
    for ( auto const& [key, val] : getTable()->getSymbols() ) {
        if (!val->getIsUsed()) {
            logger::WARN_VariableNotUsed(analyzer, val);
        }
    }
}

#pragma endregion Scope

#pragma region Analyzer

/***********************************************
*  SCOPE MANAGEMENT
***********************************************/

void SemanticAnalyzer::enterScope(Scope *scope) {
    scopes_.push(scope);
    #if PENDANTIC_DEBUG
    cout << "Entering Scope: " << scope->getName() << endl;
    #endif
}

void SemanticAnalyzer::leaveScope() {
    auto scope = scopes_.top();
    checkForUse(scope);
    scopes_.pop();
    if (scope != nullptr) {
        if( scope == globalScope_ ) {
            #if PENDANTIC_DEBUG
            cerr << "ERROR(SCOPE): Cannot leave the global scope." << endl;
            #endif
            return;
        }
        #if PENDANTIC_DEBUG
        cout << "Leaving Scope: " << scope->getName() << endl;
        scope->printScope();
        #endif
    }
}

void SemanticAnalyzer::printScopes() {
    std::vector<Scope*> scopes;
    std::stack<Scope*> scopesTemp = scopes_;

    while (!scopesTemp.empty()) {
        Scope* currentScope = scopesTemp.top();
        if (currentScope != nullptr) {
            scopes.push_back(currentScope);
        }
        scopesTemp.pop();
    }
    
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        (*it)->printScope();
    }
}

/***********************************************
*  SYMBOL TABLE MANAGEMENT
***********************************************/

void SemanticAnalyzer::insertGlobalSymbol(node::Node *sym) {
    auto *decl = lookupGlobalSymbol(sym);
    if (decl != nullptr) {
        logger::ERROR_VariableAlreadyDeclared(this, sym, decl);
        return;
    }
    #if PENDANTIC_DEBUG
    cout << "[Global] ";
    #endif
    getGlobalScope()->insertSymbol(sym);
}

void SemanticAnalyzer::insertLocalSymbol(node::Node *sym) {
    // According to the semantics of C-, do not just global scope here
    if ( getScopeCount() > 1 ) { // currently in multiple scopes, iterate thru them
        auto *decl = lookupAllScopes(sym);
        if (decl != nullptr) {
            logger::ERROR_VariableAlreadyDeclared(this, sym, decl);
            return;
        }
    } else { // only in the one scope so therefore just check here
        auto *decl = lookupLocalSymbol(sym);
        if (decl != nullptr) {
            logger::ERROR_VariableAlreadyDeclared(this, sym, decl);
            return;
        }
    }
    #if PENDANTIC_DEBUG
    cout << "[Local] ";
    #endif
    getCurrentScope()->insertSymbol(sym);
}

node::Node* SemanticAnalyzer::lookupGlobalSymbol(const node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Global] ";
    #endif
    return getGlobalScope()->lookupSymbol(sym);
}

node::Node* SemanticAnalyzer::lookupLocalSymbol(const node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Local] ";
    #endif 
    return getCurrentScope()->lookupSymbol(sym);
}

node::Node* SemanticAnalyzer::lookupAllScopes(node::Node* sym) {
    auto scopes = utils::stackToVector(scopes_);
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        #if PENDANTIC_DEBUG
        cout << (it == scopes.rbegin() ? "[Local] " : "[Scopes] ");
        #endif
        // Do not search through function scopes for duplicate variables
        if ((*it)->getName().find("FUNCTION_") != string::npos) {
            continue;
        }
        if (auto decl = (*it)->lookupSymbol(sym)) {
            return decl; // Return if symbol found
        }
    }
    return nullptr; // Return null if symbol not found
}

void SemanticAnalyzer::checkVariableDeclaration(node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Check Var] ";
    #endif

    // (1) Check if the variable is being used as a function (all functions declared in global scope)
    if (auto decl = lookupGlobalSymbol(sym)) {
        if( decl->getNodeType() == types::NodeType::FUNCTION ) {
            logger::ERROR_VariableAsFunction(this, sym);
            return;
        }
    }

    // (2) Lambda function to set the variable as used
    auto setVarUsed = [](Scope* scope, node::Node* sym) {
        auto decl = scope->lookupSymbol(sym);
        if (decl) decl->setIsUsed(true);
        return decl;
    };

    // (3) Check if the variable is declared in other scopes
    if (getScopeCount() > 1) {
        for (auto& scope : utils::stackToVector(scopes_)) {
            #if PENDANTIC_DEBUG
            cout << (&scope == &scopes_.top() ? "[Local] " : "[Scopes] ");
            #endif
            if (setVarUsed(scope, sym)) return;
        }
        logger::ERROR_VariableNotDeclared(this, sym);
    }
    // (4) Check if the variable is declared in the current local scope 
    else if (auto decl = lookupLocalSymbol(sym)) {
        decl->setIsUsed(true);
    }
    // (5) Default case, not found in any scope 
    else {
        logger::ERROR_VariableNotDeclared(this, sym);
    }
}

void SemanticAnalyzer::checkCallDeclaration(node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Check Call] ";
    #endif

    // (1) Check if the function is declared in the global scope as all functions are declared there
    if (lookupGlobalSymbol(sym) != nullptr) {
        sym->setIsUsed(true);
        return;
    }

    // (2) Default case, not found in global scope so function is not declared
    logger::ERROR_VariableNotDeclared(this, sym);
}

#pragma endregion Analyzer

#pragma region Traversal

/******************************************************************************
 * Analyzing the AST for semantic errors and warnings.
 *
 * Using the root of the AST and a symbol table, the semantic analyzer will
 * traverse the AST and check for semantic errors and warnings.
 *
 * Implementations for DFS and BFS traversal are provided. Likely be using
 * DFS traversal for the semantic analysis.
 ******************************************************************************/

void SemanticAnalyzer::traverseGlobals(node::Node *node) {
    if (node == nullptr) { return; }

    switch (node->getNodeType()) {
        case NT::FUNCTION:
        case NT::VARIABLE:
        case NT::VARIABLE_ARRAY:
        case NT::VARIABLE_STATIC:
        case NT::VARIABLE_STATIC_ARRAY:
            node->setIsVisited(true);
            insertGlobalSymbol(node);
    }

    // Do not traverse into children as this access the local scopes

    if (node->getSibling() != nullptr) {
        traverseGlobals(node->getSibling());
    }
}

void SemanticAnalyzer::traverseLocals(node::Node *node) {
    if (node == nullptr) { return; }

    switch (node->getNodeType()) {
        case NT::FUNCTION:
            enterScope(new Scope(node, "FUNCTION_" + node->getString()));
            break;
        case NT::COMPOUND:
            if (node->getFunctionNode() == nullptr) {
                enterScope(new Scope(node, "COMPOUND_" + to_string(compoundLevel_))); 
                compoundLevel_++;
            }
            break;
        case NT::VARIABLE:
        case NT::VARIABLE_ARRAY:
        case NT::VARIABLE_STATIC:
        case NT::VARIABLE_STATIC_ARRAY:
        case NT::PARAMETER:
        case NT::PARAMETER_ARRAY:
            if (!node->getIsVisited()) {
                insertLocalSymbol(node);
            }
            break;
        case NT::ID:
            checkVariableDeclaration(node);
            break;
        case NT::CALL:
            checkCallDeclaration(node);
            break;
        default:
            break;
    }

    // Traverse children nodes recursively first (inner blocks)
    for (node::Node *child : node->getChildren()) {
        traverseLocals(child);
    }

    // After children, check sibling node (for next block at the same level)
    if (node->getSibling() != nullptr) {
        if (node->getNodeType() == NT::COMPOUND) {
            compoundLevel_--; leaveScope();
        }
        traverseLocals(node->getSibling());
    }
    else {
        if (node->getNodeType() == NT::COMPOUND) {
            if (node->getFunctionNode() == nullptr) { // regular compound
                compoundLevel_--; leaveScope();
            } else { // function's body compound
                leaveScope();
            }
        }
    }
}

void SemanticAnalyzer::analyze() {
    // Traverse for Global Variables and Functions
    traverseGlobals(tree_);
    
    // Check Linker?

    #if PENDANTIC_DEBUG
    printGlobal(); // will only print the global scope
    #endif

    traverseLocals(tree_);
}

#pragma endregion Traversal

}