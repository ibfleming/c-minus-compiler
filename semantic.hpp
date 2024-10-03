/*********************************************************************
 * @file semantic.hpp
 * 
 * @brief Header file for semantic analysis.
 *********************************************************************/

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#define SPACE 48
#define PENDANTIC_DEBUG false

#include "types.hpp"
#include "node.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stack>
#include <map>

typedef types::NodeType NT;

/**
 * @namespace semantic
 * @brief Contains the semantic analysis members and functions.
 */
namespace semantic {

class Scope; // forward declaration

#pragma region SymbolTable

class SymbolTable {

private:
    std::map<std::string, node::Node*> symbols_;    // Map of variable declarations (symbols), (name, node)
    bool debug_;                                    // Debug flag?

public:
    /**
     * @fn SymbolTable
     * @brief Constructor for the symbol table.
     */
    SymbolTable(): debug_(false) {};

    /**
     * @fn getSymbols
     * @brief Returns the symbols in the symbol table.
     * @return std::map<std::string, node::Node*>
     */

    std::map<std::string, node::Node*> getSymbols() { return symbols_; }

    /**
     * @fn setDebug
     * @brief Sets the debug flag.
     * @param debug The debug flag.
     */
    void setDebug(bool debug) { debug_ = debug; }

    /**
     * @fn insertSymbol
     * @brief Inserts a symbol into the symbol table.
     * @param node The node (symbol) to insert.
     * @return bool
     */
    bool insertSymbol(node::Node* node);

    /**
     * @fn lookupSymbol
     * @brief Looks up a symbol in the symbol table.
     * @param name The name of the symbol to lookup.
     * @return node::Node*
     */
    node::Node* lookupSymbol(const std::string name);

    /**
     * @fn getSize
     * @brief Returns the size of the symbol table.
     * @return int
     */
    int getSize() { return symbols_.size(); }

    /**
     * @fn printSymbols
     * @brief Prints the symbols in the symbol table.
     */
    void printSymbols();
};

#pragma endregion SymbolTable

#pragma region Scope

class Scope {

private:
    SymbolTable table_;     // Symbol table for the scope
    node::Node  *parent_;   // Node of the scope (FUNCTION, COMPOUND, LOOP, etc.)
    std::string name_;      // Name of the scope
    int location_;          // Location of the scope relative to other scopes?

public:
    /**
     * @fn Scope
     * @brief Constructor for the scope.
     * @param parent The parent node of the scope.
     */
    Scope(node::Node *parent)
     : parent_(parent), name_(types::pendaticNodeTypeToStr(parent->getNodeType())), location_(-1) {}
    /**
     * @fn Scope
     * @brief Constructor for the scope.
     * @param parent The parent node of the scope.
     * @param name The name of the scope.
     */
    Scope(node::Node *parent, std::string name) : parent_(parent), name_(name), location_(-1) {}

    /**
     * @fn getSymbols
     * @brief Returns the symbol table of the scope.
     * @return SymbolTable*
     */
    SymbolTable* getTable() { return &table_; }

    /**
     * @fn getParent
     * @brief Returns the parent node of the scope.
     * @return node::Node*
     */
    node::Node* getParent() { return parent_; }

    /**
     * @fn getName
     * @brief Returns the name of the scope.
     * @return std::string
     */
    std::string getName() { return name_; }

    /**
     * @fn setDebug
     * @brief Sets the debug flag for the symbol table.
     * @param debug The debug flag.
     * @return void
     */
    void setDebug(bool debug) { table_.setDebug(debug); }

    /**
     * @fn insertSymbol
     * @brief Inserts a symbol into the symbol table.
     * @param node The node to insert.
     * @return bool
     */
    bool insertSymbol(node::Node *node) { return table_.insertSymbol(node); }

    /**
     * @fn lookupSymbol
     * @brief Looks up a symbol in the symbol table.
     * @param name The name of the symbol to lookup.
     * @return node::Node*
     */
    node::Node* lookupSymbol(const std::string name) { return table_.lookupSymbol(name); }

    /**
     * @fn printScope
     * @brief Prints the scope and its contained symbols.
     * @return void
     */
    void printScope();
};

#pragma endregion Scope

#pragma region SemanticAnalyzer

class SemanticAnalyzer {

private:
    node::Node *tree_;                          // Root of the AST
    Scope *globalScope_;                        // Global scope
    Scope *currentScope_;                       // Current scope
    std::stack<Scope*> scopes_;                 // Stack of scopes
    int compoundLevel_;
    int warnings_;                              // Number of warnings
    int errors_;                                // Number of errors

public:
    /**
     * @fn SemanticAnalyzer
     * @brief Constructor for the semantic analyzer.
     * @param tree Root of the AST.
     */
    SemanticAnalyzer(node::Node *tree) 
    : tree_(tree), globalScope_(new Scope(nullptr, "GLOBAL")), warnings_(0), errors_(0), compoundLevel_(0) {
        // push the global scope onto the stack?
        scopes_.push(globalScope_);
        analyze();
    }

    /**
     * @fn traverseGlobals
     * @brief Depth-first search traversal of the AST for global variables and functions.
     * @param node The node to start the traversal from.
     */
    void traverseGlobals(node::Node *node);

    /**
     * @fn traverseLocals
     * @brief Depth-first search traversal of the AST for local variables.
     * @param node The node to start the traversal from.
     */
    void traverseLocals(node::Node *node);

    /**
     * @fn bfsTraversal
     * @brief Breadth-first search traversal of the AST.
     * @param root The root of the AST.
     */
    void bfsTraversal(node::Node *root);

    /**
     * @fn analyze
     * @brief Analyzes the AST.
     */
    void analyze();

    /**
     * @fn getGlobalScope
     * @brief Returns the global scope.
     * @return Scope*
     */
    Scope* getGlobalScope() { return globalScope_; }

    /**
     * @fn printWarnings
     * @brief Prints the number of warnings.
     */
    void printWarnings();
    
    /**
     * @fn printErrors
     * @brief Prints the number of errors.
     */
    void printErrors();

    /**
     * @fn insertGlobalSymbol
     * @brief Inserts a global symbol into the global scope.
     * @param node The node to insert.
     */
    void insertGlobalSymbol(node::Node *node) { globalScope_->insertSymbol(node); }

    bool leaveScope(Scope *scope);

    bool enterScope(Scope *scope);

    Scope* getScope(std::string name);

    /**
     * @fn printScopes
     * @brief Prints the stack of scopes in reverse order.
     * @return void
     */
    void printScopes();
};

#pragma endregion SemanticAnalyzer

} // namespace semantic

#endif // SEMANTIC_HPP