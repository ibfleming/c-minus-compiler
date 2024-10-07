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
        cout << "Symbol Inserted: " << name << endl;
        #endif
        return true;
    }
    #if PENDANTIC_DEBUG
    cout << "Symbol Exists Already: " << name << endl;
    #endif
    return false;
}

node::Node* SymbolTable::lookupSymbol(const node::Node* sym) {
    for (auto const& item : symbols_) {
        if ( item.first == sym->getString() ) {
            #if PENDANTIC_DEBUG
            cout << "Symbol Found: \"" << sym->getString() << "\", "; 
            cout << types::literalNodeTypeStr(item.second->getNodeType()) << endl;
            #endif
            return item.second;
        }
    }
    #if PENDANTIC_DEBUG
    cout << "Symbol NOT Found: \"" << sym->getString() << "\"" << endl;
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
            string nodeType = types::literalNodeTypeStr(val->getNodeType());
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
    cout << endl;
    cout << string(SPACE + 2, '*') << endl;
    cout << "(" << scope->getParent()->getLine() << ") ENTERING SCOPE: " << scope->getName() << endl;
    cout << string(SPACE + 2, '*') << endl;
    cout << endl;
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
        cout << endl;
        cout << string(SPACE + 2, '#') << endl;
        cout << "(" << scope->getParent()->getLine() << ") LEAVING SCOPE: " << scope->getName() << endl;
        scope->printScope();
        cout << string(SPACE + 2, '#') << endl;
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
    #if PENDANTIC_DEBUG
    cout << "[Symbol Insert]:" << endl;
    #endif
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
    #if PENDANTIC_DEBUG
    cout << "[Symbol Insert]:" << endl;
    #endif
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

node::Node* SemanticAnalyzer::checkVariableDeclaration(node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Check Variable]:" << endl;
    #endif

    // (1) Check if the variable is being used as a function (all functions declared in global scope)
    if (auto decl = lookupGlobalSymbol(sym)) {
        if( decl->getNodeType() == NT::FUNCTION ) {
            logger::ERROR_VariableAsFunction(this, sym);
        }
        decl->setIsUsed(true);
        sym->setVarType(decl->getVarType());
        return decl;
    }

    // (2) Lambda function to set the variable as used
    auto setVarUsed = [](Scope* scope, node::Node* sym) -> node::Node* {
        auto decl = scope->lookupSymbol(sym);
        if (decl) { 
            decl->setIsUsed(true); 
            sym->setVarType(decl->getVarType());
            return decl;
        }
        return nullptr;
    };

    // (3) Check if the variable is declared in other scopes
    if (getScopeCount() > 1) {
        for (auto& scope : utils::stackToVector(scopes_)) {
            #if PENDANTIC_DEBUG
            cout << (&scope == &scopes_.top() ? "[Local] " : "[Scopes] ");
            #endif
            if (auto decl = setVarUsed(scope, sym)) return decl;
        }
        logger::ERROR_VariableNotDeclared(this, sym);
        return nullptr;
    }
    // (4) Check if the variable is declared in the current local scope 
    else if (auto decl = lookupLocalSymbol(sym)) {
        decl->setIsUsed(true);
        sym->setVarType(decl->getVarType());
        return decl;
    }
    // (5) Default case, not found in any scope 
    else {
        logger::ERROR_VariableNotDeclared(this, sym);
        return nullptr;
    }
    return nullptr;
}

node::Node* SemanticAnalyzer::processCall(node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Process Call]:" << endl;
    #endif

    node::Node* decl = nullptr;

    // (1) Check if the function is declared in the global scope as all functions are declared there
    if (auto func = lookupGlobalSymbol(sym)) {
        sym->setIsUsed(true);
        sym->setVarType(func->getVarType());
        decl = func;
    }

    // (2) Default case, not found in global scope so function is not declared
    if(decl == nullptr) {
        logger::ERROR_VariableNotDeclared(this, sym);
    }

    for(auto child: sym->getChildren()) {
        if(child != nullptr) {
            child->setIsVisited(true);
            switch(child->getNodeType()) {
                case NT::ID:
                    if (auto decl = checkVariableDeclaration(child)) {
                        if (!decl->getIsInitialized()) {
                            decl->setIsInitialized(true);
                            logger::WARN_VariableNotInitialized(this, child);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return decl;
}

void SemanticAnalyzer::checkLinker() {
    auto decl = getGlobalScope()->getTable()->getSymbols();
    bool isValidMain = false;
    for (auto& symbol : decl) {
        if (symbol.first == "main") {
            auto node = symbol.second;
            if (node->getChildren().size() == 1 && node->getVarType() == types::VarType::VOID) {
                isValidMain = true;
                break;
            }
        }
    }
    if (!isValidMain) {
        logger::ERROR_Linker(this);
    }
}

node::Node* SemanticAnalyzer::processArrayIndex(node::Node* sym) {
    /*
    sym = ID_ARRAY = '['

    '[' -> ID, INDEX (NUMCONST)

    (1) Access children of array and get the first child, ID
    (2) check if ID is declared
    (3) if so, get the node, and check if its an array
    (4) if so, get the index node and check if its a number
    (5) if so, check if the index is within the bounds of the array???
    (6) set the var type of the sym to the var type of the found symbol for the ID
    */

    #if PENDANTIC_DEBUG
    cout << "[Process Array]" << endl;
    #endif

    // (1)
    auto id = sym->getChildren()[0];
    auto index = sym->getChildren()[1];

    if( id == nullptr || index == nullptr ) return nullptr;

    /** 
     * This particular ID in the AST must be marked as visited
     * very well could be duplicate IDs in the AST after this one
     * given how parsing ID_ARRAYS makes this behavior
     * i.e.
     * int c[3]; <-- declaration
     * c[1];     <-- ID_ARRAY -- this function processes this! Marks the 'c' ID within as visited
     * c;        <-- Just a regular ID of c[] and therefore will not be processed by this function
     */
    id->setIsVisited(true); index->setIsVisited(true);

    // ID[INDEX]

    switch (index->getNodeType()) {
        case NT::ID:
            if (auto decl = checkVariableDeclaration(index)) {
                if (decl->getVarType() != VT::INT) {
                    logger::ERROR_ArrayIndexNotInt(this, id, index);
                }
                if (decl->getNodeType() == NT::VARIABLE_ARRAY) {
                    logger::ERROR_ArrayIndexIsUnindexedArray(this, index);
                }
            }
            break;
        case NT::ID_ARRAY:
            if (auto decl = processArrayIndex(index)) {
                if (decl->getVarType() != VT::INT) {
                    logger::ERROR_ArrayIndexNotInt(this, id, index);
                }
            }
            break;
        default:
            break;
    }

    // (2)
    if (auto decl = checkVariableDeclaration(id)) {
        sym->setVarType(decl->getVarType()); // set the var type of the array to the var type of the ID

        if( decl->getNodeType() != NT::VARIABLE_ARRAY && 
            decl->getNodeType() != NT::PARAMETER_ARRAY && 
            decl->getNodeType() != NT::VARIABLE_STATIC_ARRAY ) 
        {
            logger::ERROR_CannotIndexNonArray(this, id);
        }

        return decl;
    }

    return nullptr;
}

void SemanticAnalyzer::checkTypes(node::Node *op, node::Node *lhs, node::Node *rhs) {
    #if PENDANTIC_DEBUG
    cout << "[Process Types - ";
    #endif

    /* In some cases the rhs is a nullptr! */

    if ( lhs && rhs ) {
        #if PENDANTIC_DEBUG
        cout << "Binary]" << endl;
        #endif

        switch(op->getNodeType()) {
            case NT::ASSIGNMENT: {
                switch(op->getAsgnType()) {
                    case AT::ASGN:
                        op->setVarType(lhs->getVarType());
                        if (lhs->getVarType() != rhs->getVarType()) {
                            logger::ERROR_RequiresOperandsEqualTypes(this, op, lhs, rhs);
                        }
                        return;
                    case AT::ADDASGN:
                    case AT::SUBASGN:
                    case AT::MULASGN:
                    case AT::DIVASGN:
                        if (lhs->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, lhs, logger::OperandType::LHS);
                        }
                        if (rhs->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, rhs, logger::OperandType::RHS);
                        }
                        return;
                    default:
                        return;
                }
            }
            case NT::OPERATOR: {
                switch(op->getOpType()) {
                    case OT::ADD:
                    case OT::SUB:
                    case OT::MUL:
                    case OT::DIV:
                    case OT::MOD:
                        if (lhs->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, lhs, logger::OperandType::LHS);
                        }
                        if (rhs->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, rhs, logger::OperandType::RHS);
                        } 
                        return;
                    case OT::LESS:
                    case OT::GREATER:
                    case OT::LEQ:
                    case OT::GEQ:
                    case OT::EQL:
                    case OT::NEQ:
                        if (lhs->getVarType() != rhs->getVarType()) {
                            logger::ERROR_RequiresOperandsEqualTypes(this, op, lhs, rhs);
                        }
                        return;
                    default:
                        return;
                }
            }
            case NT::AND:
            case NT::OR:
                if (lhs->getVarType() != VT::BOOL) {
                    logger::ERROR_RequiresOperandBoolTypes(this, op, lhs, logger::OperandType::LHS);
                }
                return;
            default:
                return;
        }
    }
    else if ( lhs ) { // Unary (++, --, chsign, -, sizeof, ques, not)
        #if PENDANTIC_DEBUG
        cout << "Unary]" << endl;
        #endif
        if( op->getVarType() != lhs->getVarType() ) {
            logger::ERROR_UnaryRequiresOperandSameType(this, op, lhs);
            return;
        }
    }
    else {
        throw runtime_error("ERROR: LHS OR RHS IS NULL! (in checkTypes())");
    }
}

void SemanticAnalyzer::processReturn(node::Node *sym) {}

void SemanticAnalyzer::processOperation(node::Node *operation) {

    /**
     * POTENTIAL NOTICE OF IMPORTANCE!
     * 
     * We should check the variable of the ID in higher scopes in comparison to the rhs 
     * Rhs is more local
     * 
     */

    #if PENDANTIC_DEBUG
    cout << "[Process Operation: " << types::literalNodeTypeStr(operation->getNodeType()) << "] ";
    #endif

    int children = operation->getChildren().size();

    if (children == 1) { // Unary (++, --, chsign, -, sizeof, ques, not)
        auto operand = operation->getChildren()[0];

        if( operand == nullptr ) throw runtime_error("ERROR: OPERAND IS NULL! (in processOperation())");

        operand->setIsVisited(true);

        switch (operand->getNodeType()) {
            case NT::ID:
                if (auto decl = checkVariableDeclaration(operand)) {
                    if (!decl->getIsInitialized()) {
                        decl->setIsInitialized(true);
                        logger::WARN_VariableNotInitialized(this, operand);
                    }
                }
                break;
            default:
                break;
        }

        checkTypes(operation, operand, nullptr);
    }

    if (children == 2) { // Binary (most operators and assignments)
        auto lhs = operation->getChildren()[0];
        auto rhs = operation->getChildren()[1];

        bool lhsNotDeclared = false;
        bool rhsNotDeclared = false;

        if( lhs == nullptr || rhs == nullptr ) throw runtime_error("ERROR: LHS OR RHS IS NULL! (in processOperation())");

        lhs->setIsVisited(true); rhs->setIsVisited(true);
        
        switch (lhs->getNodeType()) {
            case NT::ID:
                if (auto decl = checkVariableDeclaration(lhs)) {
                    decl->setIsInitialized(true); // maybe only required for the := assignment
                } else {
                    lhsNotDeclared = true;
                }
                break;
            case NT::ID_ARRAY:
                if (auto decl = processArrayIndex(lhs)) {
                    decl->setIsInitialized(true); // maybe only required for the := assignment
                }
                break;
            case NT::CALL:
                processCall(lhs);
                break;
            default:
                break;
        }

        switch (rhs->getNodeType()) { // definitely check if vars are initialized on RHS
            case NT::ID:
                if (auto decl = checkVariableDeclaration(rhs)) {
                    if (!decl->getIsInitialized()) {
                        decl->setIsInitialized(true);
                        logger::WARN_VariableNotInitialized(this, rhs);
                    }
                } else {
                    rhsNotDeclared = true;
                }
                break;
            case NT::ID_ARRAY:
                if (auto decl = processArrayIndex(lhs)) {
                    if (!decl->getIsInitialized()) {
                        decl->setIsInitialized(true);
                        logger::WARN_VariableNotInitialized(this, lhs);
                    }
                }
                break;
            case NT::CALL:
                processCall(rhs);
                break;
            default:
                break;
        }

        if (lhsNotDeclared || rhsNotDeclared) {
            return;
        }

        checkTypes(operation, lhs, rhs);
    }

    return;
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

void SemanticAnalyzer::processSemantics(node::Node *node) {

    // SCOPES
    switch (node->getNodeType()) {
        case NT::FUNCTION:
            enterScope(new Scope(node, "FUNCTION_" + node->getString()));
            return;
        case NT::COMPOUND:
            if (node->getFunctionNode() == nullptr) {
                enterScope(new Scope(node, "COMPOUND_" + to_string(compoundLevel_))); 
                compoundLevel_++;
            }
            return;
        case NT::FOR:
            if (node->getFunctionNode() == nullptr) {
                enterScope(new Scope(node, "FOR_" + to_string(compoundLevel_))); 
                compoundLevel_++;
            }
        default:
            break;
    }

    // OTHER NODES
    if( !node->getIsVisited() ) {
        switch(node->getNodeType()) {
            // DECLARATIONS
            case NT::VARIABLE:
            case NT::VARIABLE_STATIC:
            case NT::PARAMETER:
                #if PENDANTIC_DEBUG
                cout << "(" << node->getLine() << ") " ;
                #endif
                insertLocalSymbol(node);
                return;
            // DECLARATIONS (ARRAYS)
            case NT::VARIABLE_ARRAY:
            case NT::VARIABLE_STATIC_ARRAY:
            case NT::PARAMETER_ARRAY:
                #if PENDANTIC_DEBUG
                cout << "(" << node->getLine() << ") [Array] ";
                #endif
                insertLocalSymbol(node);
                return;
            // IDENTIFIERS/VARIABLES
            case NT::ID:
                if (auto decl = checkVariableDeclaration(node)) {
                    if (!decl->getIsInitialized()) {
                        decl->setIsInitialized(true);
                        logger::WARN_VariableNotInitialized(this, node);
                    }
                }
                return;
            case NT::ID_ARRAY:
                if (auto decl = processArrayIndex(node)) {
                    if (!decl->getIsInitialized()) {
                        decl->setIsInitialized(true);
                        logger::WARN_VariableNotInitialized(this, node);
                    }
                }
                return;
            case NT::CALL:
                processCall(node);
                return;
            case NT::ASSIGNMENT:
            case NT::OPERATOR:
            case NT::QUES_UNARY:
            case NT::CHSIGN_UNARY:
            case NT::NOT:
            case NT::AND:
            case NT::OR:
                #if PENDANTIC_DEBUG
                cout << "(" << node->getLine() << ") " ;
                #endif
                processOperation(node);
                return;
            case NT::SIZEOF_UNARY:
            default:
                return;
        }
    }
}

void SemanticAnalyzer::traverseGlobals(node::Node *node) {
    if (node == nullptr) { return; }

    switch (node->getNodeType()) {
        case NT::FUNCTION:
        case NT::VARIABLE:
        case NT::VARIABLE_ARRAY:
        case NT::VARIABLE_STATIC:
        case NT::VARIABLE_STATIC_ARRAY:
            node->setIsVisited(true);
            node->setIsInitialized(true);
            insertGlobalSymbol(node);
    }

    // Do not traverse into children as this access the local scopes

    if (node->getSibling() != nullptr) {
        traverseGlobals(node->getSibling());
    }
}

void SemanticAnalyzer::traverseLocals(node::Node *node) {
    if (node == nullptr) { return; }

    processSemantics(node);

    // Traverse children nodes recursively first (inner blocks)
    for (node::Node *child : node->getChildren()) {
        traverseLocals(child);
    }

    // After children, check sibling node (for next block at the same level)
    if (node->getSibling() != nullptr) {
        if (node->getNodeType() == NT::COMPOUND || node->getNodeType() == NT::FOR) {
            compoundLevel_--; leaveScope();
        }
        traverseLocals(node->getSibling());
    }
    else {
        if (node->getNodeType() == NT::COMPOUND || node->getNodeType() == NT::FOR) {
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
    
    checkLinker();

    #if PENDANTIC_DEBUG
    printGlobal(); // will only print the global scope
    #endif

    traverseLocals(tree_);

    printWarnings();
    printErrors();
}

#pragma endregion Traversal

}