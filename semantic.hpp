/*********************************************************************
 * @file semantic.hpp
 *
 * @brief Header file for semantic analysis.
 *********************************************************************/

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#define PENDANTIC_DEBUG 0
#define SPACE 64

#include "logger.hpp"
#include "node.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <stack>

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
    std::map<std::string, node::Node*> getSymbols()
    {
        return symbols_;
    }

    /**
     * @fn getSize
     * @brief Returns the size of the symbol table.
     * @return int
     */
    int getSize()
    {
        return symbols_.size();
    }

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
    SymbolTable table_; // Symbol table for the scope
    node::Node* parent_; // Node of the scope (FUNCTION, COMPOUND, LOOP, etc.)
    std::string name_; // Name of the scope
    int location_; // Location of the scope relative to other scopes?

public:
    /**
     * @fn Scope
     * @brief Constructor for the scope.
     * @param parent The parent node of the scope.
     */
    Scope(node::Node* parent)
        : parent_(parent)
        , name_(types::literalNodeTypeStr(parent->getNodeType()))
        , location_(-1)
    {
    }
    /**
     * @fn Scope
     * @brief Constructor for the scope.
     * @param parent The parent node of the scope.
     * @param name The name of the scope.
     */
    Scope(node::Node* parent, std::string name)
        : parent_(parent)
        , name_(name)
        , location_(-1)
    {
    }

    /***********************************************
     *  ACCESSORS
     ***********************************************/

    /**
     * @fn getSymbols
     * @brief Returns the symbol table of the scope.
     * @return SymbolTable*
     */
    SymbolTable* getTable()
    {
        return &table_;
    }

    /**
     * @fn getParent
     * @brief Returns the parent node of the scope.
     * @return node::Node*
     */
    node::Node* getParent()
    {
        return parent_;
    }

    /**
     * @fn getName
     * @brief Returns the name of the scope.
     * @return std::string
     */
    std::string getName()
    {
        return name_;
    }

    /***********************************************
     *  SYMBOL TABLE MANAGEMENT
     ***********************************************/

    /**
     * @fn insertSymbol
     * @brief Inserts a symbol into the symbol table.
     * @param node The node to insert.
     * @return bool
     */
    bool insertSymbol(node::Node* node)
    {
        return table_.insertSymbol(node);
    }

    /**
     * @fn lookupSymbol
     * @brief Looks up a symbol in the symbol table.
     * @param name The name of the symbol to lookup.
     * @return node::Node*
     */
    node::Node* lookupSymbol(const node::Node* sym)
    {
        return table_.lookupSymbol(sym);
    }

    /**
     * @fn checkUsedVariables
     * @brief Checks for unused variables in the scope.
     * @return void
     */
    void checkUsedVariables(semantic::SemanticAnalyzer* analyzer);

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
    node::Node* tree_; // Root of the AST
    Scope* globalScope_; // Global scope
    std::stack<Scope*> scopes_; // Stack of scopes
    int compoundLevel_; // Tracks the level of compounds in the scope stack
    int warnings_; // Number of warnings
    int errors_; // Number of errors

public:
    /**
     * @fn SemanticAnalyzer
     * @brief Constructor for the semantic analyzer.
     * @param tree Root of the AST.
     */
    SemanticAnalyzer(node::Node* tree)
        : tree_(tree)
        , globalScope_(new Scope(nullptr, "GLOBAL"))
        , warnings_(0)
        , errors_(0)
        , compoundLevel_(0)
    {
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
    void enterScope(Scope* scope);

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
    Scope* getCurrentScope()
    {
        return scopes_.top();
    }

    /**
     * @fn getGlobalScope
     * @brief Returns the global scope.
     * @return Scope*
     */
    Scope* getGlobalScope()
    {
        return globalScope_;
    }

    /**
     * @fn getScopeCount
     * @brief Returns the number of scopes on the stack.
     * @return int
     */
    int getScopeCount()
    {
        return scopes_.size();
    }

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
    void printGlobal()
    {
        globalScope_->printScope();
    }

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
    bool insertSymbol(node::Node* sym);

    /**
     * @fn lookupSymbol
     * @brief Looks up a declaration in the current scope and other scopes if there are more on the stack.
     * @param id The identifier to lookup.
     * @param init If the symbol is being check for initialization or not, check it's lookup logic.
     * @return node::Node*
     */
    node::Node* lookupSymbol(node::Node* id, bool init = true);

#pragma region Semantic_M

    /***********************************************
     *  SEMANTIC ANALYSIS FUNCTIONS
     ***********************************************/

    void checkLinker();
    void checkForInitializer(node::Node* var);
    void checkInit(node::Node* decl);
    node::Node* checkBinaryTypes(node::Node* op, node::Node* lhs, node::Node* rhs);
    node::Node* checkUnaryTypes(node::Node* op, node::Node* operand);
    void checkForUse(Scope* scope)
    {
        scope->checkUsedVariables(this);
    }
    void processReturn(node::Node* ret);
    node::Node* processArray(node::Node* arr,
        bool init = true); // @note true = check for initialization, false = apply initialization
    node::Node* processCall(node::Node* call, bool init = true);
    node::Node* processIdentifier(
        node::Node* id, bool init = true); // @note true = check for initialization, false = apply initialization
    node::Node* processOperator(node::Node* op);
    void processUnaryOperation(node::Node* op);
    void processIf(node::Node* op);
    void processWhile(node::Node* op);
    void processAssignment(node::Node* node);
    void processBinaryOperator(node::Node* node);
    void processUnaryOperator(node::Node* node);
    void processBooleanBinaryOperator(node::Node* node);
    bool isDeclarationFunctionAsVariable(node::Node* id, node::Node* decl);

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
    void analyzeNode(node::Node* node);

    /**
     * @fn traverse
     * @brief Depth-first search traversal of the AST for local variables.
     * @param node The node to start the traversal from.
     */
    void traverse(node::Node* node);

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
    void incWarnings()
    {
        warnings_++;
    }

    /**
     * @fn incErrors
     * @brief Incremenet the warnings
     * @return void
     */
    void incErrors()
    {
        errors_++;
    }

    /**
     * @fn printWarnings
     * @brief Prints the number of warnings.
     */
    void printWarnings()
    {
        std::cout << "Number of warnings: " << warnings_ << std::endl;
        std::flush(std::cout);
    }

    /**
     * @fn printErrors
     * @brief Prints the number of errors.
     */
    void printErrors()
    {
        std::cout << "Number of errors: " << errors_ << std::endl;
        std::flush(std::cout);
    }

#pragma endregion ErrWarns_M
};

#pragma endregion Analyzer

} // namespace semantic

#endif // SEMANTIC_HPP