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
            cout << "Symbol Found: \"" << sym->getString() << "\" ("; 
            cout << types::literalNodeTypeStr(item.second->getNodeType()) << ")" << endl;
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

#pragma region ScopeManagement

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

#pragma endregion ScopeManagement

#pragma region SymbolInsert

/***********************************************
*  SYMBOL INSERTION
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

#pragma endregion SymbolInsert

#pragma region SymbolLookup

/***********************************************
*  SYMBOL LOOKUP
***********************************************/

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
        /*         
        // Do not search through function scopes for duplicate variables
        if ((*it)->getName().find("FUNCTION_") != string::npos) {
            continue;
        } 
        */
        if (auto decl = (*it)->lookupSymbol(sym)) {
            return decl; // Return if symbol found
        }
    }
    return nullptr; // Return null if symbol not found
}

#pragma endregion SymbolLookup

/***********************************************
*  SEMANTIC ANALYSIS FUNCTIONS
***********************************************/

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

void SemanticAnalyzer::checkForInitializer(node::Node *var) {
    if (var->getChildren().size() == 1) {
        #if PENDANTIC_DEBUG
        cout << "[Has Initializer]" << endl;
        #endif

        auto init = var->getChildren()[0];

        if (init == nullptr) throw runtime_error("Error in checkForInitializer: 'init' is nullptr!");

        init->setIsVisited(true);

        // WIP...
    }
}

node::Node* SemanticAnalyzer::checkInitialization(node::Node *id) {
    /* (1) Check if the ID is declared already */
    if (id->getDeclaration()) { 
        auto decl = id->getDeclaration();
        if (!decl->getIsInitialized()) {
            logger::WARN_VariableNotInitialized(this, id);
            decl->setIsInitialized(true);
        }
        return decl;
    }
    /* (2) No declaration, requires a symbol look up */
    else if ( auto decl = lookupDeclaration(id) ) {
        if (!decl->getIsInitialized()) {
            logger::WARN_VariableNotInitialized(this, id);
            decl->setIsInitialized(true);
        }
        return decl;
    }
    return nullptr;
}

node::Node* SemanticAnalyzer::applyInitialization(node::Node *id) {
    /* (1) Check if the ID is declared already */
    if (id->getDeclaration()) { 
        auto decl = id->getDeclaration();
        if (!decl->getIsInitialized()) {
            decl->setIsInitialized(true);
        }
        return decl;
    }
    /* (2) No declaration, requires a symbol look up */
    else if ( auto decl = lookupDeclaration(id) ) {
        if (!decl->getIsInitialized()) {
            decl->setIsInitialized(true);
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

void SemanticAnalyzer::processReturn(node::Node *ret) {
    if (ret == nullptr) throw runtime_error("Error in processReturn(): 'ret' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process Return] ";
    #endif

    ret->setIsVisited(true);

    // Set type to LHS
}

node::Node* SemanticAnalyzer::processArray(node::Node* arr, bool isLHSinASGN) {
    if (arr == nullptr) throw runtime_error("Error in processArray(): 'arr' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process Array]" << endl;
    #endif

    arr->setIsVisited(true);

    if (arr->getChildren().size() == 2) {

        auto id = arr->getChildren()[0];
        auto index = arr->getChildren()[1];

        if (id == nullptr || index == nullptr) throw runtime_error("Error in processArray(): 'id' or 'index' is null.");

        id->setIsVisited(true); index->setIsVisited(true);

        /**
         *  Processing Child 0: ID
         */

        // (1) Process Child 0 (the ID)
        auto idDecl = lookupDeclaration(id);

        if (idDecl) { 
            // (2) Set the ID_ARRAY type to the ID if found
            arr->setVarType(idDecl->getVarType());  

            // (3) Error if the array declaration isn't actually an array
            if (idDecl->getNodeType() != NT::VARIABLE_ARRAY && 
                idDecl->getNodeType() != NT::PARAMETER_ARRAY && 
                idDecl->getNodeType() != NT::VARIABLE_STATIC_ARRAY) 
            {
                logger::ERROR_CannotIndexNonArray(this, id);
            }

            // (4) If it's the LHS of an ASGN then set initialized.
            if (isLHSinASGN) idDecl->setIsInitialized(true);
            else checkInitialization(id);
        }

        /**
         *  Processing Child 1: Index
         */

        node::Node *indexDecl = nullptr;
        switch (index->getNodeType()) {
            case NT::ID: {
                if (isLHSinASGN) {
                    indexDecl = applyInitialization(index);
                } 
                else {
                    indexDecl = checkInitialization(index);
                }
                if (indexDecl->getNodeType() == NT::VARIABLE_ARRAY || 
                    indexDecl->getNodeType() == NT::PARAMETER_ARRAY || 
                    indexDecl->getNodeType() == NT::VARIABLE_STATIC_ARRAY) 
                {
                    logger::ERROR_ArrayIndexIsUnindexedArray(this, index);
                }
                break;
            }
            case NT::ID_ARRAY: {
                indexDecl = processArray(index, isLHSinASGN);
                break;
            }
            case NT::CALL: {
                processCall(index);
                break;
            }
            case NT::OPERATOR: {
                processOperator(index, isLHSinASGN, false); // Might need to return the node here...
                break;
            }
            case NT::ASSIGNMENT: {
                switch(index->getAsgnType()) {
                    case AT::ASGN:
                        processAssignment(index);
                        break;
                    case AT::INC:
                    case AT::DEC:
                        processUnaryOperation(index);
                        break;
                    default:
                        break;
                }
            }
        }

        if (indexDecl) {
            if (indexDecl->getVarType() != VT::INT) {
                logger::ERROR_ArrayIndexNotInt(this, arr, index);
            }
        } 
        // WIP? If its not a variable then likely a CONSTANT to check type here...
        else {
            if (index->getVarType() != VT::INT) {
                logger::ERROR_ArrayIndexNotInt(this, arr, index);
            }
        }

        // Last: return the ID declaration
        return idDecl;
    }

    return nullptr;
}

node::Node* SemanticAnalyzer::processCall(node::Node* call) {
    if (call == nullptr) throw runtime_error("Error in processCall(): 'call' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process Call] ";
    #endif

    call->setIsVisited(true);

    node::Node* decl = lookupDeclaration(call);

    // (1) Process the declaration of the function if found
    if (decl) {
        if (decl->getNodeType() != NT::FUNCTION) {
            logger::ERROR_CannotCallSimpleVariable(this, call);
        }
    }

    // (2) Process the arguments of the function
    if (call->getChildren().size() > 0) {
        for(auto child: call->getChildren()) {
            if(child != nullptr) {
                child->setIsVisited(true);
                switch(child->getNodeType()) {
                    case NT::ID: {
                        checkInitialization(child);
                        break;
                    }
                    case NT::ID_ARRAY: {
                        processArray(child, false);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    return decl;
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

        // Left-hand side of the assignment
        node::Node *lhsDecl = nullptr;
        switch (lhs->getNodeType()) {
            case NT::ID: {
                lhsDecl = applyInitialization(lhs);
                break;
            }
            case NT::ID_ARRAY: {
                lhsDecl = processArray(lhs, true);
                break;
            }
            default:
                break;
        }

        // Right-hand side of the assignment
        node::Node *rhsDecl = nullptr;
        switch (rhs->getNodeType()) {
            case NT::ID: {
                rhsDecl = checkInitialization(rhs);
                break;
            }
            case NT::ID_ARRAY: {
                rhsDecl = processArray(rhs, false);
                break;
            }
            case NT::OPERATOR: {
                processOperator(rhs, false, false);
                break;
            }
            case NT::ASSIGNMENT: {
                switch(rhs->getAsgnType()) {
                    case AT::ASGN:
                        processAssignment(rhs);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }

        if (lhsDecl) {
            // Set the type of the assignment to the type of the LHS as per the semantics for ASGN
            asgn->setVarType(lhsDecl->getVarType());

            // Check if the LHS is a function
            if (lhsDecl->getNodeType() == NT::FUNCTION) {
                logger::ERROR_VariableAsFunction(this, lhs);
            }

            // Check if the LHS and RHS are of the same type
            if (rhsDecl) {
                if (lhsDecl->getVarType() != rhsDecl->getVarType()) {
                    logger::ERROR_RequiresOperandsEqualTypes(this, asgn, lhs, rhs);
                }
            } else {
                if (lhsDecl->getVarType() != rhs->getVarType()) {
                    logger::ERROR_RequiresOperandsEqualTypes(this, asgn, lhs, rhs);
                }
            }
        }
        // Default to this if no LHS declaration was found?
        else {
            asgn->setVarType(lhs->getVarType());
        }
    }
}

node::Node* SemanticAnalyzer::processIdentifier(node::Node *id) {
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
            return processArray(id, false);
            break;
        }
        case NT::ID: {
            #if PENDANTIC_DEBUG
            cout << "ID] ";
            #endif
            return checkInitialization(id);
        }
        default:
            break;
    }
    return nullptr;
}

node::Node* SemanticAnalyzer::processOperator(node::Node *op, bool isLHSinASGN, bool useArray) {
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
                if (isLHSinASGN) {
                    lhsDecl = applyInitialization(lhs);
                } 
                else
                {
                    lhsDecl = checkInitialization(lhs);
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
                if (isLHSinASGN) {
                    rhsDecl = applyInitialization(rhs);
                } 
                else
                {
                    rhsDecl = checkInitialization(rhs);
                }
                break;
            }
            default:
                break;
        }

        if (lhsDecl) {            
            // Check if the LHS is a function
            if (lhsDecl->getNodeType() == NT::FUNCTION) {
                logger::ERROR_VariableAsFunction(this, lhs);
            }
        }

        if (rhsDecl) {
            // Check if the LHS is a function
            if (rhsDecl->getNodeType() == NT::FUNCTION) {
                logger::ERROR_VariableAsFunction(this, rhs);
            }
        }

    }

    return op;
}

void SemanticAnalyzer::evaluateOPERATION(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in evaluateOPERATION(): 'op' is null.");   

    #if PENDANTIC_DEBUG
    cout << "[OPERATION: " << logger::loggerNodeTypeToStr(op) << "] ";
    #endif

    op->setIsVisited(true);

    switch (op->getNodeType()) {
        case NT::NOT:
        case NT::SIZEOF_UNARY:
        case NT::CHSIGN_UNARY:
        case NT::QUES_UNARY:
            processUnaryOperation(op);
            return;
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
                case AT::INC:
                case AT::DEC: {
                    processUnaryOperation(op);
                    return;
                }
            }
            return;
        }
        case NT::OPERATOR: {
            switch (op->getOpType()) {
                /* EQUAL TYPES ONLY (LHS = RHS), ARRAYS */
                case OT::EQL:
                case OT::NEQ:
                case OT::LESS:
                case OT::LEQ:
                case OT::GREATER:
                case OT::GEQ: {
                    processOperator(op, false, true);
                    return;
                }
                /* INT TYPES ONLY (LHS and RHS), NO ARRAYS */
                case OT::ADD:
                case OT::SUB:
                case OT::MUL:
                case OT::DIV:
                case OT::MOD: {
                    processOperator(op, false, false);
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
        /* BOOL ONLY, NO ARRAY */
        case NT::NOT:
            return;
        /* ARRAY OPERAND ONLY */
        case NT::SIZEOF_UNARY:
            switch (operand->getNodeType()) {
                case NT::ID:
                case NT::ID_ARRAY:
                    auto decl = processIdentifier(operand);
                    if (decl) op->setVarType(decl->getVarType());
                    return;
            }
            return;
        /* INT TYPES ONLY, NO ARRAY*/
        case NT::CHSIGN_UNARY:
        case NT::QUES_UNARY: {
            switch (operand->getNodeType()) {
                case NT::ID:
                case NT::ID_ARRAY:
                    processIdentifier(operand);
                    return;
            }
            return;
        }
        /* INC/DEC, INT TYPES ONLY */
        case NT::ASSIGNMENT: {
            switch(op->getAsgnType()) {
                case AT::INC:
                case AT::DEC:
                    switch(operand->getNodeType()) {
                        case NT::ID:
                        case NT::ID_ARRAY:
                            auto decl = processIdentifier(operand);
                            if (decl->getNodeType() == NT::FUNCTION) {
                                logger::ERROR_VariableAsFunction(this, operand);
                            }
                            return;
                    }
                    return;
            }
            return;
        }
        default:
            return;
    }
}

void SemanticAnalyzer::processIf(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in processIf(): 'op' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process If] ";
    #endif

    op->setIsVisited(true);

    // IF simpleExpression THEN matchedStatements ELSE matchedStatements

    // (1) Process the simple expression/conditional
    auto cnd = op->getChildren()[0];
    if (cnd) {

        cnd->setIsVisited(true);

        #if PENDANTIC_DEBUG
        cout << "[Condition]: ";
        #endif

        switch (cnd->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(cnd);
                break;
            default:
                evaluateOPERATION(cnd);
                break;
        }
    }

    // (2) Process the 'then' block
    auto thenStmt = op->getChildren()[1];
    if (thenStmt) {

        thenStmt->setIsVisited(true);

        #if PENDANTIC_DEBUG
        cout << "[Then]: ";
        #endif

        switch (thenStmt->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(thenStmt);
                break;
            case NT::IF:
                processIf(thenStmt);
                break;
            case NT::WHILE:
                processWhile(thenStmt);
                break;
            default:
                evaluateOPERATION(thenStmt);
                break;
        }
    }

    // (3) Process the 'else' block
    auto elseStmt = op->getChildren()[2];
    if (elseStmt) {

        elseStmt->setIsVisited(true);

        #if PENDANTIC_DEBUG
        cout << "[Else]: ";
        #endif

        switch (elseStmt->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(elseStmt);
                break;
            case NT::IF:
                processIf(elseStmt);
                break;
            case NT::WHILE:
                processWhile(elseStmt);
                break;
            default:
                evaluateOPERATION(elseStmt);
                break;
        }
    }
}

void SemanticAnalyzer::processWhile(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in processWhile(): 'op' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process While] ";
    #endif

    op->setIsVisited(true);

    // WHILE simpleExpression DO matchedStatements

    auto cnd = op->getChildren()[0];
    if (cnd) {

        cnd->setIsVisited(true);

        #if PENDANTIC_DEBUG
        cout << "[Condition]: ";
        #endif

        switch (cnd->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(cnd);
                break;
            default:
                evaluateOPERATION(cnd);
                break;
        }
    }

    // (2) Process the 'do' block
    auto stmt = op->getChildren()[1];
    if (stmt) {
        
        stmt->setIsVisited(true);

        #if PENDANTIC_DEBUG
        cout << "[Do]: ";
        #endif

        switch (stmt->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(stmt);
                break;
            case NT::IF:
                processIf(stmt);
                break;
            case NT::WHILE:
                processWhile(stmt);
                break;
            default:
                evaluateOPERATION(stmt);
                break;
        }
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
        cout << endl;
        cout << "(" << node->getLine() << ") ";
        #endif
        switch (node->getNodeType()) {
            
            /* IF */
            case NT::IF:
                processIf(node);
                return;

            /* WHILE */
            case NT::WHILE:
                processWhile(node);
                return;

            /* DECLARATIONS */ 
            case NT::VARIABLE:
            case NT::VARIABLE_STATIC:
            case NT::PARAMETER:
                insertLocalSymbol(node);
                checkForInitializer(node);
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

            /* CALL */
            case NT::CALL:
                processCall(node);
                return;

            /* ASSIGNMENTS */
            case NT::ASSIGNMENT:
            case NT::OPERATOR:
            case NT::AND:
            case NT::OR:
            case NT::NOT:     
            case NT::SIZEOF_UNARY:
            case NT::CHSIGN_UNARY:
            case NT::QUES_UNARY: {
                evaluateOPERATION(node);
            }

            /* RETURN */
            case NT::RETURN:
                processReturn(node);
                return;

            default:
                return;
        }

    }
}

void SemanticAnalyzer::traverseGlobals(node::Node *node) {
    if (node == nullptr) { return; }

    switch (node->getNodeType()) {
        case NT::VARIABLE:
        case NT::VARIABLE_STATIC:
            #if PENDANTIC_DEBUG
            cout << endl;
            #endif
            insertGlobalSymbol(node);
            node->setIsVisited(true);
            node->setIsInitialized(true);
            checkForInitializer(node);
            break;
        case NT::FUNCTION:
        case NT::VARIABLE_ARRAY:
        case NT::VARIABLE_STATIC_ARRAY:
            #if PENDANTIC_DEBUG
            cout << endl;
            #endif
            insertGlobalSymbol(node);
            node->setIsVisited(true);
            node->setIsInitialized(true);
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

    #if PENDANTIC_DEBUG
    cout << string(SPACE + 2, '*') << endl;
    cout << "(0) START: GLOBAL SCOPE" << endl;
    cout << string(SPACE + 2, '*') << endl;
    #endif

    // (1) Traverse Globals
    traverseGlobals(tree_);
    
    #if PENDANTIC_DEBUG
    if (getGlobalScope()->getTable()->getSize() > 0) cout << endl;
    printGlobal();
    #endif

    // (2) Traverse Locals
    traverseLocals(tree_);

    // (3) Check Linker
    checkLinker();
}

#pragma endregion Traversal

}