/*********************************************************************
 * @file semantic.hpp
 * 
 * @brief Header file for semantic analysis.
 *********************************************************************/

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#define PENDANTIC_DEBUG true
#define SPACE 48

#include "utils.hpp"
#include "types.hpp"
#include "node.hpp"
#include "logger.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stack>
#include <map>

typedef types::NodeType NT;
typedef types::VarType VT;
typedef types::AssignmentType AT;
typedef types::OperatorType OT;

/**
 * @namespace semantic
 * @brief Contains the semantic analysis members and functions.
 */
namespace semantic {

class Scope;
class SemanticAnalyzer;

#pragma region SymbolTable

/**
 * @class SymbolTable
 * @brief Represents a symbol table.
 */
class SymbolTable {

private:
    // Map of variable declarations, (name => node)
    std::map<std::string, node::Node*> symbols_;
public:
    /**
     * @fn SymbolTable
     * @brief Constructor for the symbol table.
     */
    SymbolTable() = default;

    /***********************************************
    *  ACCESSORS
    ***********************************************/

    /**
     * @fn getSymbols
     * @brief Returns the symbols in the symbol table.
     * @return std::map<std::string, node::Node*>
     */
    std::map<std::string, node::Node*> getSymbols() { return symbols_; }

    /**
     * @fn getSize
     * @brief Returns the size of the symbol table.
     * @return int
     */
    int getSize() { return symbols_.size(); }

    /***********************************************
    *  SYMBOL MANAGEMENT
    ***********************************************/

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
    node::Node* lookupSymbol(const node::Node* sym);

};

#pragma endregion SymbolTable

#pragma region Scope

/**
 * @class Scope
 * @brief Represents a scope in the program.
 */
class Scope {

private:
    SymbolTable table_;     // Symbol table for the scope
    node::Node *parent_;    // Node of the scope (FUNCTION, COMPOUND, LOOP, etc.)
    std::string name_;      // Name of the scope
    int location_;          // Location of the scope relative to other scopes?

public:
    /**
     * @fn Scope
     * @brief Constructor for the scope.
     * @param parent The parent node of the scope.
     */
    Scope(node::Node *parent)
     : parent_(parent), name_(types::literalNodeTypeStr(parent->getNodeType())), location_(-1) {}
    /**
     * @fn Scope
     * @brief Constructor for the scope.
     * @param parent The parent node of the scope.
     * @param name The name of the scope.
     */
    Scope(node::Node *parent, std::string name) : parent_(parent), name_(name), location_(-1) {}

    /***********************************************
    *  ACCESSORS
    ***********************************************/

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

    /***********************************************
    *  SYMBOL TABLE MANAGEMENT
    ***********************************************/

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
    node::Node* lookupSymbol(const node::Node* sym) { return table_.lookupSymbol(sym); }

    /**
     * @fn checkUsedVariables
     * @brief Checks for unused variables in the scope.
     * @return void
     */
    void checkUsedVariables(semantic::SemanticAnalyzer *analyzer);

    /***********************************************
    *  SCOPE INFORMATION
    ***********************************************/

    /**
     * @fn printScope
     * @brief Prints the scope and its contained symbols.
     * @return void
     */
    void printScope();

};

#pragma endregion Scope

#pragma region Analyzer

/**
 * @class SemanticAnalyzer
 * @brief Analyzes the AST for semantic errors and warnings.
 */
class SemanticAnalyzer {

private:
    node::Node *tree_;          // Root of the AST
    Scope *globalScope_;        // Global scope
    std::stack<Scope*> scopes_; // Stack of scopes
    int compoundLevel_;         // Tracks the level of compounds in the scope stack
    int warnings_;              // Number of warnings
    int errors_;                // Number of errors

public:
    /**
     * @fn SemanticAnalyzer
     * @brief Constructor for the semantic analyzer.
     * @param tree Root of the AST.
     */
    SemanticAnalyzer(node::Node *tree) 
    : tree_(tree), globalScope_(new Scope(nullptr, "GLOBAL")), warnings_(0), errors_(0), compoundLevel_(0) {
        #if PENDANTIC_DEBUG
        std::cout << std::endl;
        std::cout << std::string(SPACE + 2, '=') << std::endl;
        std::cout << "Analyzing the AST..." << std::endl;
        std::cout << std::string(SPACE + 2, '=') << std::endl;
        std::cout << std::endl;
        #endif
        analyze();
    }

    /***********************************************
    *  SCOPE MANAGEMENT
    ***********************************************/

    #pragma region Scope_M

    /**
     * @fn enterScope
     * @brief Enters a new scope, i.e. pushes a new scope onto the stack.
     * @param scope The scope to enter.
     */
    void enterScope(Scope *scope);

    /**
     * @fn leaveScope
     * @brief Leaves the current scope, i.e. pops the top scope on the stack.
     */
    void leaveScope();

    /**
     * @fn getCurrentScope
     * @brief Gets the current scope, i.e. scope on the top of the stack.
     * @return Scope*
     */
    Scope* getCurrentScope() { return scopes_.top(); }

    /**
     * @fn getGlobalScope
     * @brief Returns the global scope.
     * @return Scope*
     */
    Scope* getGlobalScope() { return globalScope_; }

    /**
     * @fn getScopeCount
     * @brief Returns the number of scopes on the stack.
     * @return int
     */
    int getScopeCount() { return scopes_.size(); }

    /**
     * @fn printScopes
     * @brief Prints the stack of scopes in reverse order.
     * @return void
     */
    void printScopes();

    /**
     * @fn printGlobal
     * @brief Prints the global scope.
     * @return void
     */
    void printGlobal() { globalScope_->printScope(); }

    #pragma endregion Scope_M

    /***********************************************
    *  SYMBOL TABLE MANAGEMENT
    ***********************************************/

    #pragma region Table_M

    /**
     * @fn insertGlobalSymbol
     * @brief Inserts a global symbol into the global scope.
     * @param sym The symbol to insert.
     */
    void insertGlobalSymbol(node::Node *sym);

    /**
     * @fn insertLocalSymbol
     * @brief Inserts a local symbol into the current scope.
     * @param sym The symbol to insert
     */
    void insertLocalSymbol(node::Node *sym);

    /**
     * @fn lookupGlobalSymbol
     * @brief Looks up a symbol in the global scope for analysis.
     * @param sym The symbol to lookup.
     * @return node::Node*
     */
    node::Node* lookupGlobalSymbol(const node::Node* sym);

    /**
     * @fn lookupLocalSymbol
     * @brief Looks up a symbol in the current scopeo and other scopes if there are more on the stack for analysis.
     * @param sym The symbol to lookup.
     * @return node::Node*
     */
    node::Node* lookupLocalSymbol(const node::Node* sym);

    /**
     * @fn checkThroughScopes
     * @brief Checks through the scopes on the stack for a symbol.
     * @param sym The symbol to check for.
     * @return node::Node*
     */
    node::Node* lookupAllScopes(node::Node* sym);

    /**
     * @fn checkForDeclaration
     * @brief Checks for a declaration in the current scope of the given node, i.e. check symbol table.
     * @param node The node to check.
     * @return node::Node*
     * @note Marks the found symbol as used and sets the searched node's var type. Using stackToVectorReverse.
     */
    node::Node* checkVariableDeclaration(node::Node* sym);

    /**
     * @fn checkLHSVariableDeclaration
     * @brief Checks for the variable declaration in the current/recent scopes -- only for RHS declarations.
     * @param sym The RHS variable to check.
     * @return node::Node*
     * @note Is slightly different in logic from checkVariableDeclaration. Using stackToVectorInOrder.
     * @note Primarily looks in the most recent scopes then going for the top-level scopes (i.e. globals/functions).
     */
    node::Node* checkRHSVariableDeclaration(node::Node* sym);

    /**
     * @fn checkCallDeclaration
     * @brief Checks for the use of a function in the current scope.
     * @param sym The function to check.
     */
    node::Node* processCall(node::Node* sym);

    /**
     * @fn checkForUse
     * @brief Checks for the use of a variables in the current scope.
     * @param scope The scope to check.
     */
    void checkForUse(Scope *scope) { scope->checkUsedVariables(this); }

    /**
     * @fn checkLinker
     * @brief Checks for the presence of a main function.
     */
    void checkLinker();

    /**
     * @fn processArrayIndex
     * @brief Processes an array index node.
     * @param sym The array index node to process.
     */
    node::Node* processArrayIndex(node::Node* sym);

    /**
     * @fn checkTypes
     * @brief Checks if the operands of operators/assignments have matching types.
     * @param op The operator/assignment node.
     * @param lhs The left-hand side node.
     * @param rhs The right-hand side node.
     * @param lhsDecl The declaration of the lhs.
     * @param rhsDecl The declaration of the rhs. 
     * @note The rhs can be null depending on the operation!
     */
    void checkTypes(node::Node *op, node::Node *lhs, node::Node *rhs, node::Node *lhsDecl, node::Node* rhsDecl);

    /**
     * @fn processAssignment
     * @brief Processes an assignment node. (:=)
     * @param asgn The assignment node to process.
     * @note The assignment node is a binary operation and is a special case.
     */
    void processAssignment(node::Node *asgn);

    /**
     * @fn processReturn
     * @brief Processes a return node.
     * @param return The return node to process.
     */
    void processReturn(node::Node *sym);

    node::Node* lookupDeclaration(node::Node* id);

    void processIdentifier(node::Node *id);

    void processOperator(node::Node *operation);

    void processBinaryOperation(node::Node *op);

    void processUnaryOperation(node::Node *op);

    #pragma endregion Table_M

    /***********************************************
    *  TRAVERSAL & ANALYSIS
    ***********************************************/

    #pragma region Traversal_M

    /**
     * @fn processSemantics
     * @brief Processes the semantics of the AST.
     * @param node The node to process.
     */
    void processSemantics(node::Node *node);

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
     * @fn analyze
     * @brief Analyzes the AST.
     */
    void analyze();

    #pragma endregion Traversal_M

    /***********************************************
    *  ERRORS/WARNINGS MANAGEMENT
    ***********************************************/

    #pragma region ErrWarns_M
    
    /**
     * @fn incWarnings
     * @brief Incremenet the warnings
     * @return void
     */
    void incWarnings() { warnings_++; }

    /**
     * @fn incErrors
     * @brief Incremenet the warnings
     * @return void
     */
    void incErrors() { errors_++; }

    /**
     * @fn printWarnings
     * @brief Prints the number of warnings.
     */
    void printWarnings() { 
        std::cout << "Number of warnings: " << warnings_ << std::endl; 
        std::flush(std::cout); }

    /**
     * @fn printErrors
     * @brief Prints the number of errors.
     */
    void printErrors() { 
        std::cout << "Number of errors: " << errors_ << std::endl; 
        std::flush(std::cout); 
    }

    #pragma endregion ErrWarns_M

};

#pragma endregion Analyzer

} // namespace semantic

#endif // SEMANTIC_HPP