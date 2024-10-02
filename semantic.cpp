/*********************************************************************
 * @file semantic.cpp
 * 
 * @brief Source file for semantic analysis.
 *********************************************************************/

#include "semantic.hpp"
#include "types.hpp"

#define SPACE 48

typedef types::NodeType NT;

using namespace std;

namespace semantic {

bool SymbolTable::insertSymbol(shared_ptr<node::Node> node) {
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
        return symbols_[name].get();
    }
    #if PENDANTIC_DEBUG
    cout << "Error: " << name << " not declared." << endl;
    #endif
    return nullptr;
}

void SymbolTable::printSymbols(Scope &scope) {
    cout << "+" << string(SPACE, '-') << "+" << endl;
    cout << "| SCOPE: \"" << scope.getName() << "\"" << string(SPACE - 10 - scope.getName().size(), ' ') << "|" << endl;
    cout << "+" << string(SPACE, '-') << "+" << endl;

    for (auto const& [key, val] : symbols_) {

        string line = to_string(val->getLine());
        string nodeType = types::pendaticNodeTypeToStr(val->getNodeType());
        string varType = types::varTypeToStr(val->getVarType());

        cout << "| " << setw(5) << left << "(" + line + ")";
        cout << " " << nodeType;
        cout << " => " << key << " : " << varType;
        cout << string(SPACE - line.size() - nodeType.size() - key.size() - varType.size() - 10, ' ');
        cout << "|" << endl;
    }
    cout << "+" << string(SPACE, '-') << "+" << endl;
}

void SemanticAnalyzer::printWarnings() {
    cout << "Number of warnings: " << warnings_ << endl;
    flush(cout);
}

void SemanticAnalyzer::printErrors() {
    cout << "Number of errors: " << errors_ << endl;
    flush(cout);
}

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
            node->pendanticPrint();
            globalScope_->insertSymbol(node);
    }

    // Do not traverse into children as this access the local scopes

    if (node->getSibling() != nullptr) {
        traverseGlobals(node->getSibling());
    }
}

void SemanticAnalyzer::bfsTraversal(node::Node *root) {
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
}

void SemanticAnalyzer::analyze() {

    #if PENDANTIC_DEBUG
    cout << "Analyzing the AST..." << endl;
    #endif

    // Traverse for Global Variables and Functions
    traverseGlobals(tree_);
    globalScope_->printSymbolTable();

}

} // namespace semantic