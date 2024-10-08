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
    auto scopes = utils::stackToVectorReverse(scopes_);
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        #if PENDANTIC_DEBUG
        cout << (it == scopes.rbegin() ? "[Local] " : "[Scope] ");
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
        for (auto& scope : utils::stackToVectorReverse(scopes_)) {
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

node::Node* SemanticAnalyzer::checkRHSVariableDeclaration(node::Node* sym) {
    #if PENDANTIC_DEBUG
    cout << "[Check RHS Variable]:" << endl;
    #endif

    // Lambda function to set the variable as used
    auto setVarUsed = [](Scope* scope, node::Node* sym) -> node::Node* {
        auto decl = scope->lookupSymbol(sym);
        if (decl) { 
            decl->setIsUsed(true); 
            sym->setVarType(decl->getVarType());
            return decl;
        }
        return nullptr;
    };

    // (1) Check if the variable is declared in other scopes
    if (getScopeCount() > 1) {
        for (auto& scope : utils::stackToVectorInOrder(scopes_)) {
            #if PENDANTIC_DEBUG
            cout << (&scope == &scopes_.top() ? "[Local] " : "[Scopes] ");
            #endif
            if (auto decl = setVarUsed(scope, sym)) return decl;
        }
        logger::ERROR_VariableNotDeclared(this, sym);
        return nullptr;
    }
    // (2) Check if the variable is declared in the current local scope 
    else if (auto decl = lookupLocalSymbol(sym)) {
        decl->setIsUsed(true);
        sym->setVarType(decl->getVarType());
        return decl;
    }
    // (3) Check if the variable is being used as a function (all functions declared in global scope)
    else if (auto decl = lookupGlobalSymbol(sym)) {
        if( decl->getNodeType() == NT::FUNCTION ) {
            logger::ERROR_VariableAsFunction(this, sym);
        }
        decl->setIsUsed(true);
        sym->setVarType(decl->getVarType());
        return decl;
    }
    // (4) Default case, not found in any scope 
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

    /**
     * (1) Not found in global so therefore isn't a function
     * Therefore check if its a simple variable through the scopes.
     * If so, print the error otherwise, it is a variable not declared...
     */
    if( decl = checkVariableDeclaration(sym) ) {
        if( decl->getNodeType() != NT::FUNCTION ) {
            logger::ERROR_CannotCallSimpleVariable(this, sym);
        }
    }

    // (3) Process the arguments of the function
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
            if (node->getChildren().size() == 1) {
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

void SemanticAnalyzer::checkTypes(node::Node *op, node::Node *lhs, node::Node *rhs, node::Node *lhsDecl, node::Node* rhsDecl) {
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
                    {
                        if (lhsDecl) {  // maybe not needed from just lhsDecl
                            op->setVarType(lhsDecl->getVarType()); 
                        }
                        if (lhs->getVarType() != rhs->getVarType()) {
                            logger::ERROR_RequiresOperandsEqualTypes(this, op, lhs, rhs);
                        }
                        if (lhsDecl && rhsDecl) {
                            if( lhsDecl->getIsArray() != rhsDecl->getIsArray() ) {
                                logger::ERROR_RequiresOperandsAsArrayTypes(this, op, lhsDecl, rhsDecl);
                            }
                        }
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
                        if (lhsDecl || rhsDecl) {
                            if (lhsDecl != nullptr && lhsDecl->getIsArray()) {
                                logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                            }
                            else if (rhsDecl != nullptr && rhsDecl->getIsArray()) {
                                logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                            }
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
                        if (lhsDecl || rhsDecl) {
                            if (lhsDecl != nullptr && lhsDecl->getIsArray()) {
                                logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                            }
                            else if (rhsDecl != nullptr && rhsDecl->getIsArray()) {
                                logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                            }
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
                if (lhsDecl || rhsDecl) {
                    if (lhsDecl != nullptr && lhsDecl->getIsArray()) {
                        logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                    }
                    else if (rhsDecl != nullptr && rhsDecl->getIsArray()) {
                        logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                    }
                }
                return;
            default:
                return;
        }
    }
    else if (lhs) { // Unary (++, --, chsign, -, sizeof, ques, not)
        #if PENDANTIC_DEBUG
        cout << "Unary]" << endl;
        #endif
        if (op->getVarType() != lhs->getVarType()) {
            logger::ERROR_UnaryRequiresOperandSameType(this, op, lhs);
            return;
        }
        switch (op->getAsgnType()) {
            case AT::INC:
            case AT::DEC:
                if (lhsDecl) {
                    if( lhsDecl->getIsArray() ) {
                        logger::ERROR_OperationCannotUseArrays(this, op, lhs);
                    }
                }
                return;
        }
    }
    else {
        throw runtime_error("ERROR: LHS OR RHS IS NULL! (in checkTypes())");
    }
}

void SemanticAnalyzer::processReturn(node::Node *sym) {
    #if PENDANTIC_DEBUG
    cout << "[Process Return] ";
    #endif

    if (sym->getChildren().size() == 1) {
        auto child = sym->getChildren()[0];

        if( child == nullptr ) throw runtime_error("ERROR: CHILD IS NULL! (in processReturn())");

        switch (child->getNodeType()) {
            case NT::ID:
                if (auto decl = checkVariableDeclaration(child)) {
                    sym->setVarType(decl->getVarType());
                    switch(decl->getNodeType()) {
                        case NT::VARIABLE_ARRAY:
                        case NT::VARIABLE_STATIC_ARRAY:
                        case NT::PARAMETER_ARRAY:
                            logger::ERROR_CannotReturnArray(this, child);
                            break;
                    }
                }
                break;
            case NT::ID_ARRAY:
                cout << "RETURN ID_ARRAY" << endl;
                break;
            case NT::CALL:
                cout << "RETURN CALL" << endl;
                break;
            case NT::OPERATOR:
                //cout << "RETURN OPERATOR" << endl;
                break;
        }
    }
}

void SemanticAnalyzer::processAssignment(node::Node *asgn) {
    #if PENDANTIC_DEBUG
    cout << "[Process ASGN] ";
    #endif

    if (asgn->getChildren().size() == 2) {
        auto lhs = asgn->getChildren()[0];
        auto rhs = asgn->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) throw runtime_error("Error in processAssignment(): LHS or RHS is null.");

        lhs->setIsVisited(true); rhs->setIsVisited(true);

        // Right-hand side of the assignment
        node::Node *rhsDecl = nullptr;
        switch (rhs->getNodeType()) {
            case NT::ID: {
                /* Check if the ID is declared already */
                if (rhs->getDeclaration()) { 
                    rhsDecl = rhs->getDeclaration();
                    if (!rhsDecl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, rhs);
                        rhsDecl->setIsInitialized(true);
                    }
                }
               /* No declaration, requires a symbol look up */
                else if ( rhsDecl = lookupDeclaration(rhs) ) {
                    if (!rhsDecl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, rhs);
                        rhsDecl->setIsInitialized(true);
                    }
                }
                break;
            }
            case NT::OPERATOR: {
                processOperator(rhs);
                break;
            }
            default:
                break;
        }

        // Left-hand side of the assignment
        node::Node *lhsDecl = nullptr;
        switch (lhs->getNodeType()) {
            case NT::ID: {
                /* Check if the ID is declared already */
                if (lhsDecl = lhs->getDeclaration()) { 
                    lhsDecl->setIsInitialized(true);
                }
               /* No declaration, requires a symbol look up */
                else if (lhsDecl = lookupDeclaration(lhs)) {
                    lhsDecl->setIsInitialized(true);
                }
                break;
            }
            default:
                break;
        }

        if (lhsDecl) {
            if (lhsDecl->getNodeType() == NT::FUNCTION) {
                logger::ERROR_VariableAsFunction(this, lhs);
            }
        }

    }
}

node::Node* SemanticAnalyzer::lookupDeclaration(node::Node* id) {
    if (id == nullptr) throw runtime_error("Error in getDeclaration(): 'id' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Get Declaration]:" << endl;
    #endif

    id->setIsVisited(true);

    node::Node *decl = nullptr;

    // (1) Search through all available scopes
    if (getScopeCount() > 1) {
        decl = lookupAllScopes(id);
    }

    // (2) Else search through the current (local) scope
    if (decl == nullptr) {
        decl = lookupLocalSymbol(id);
    }

    // (3) Else search through the global scope
    if (decl == nullptr) {
        decl = lookupGlobalSymbol(id);
    }

    // If no declaration was found in any scope, throw an error
    if (decl == nullptr) {
        logger::ERROR_VariableNotDeclared(this, id);
        return nullptr;
    }

    id->setDeclaration(decl);
    return decl;
}

void SemanticAnalyzer::processIdentifier(node::Node *id) {
    if (id == nullptr) throw runtime_error("Error in processIdentifier(): 'id' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process ";
    #endif

    id->setIsVisited(true);

    switch(id->getNodeType()) {
        case NT::ID_ARRAY: {
            #if PENDANTIC_DEBUG
            cout << "ID Array] ";
            #endif
            break;
        }
        case NT::ID: {
            #if PENDANTIC_DEBUG
            cout << "ID] ";
            #endif
            if( auto decl = id->getDeclaration() ) {
                if( !decl->getIsInitialized() ) {
                    logger::WARN_VariableNotInitialized(this, id);
                    decl->setIsInitialized(true);
                }
            }
            else if (auto decl = lookupDeclaration(id)) {
                if (!decl->getIsInitialized()) {
                    logger::WARN_VariableNotInitialized(this, id);
                    decl->setIsInitialized(true);
                }
            }
            break;
        }
        default:
            break;
    }
}

void SemanticAnalyzer::processOperator(node::Node *op) {
    #if PENDANTIC_DEBUG
    cout << "[Process Operator] ";
    #endif

    op->setIsVisited(true);

    if (op->getChildren().size() == 2) {
        auto lhs = op->getChildren()[0];
        auto rhs = op->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) throw runtime_error("Error in processOperator(): LHS or RHS is null.");

        lhs->setIsVisited(true); rhs->setIsVisited(true);

        // Left-hand side of the assignment
        node::Node *lhsDecl = nullptr;
        switch (lhs->getNodeType()) {
            case NT::ID: {
                /* Check if the ID is declared already */
                if (lhs->getDeclaration()) { 
                    lhsDecl = lhs->getDeclaration();
                    if (!lhsDecl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, lhs);
                        lhsDecl->setIsInitialized(true);
                    }
                }
               /* No declaration, requires a symbol look up */
                else if ( lhsDecl = lookupDeclaration(lhs) ) {
                    if (!lhsDecl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, lhs);
                        lhsDecl->setIsInitialized(true);
                    }
                }
                break;
            }
            default:
                break;
        }

        // Right-hand side of the assignment
        node::Node *rhsDecl = nullptr;
        switch (rhs->getNodeType()) {
            case NT::ID: {
                /* Check if the ID is declared already */
                if (rhs->getDeclaration()) { 
                    rhsDecl = rhs->getDeclaration();
                    if (!rhsDecl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, rhs);
                        rhsDecl->setIsInitialized(true);
                    }
                }
               /* No declaration, requires a symbol look up */
                else {
                    if ( rhsDecl = lookupDeclaration(rhs) ) {
                        if (!rhsDecl->getIsInitialized()) {
                            logger::WARN_VariableNotInitialized(this, rhs);
                            rhsDecl->setIsInitialized(true);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

void SemanticAnalyzer::processBinaryOperation(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in processBinaryOperation(): 'op' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Operation: Binary - " << types::literalNodeTypeStr(op->getNodeType()) << "] ";
    #endif

    op->setIsVisited(true);

    switch (op->getNodeType()) {
        case NT::OPERATOR: {
            switch (op->getOpType()) {
                /* EQUAL TYPES ONLY (LHS = RHS), ARRAYS */
                case OT::EQL:
                case OT::NEQ:
                case OT::LESS:
                case OT::LEQ:
                case OT::GREATER:
                case OT::GEQ: {
                    return;
                }
                /* INT TYPES ONLY (LHS and RHS), NO ARRAYS */
                case OT::ADD:
                case OT::SUB:
                case OT::MUL:
                case OT::DIV:
                case OT::MOD: {
                    processOperator(op);
                    return;
                }
            }
            return;
        }
        case NT::ASSIGNMENT: {
            switch (op->getAsgnType()) {
                /* ASGN, EQUAL TYPES, ARRAYS, OP = TYPE OF LHS */
                case AT::ASGN:
                    processAssignment(op);
                    return;
                /* INT TYPES ONLY, NO ARRAYS */
                case AT::ADDASGN:
                case AT::SUBASGN:
                case AT::MULASGN:
                case AT::DIVASGN: {
                    return;
                }
            }
            return;
        }
        /* BOOL TYPES ONLY, NO ARRAYS */
        case NT::AND:
        case NT::OR: {
            return;
        }
        default:
            return;
    }
}

void SemanticAnalyzer::processUnaryOperation(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in processUnaryOperation(): 'op' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Operation: Unary - " << types::literalNodeTypeStr(op->getNodeType()) << "] ";
    #endif

    op->setIsVisited(true);

    auto operand = op->getChildren()[0];

    if (operand == nullptr) throw runtime_error("Error in processUnaryOperation(): 'operand' is null.");

    switch(op->getNodeType()) {
        /* ARRAY OPERAND ONLY */
        case NT::SIZEOF_UNARY:
            return;
        /* INT TYPES ONLY, NO ARRAY*/
        case NT::CHSIGN_UNARY:
        case NT::QUES_UNARY: {
            if ( auto decl = operand->getDeclaration()) {
                if( !decl->getIsInitialized() ) {
                    logger::WARN_VariableNotInitialized(this, operand);
                    decl->setIsInitialized(true);
                }
            }
            else {
                if ( auto decl = lookupDeclaration(operand) ) {
                    if (!decl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, operand);
                        decl->setIsInitialized(true);
                    }
                }
            }
            return;
        }
        /* INC/DEC, INT TYPES ONLY */
        case NT::ASSIGNMENT: {
            if ( auto decl = operand->getDeclaration()) {
                if( !decl->getIsInitialized() ) {
                    logger::WARN_VariableNotInitialized(this, operand);
                    decl->setIsInitialized(true);
                }
            }
            else {
                if ( auto decl = lookupDeclaration(operand) ) {
                    if (!decl->getIsInitialized()) {
                        logger::WARN_VariableNotInitialized(this, operand);
                        decl->setIsInitialized(true);
                    }
                }
            }
            return;
        }
        default:
            return;
    }
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
            return;
        default:
            break;
    }

    // STATEMENTS
    if( !node->getIsVisited() ) {
        #if PENDANTIC_DEBUG
        cout << "(" << node->getLine() << ") ";
        #endif
        switch (node->getNodeType()) {

            /* DECLARATIONS */ 
            case NT::VARIABLE:
            case NT::VARIABLE_STATIC:
            case NT::PARAMETER:
                insertLocalSymbol(node);
                return;

            /* DECLARATIONS (ARRAYS) */
            case NT::VARIABLE_ARRAY:
            case NT::VARIABLE_STATIC_ARRAY:
            case NT::PARAMETER_ARRAY:
                insertLocalSymbol(node);
                return;

            /* IDENTIFIERS/VARIABLES */
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(node);
                return;

            /* ASSIGNMENTS */
            case NT::ASSIGNMENT: {
                switch (node->getAsgnType()) {
                    case AT::ASGN:
                    case AT::ADDASGN:
                    case AT::SUBASGN:
                    case AT::MULASGN:
                    case AT::DIVASGN:
                        processBinaryOperation(node);
                        return;
                    case AT::INC:
                    case AT::DEC:
                        processUnaryOperation(node);
                        return;
                    default:
                        return;
                }
                return;
            }

            /* OPERATORS/AND/OR */
            case NT::OPERATOR:
            case NT::AND:
            case NT::OR: {
                processBinaryOperation(node);
                return;
            }

            /* UNARY OPERATIONS */
            case NT::NOT:
            case NT::SIZEOF_UNARY:
            case NT::CHSIGN_UNARY:
            case NT::QUES_UNARY: {
                processUnaryOperation(node);
                return;
            }


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