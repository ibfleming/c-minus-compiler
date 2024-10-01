/*********************************************************************
 * @file semantic.hpp
 * 
 * @brief Header file for semantic analysis.
 *********************************************************************/

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#define PENDANTIC_DEBUG false

#include <map>
#include <iostream>

/**
 * @namespace semantic
 * @brief Contains the semantic analysis members and functions.
 */
namespace semantic {

class SymbolTable {

private:
    std::map<std::string, node::Node*> declarations_;  // Map of variable declarations, name -> node
    bool debug_;

public:
    SymbolTable(): debug_(false) {};

    void setDebug(bool debug) { debug_ = debug; }

    bool insert(node::Node *node) {
        std::string name = node->getString();
        if (declarations_.find(name) == declarations_.end()) {
            declarations_[name] = node;
            #if PENDANTIC_DEBUG
            std::cout << "Inserted: " << name << std::endl;
            #endif
            return true;
        }
        #if PEDANTIC_DEBUG
        std::cout << "Error: " << name << " already declared." << std::endl;
        #endif
        return false;
    }

    node::Node* lookup(std::string name) {
        if (declarations_.find(name) != declarations_.end()) {
            #if PENDANTIC_DEBUG
            std::cout << "Lookup: " << name << std::endl;
            #endif
            return declarations_[name];
        }
        #if PENDANTIC_DEBUG
        std::cout << "Error: " << name << " not declared." << std::endl;
        #endif
        return nullptr;
    }

    void enterScope();
    void exitScope();

};

class SemanticAnalyzer {

private:
    node::Node *tree_;           // Root of the AST
    SymbolTable *symbolTable_;  // Symbol table for the semantic analyzer
    int warnings_;              // Number of warnings
    int errors_;                // Number of errors

public:
    SemanticAnalyzer(node::Node *tree) : tree_(tree), symbolTable_(new SymbolTable()), warnings_(0), errors_(0) {}

    /**
     * @fn getTable
     * @brief Returns the symbol table.
     */
    SymbolTable* getTable() { return symbolTable_; }

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

};

} // namespace semantic

#endif // SEMANTIC_HPP