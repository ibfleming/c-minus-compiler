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
        cout << "Inserted: " << name << endl;
        #endif
        return true;
    }
    #if PENDANTIC_DEBUG
    cout << "Error: " << name << " already declared." << endl;
    #endif
    return false;
}

node::Node* SymbolTable::lookupSymbol(const string name) {
    if (symbols_.find(name) != symbols_.end()) {
        #if PENDANTIC_DEBUG
        cout << "Lookup: " << name << endl;
        #endif
        return symbols_[name];
    }
    #if PENDANTIC_DEBUG
    cout << "Error: " << name << " not declared." << endl;
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
            string line = " (" + to_string(val->getLine()) + ")   ";
            string nodeType = types::pendaticNodeTypeToStr(val->getNodeType());
            string varType = types::varTypeToStr(val->getVarType());
            string data = line + nodeType + " => " + key + " : " + varType;
            cout << "|";
            cout << setw(SPACE) << left << data;
            cout << "|" << endl;
        }
    }
    cout << "+" << string(SPACE, '-') << "+" << endl;
}

#pragma endregion Scope

#pragma region SemanticAnalyzer

void SemanticAnalyzer::printWarnings() {
    cout << "Number of warnings: " << warnings_ << endl;
    flush(cout);
}

void SemanticAnalyzer::printErrors() {
    cout << "Number of errors: " << errors_ << endl;
    flush(cout);
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


#pragma endregion SemanticAnalyzer

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
            {
                cout << "(" << node->getLine() << ")" << " Enter Function: " << node->getString() << endl;
                Scope *scope = new Scope(node, node->getString());
                scopes_.push(scope);
            }
            break;
        case NT::COMPOUND:
            if (node->getFunctionNode() == nullptr) {
                cout << "(" << node->getLine() << ")" << " Enter Compound" << endl;
                Scope *scope = new Scope(node);
                scopes_.push(scope);
            }
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

        switch (node->getNodeType()) {
            case NT::COMPOUND:
                cout << "(" << node->getLine() << ")" << " Leave Compound" << endl;
                break;
            default:
                break;
        }

        traverseLocals(node->getSibling());

    }
    else {

        switch (node->getNodeType()) {
            case NT::COMPOUND:
                if (node->getFunctionNode() == nullptr) {
                    cout << "(" << node->getLine() << ")" << " Leave Compound" << endl;
                } else {
                    cout << "(" << node->getFunctionNode()->getLine() << ")" << " Leave Function: ";
                    cout << node->getFunctionNode()->getString() << endl;
                }
            default:
                break;
        }
    }
}


void SemanticAnalyzer::analyze() {

    #if PENDANTIC_DEBUG
    cout << "Analyzing the AST..." << endl;
    #endif

    // Traverse for Global Variables and Functions
    traverseGlobals(tree_);
    traverseLocals(tree_);

/*  cout << endl << endl << endl;
    cout << "ALL SCOPES" << endl;
    printScopes(); */
}

#pragma endregion Traversal

} // namespace semantic

/* void SemanticAnalyzer::bfsTraversal(node::Node *root) {
    if (root == nullptr) { return; }

    queue<node::Node*> queue;
    queue.push(root);

    while (!queue.empty()) {
        node::Node *node = queue.front();
        queue.pop();

        switch (node->getNodeType()) {
            case NT::FUNCTION:
            case NT::VARIABLE:
            case NT::VARIABLE_ARRAY:
            case NT::VARIABLE_STATIC:
                node->pendanticPrint();
                break;
        }

        for (node::Node *child : node->getChildren()) {
            queue.push(child);
        }

        if (node->getSibling() != nullptr) {
            queue.push(node->getSibling());
        }
    }
} */

/*  
    cout << "+" << string(SPACE, '-') << "+" << endl;
    cout << "| SCOPE: \"" << scope.getName() << "\"" << string(SPACE - 10 - scope.getName().size(), ' ') << "|" << endl;
    cout << "+" << string(SPACE, '-') << "+" << endl;

    for (auto const& [key, val] : symbols_) {
        string line = " (" + to_string(val->getLine()) + ")   ";
        string nodeType = types::pendaticNodeTypeToStr(val->getNodeType());
        string varType = types::varTypeToStr(val->getVarType());
        string data = line + nodeType + " => " + key + " : " + varType;
        cout << "|";
        cout << setw(SPACE) << left << data;
        cout << "|" << endl;
    }
    cout << "+" << string(SPACE, '-') << "+" << endl; */