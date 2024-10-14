/*********************************************************************
 * @file semantic.hpp
 * 
 * @brief Header file for semantic analysis.
 *********************************************************************/

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#define PENDANTIC_DEBUG true
#define SPACE 64

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
        enterGlobalScope();
        analyze();
    }

    /***********************************************
    *  SCOPE MANAGEMENT
    ***********************************************/

    #pragma region Scope_M

    /**
     * @fn enterGlobalScope
     * @brief Enters the global scope.
     * @return void
     */
    void enterGlobalScope();

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
     * @fn insertSymbol
     * @brief Inserts a symbol into the current scope.
     * @param sym The symbol to insert.
     * @return bool
     */
    bool insertSymbol(node::Node *sym);

    /**
     * @fn lookupSymbol
     * @brief Looks up a declaration in the current scope and other scopes if there are more on the stack.
     * @param id The identifier to lookup.
     * @return node::Node*
     */
    node::Node* lookupSymbol(node::Node* id);

    #pragma region Semantic_M

    /***********************************************
    *  SEMANTIC ANALYSIS FUNCTIONS
    ***********************************************/

    /**
     * @fn checkLinker
     * @brief Checks for the presence of a main function.
     */
    void checkLinker();

    /**
     * @fn checkForInitializer
     * @brief Checks for the presence of an initializer in a variable declaration.
     * @param var The variable to check.
     */
    void checkForInitializer(node::Node *var);

    /**
     * @fn checkInitialization
     * @brief Checks if a variable is initialized.
     * @param id The identifier to check.
     * @return node::Node*
     * @note Will emit warnings and set init to true for the variable declaration.
     */
    node::Node* checkInitialization(node::Node *id);

    /**
     * @fn applyInitialization
     * @brief Applies initialization to a variable declaration.
     * @param id The identifier to apply initialization to.
     * @return node::Node*
     * @note Will only make the variable initialized. Intended for LHS in ASGN and other special cases.
     */
    node::Node* applyInitialization(node::Node *id);
    
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
     * @fn checkForUse
     * @brief Checks for the use of a variables in the current scope.
     * @param scope The scope to check.
     */
    void checkForUse(Scope *scope) { scope->checkUsedVariables(this); }

    /**
     * @fn processReturn
     * @brief Processes a return node.
     * @param ret The return node to process.
     */
    void processReturn(node::Node *ret);

    /**
     * @fn processArray
     * @brief Processes an array  node.
     * @param arr The array node to process.
     * @param isLHSinASGN Is the array the left-hand side of an assignment?
     */
    node::Node* processArray(node::Node* arr, bool isLHSinASGN);

    /**
     * @fn processCall
     * @brief Checks for the use of a function in the current scope.
     * @param call The function to check.
     */
    node::Node* processCall(node::Node* call);

    /**
     * @fn processAssignment
     * @brief Processes an assignment node. (:=)
     * @param asgn The assignment node to process.
     * @note The assignment node is a binary operation and is a special case.
     */
    void processAssignment(node::Node *asgn);

    /**
     * @fn processIdentifier
     * @brief Processes an identifier node.
     * @param id The identifier node to process.
     * @param isLHSinASGN Is the identifier the left-hand side of an assignment?
     */
    node::Node* processIdentifier(node::Node *id, bool isLHSinASGN);

    /**
     * @fn processOperator
     * @brief Processes an operator node.
     * @param op The operator node to process.
     * @param isLHSinASGN Is the operator the left-hand side of an assignment?
     * @return node::Node*
     */
    node::Node* processOperator(node::Node *op, bool isLHSinASGN, bool useArray);

    /**
     * @fn processBinaryOperation
     * @brief Processes a binary operation node.
     * @param op The binary operation node to process.
     */
    void evaluateOperation(node::Node *op);

    /**
     * @fn processUnaryOperation
     * @brief Processes a unary operation node.
     * @param op The unary operation node to process.
     */
    void processUnaryOperation(node::Node *op);
  
    /**
     * @fn processIf
     * @brief Processes an if node.
     * @param op The if node to process.
     */
    void processIf(node::Node *op);

    /**
     * @fn processWhile
     * @brief Processes a while node.
     * @param op The while node to process.
     */
    void processWhile(node::Node *op);

    /**
     * @fn processBooleanBinaryOperators
     * @brief Processes boolean binary operators.
     * @param op The boolean binary operator node to process.
     * @note The operators are AND and OR.
     */
    void processBooleanBinaryOperators(node::Node *op);

    #pragma endregion Semantic_M

    #pragma endregion Table_M

    /***********************************************
    *  TRAVERSAL & ANALYSIS
    ***********************************************/

    #pragma region Traversal_M

    /**
     * @fn analyzeNode
     * @brief Processes the semantics of the AST.
     * @param node The node to process.
     */
    void analyzeNode(node::Node *node);

    /**
     * @fn traverse
     * @brief Depth-first search traversal of the AST for local variables.
     * @param node The node to start the traversal from.
     */
    void traverse(node::Node *node);

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