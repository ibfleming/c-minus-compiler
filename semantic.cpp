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
        cout << "\tInserted symbol \"" << name << "\"." << endl;
        #endif
        return true;
    }
    #if PENDANTIC_DEBUG
    cout << "\tSymbol already exists \"" << name << "\"." << endl;
    #endif
    return false;
}

node::Node* SymbolTable::lookupSymbol(const node::Node* sym) {
    for (auto const& item : symbols_) {
        if ( item.first == sym->getString() ) {
            #if PENDANTIC_DEBUG
            cout << "\tSymbol Found: \"" << sym->getString() << "\" ("; 
            cout << types::literalNodeTypeStr(item.second->getNodeType()) << ")" << endl;
            #endif
            return item.second;
        }
    }
    #if PENDANTIC_DEBUG
    cout << "\tSymbol \"" << sym->getString() << "\" not found!" << endl;
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

void SemanticAnalyzer::enterGlobalScope() {
    scopes_.push(globalScope_);
    #if PENDANTIC_DEBUG
    cout << string(SPACE + 2, '*') << endl;
    cout << "(0) ENTERING SCOPE: GLOBAL" << endl;
    cout << string(SPACE + 2, '*') << endl;
    #endif
}

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
    if (scope != nullptr) {
        if( scope == globalScope_ ) {
            #if PENDANTIC_DEBUG
            cout << endl;
            cout << string(SPACE + 2, '#') << endl;
            cout << "(0) LEAVING SCOPE: GLOBAL" << endl;
            scope->printScope();
            cout << string(SPACE + 2, '#') << endl;
            #endif
            scopes_.pop();
            return;
        }
        #if PENDANTIC_DEBUG
        cout << endl;
        cout << string(SPACE + 2, '#') << endl;
        cout << "(" << scope->getParent()->getLine() << ") LEAVING SCOPE: " << scope->getName() << endl;
        scope->printScope();
        cout << string(SPACE + 2, '#') << endl;
        #endif
        scopes_.pop();
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

/***********************************************
*  SYMBOL TABLE MANAGEMENT
***********************************************/

bool SemanticAnalyzer::insertSymbol(node::Node *sym) {
    if (sym == nullptr) throw runtime_error("Error in insertSymbol(): 'sym' is null.");

    #if PENDANTIC_DEBUG
    cout << "(INSERT SYMBOL)" << endl;
    #endif

    sym->setIsVisited(true);

    if (getCurrentScope() == globalScope_) { // In global scope.
        #if PENDANTIC_DEBUG
        cout << "\t[Global Scope]:" << endl;
        #endif
        sym->setIsUsed(true);
        sym->setIsInitialized(true);
        if (auto decl = getGlobalScope()->lookupSymbol(sym)) {
            logger::ERROR_VariableAlreadyDeclared(this, sym, decl);
            return false;
        }
    }
    else if (getCurrentScope() != globalScope_) { // Not in global scope.
        auto scopes = utils::stackToVectorReverse(scopes_);
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if ((*it) == globalScope_ ) continue;   // Skip global scope
            #if PENDANTIC_DEBUG
            cout << ((*it) == getCurrentScope() ? "\t[Current Scope]:" : "\t[Other Scope]:") << endl;
            #endif
            if (auto decl = (*it)->lookupSymbol(sym)) {
                logger::ERROR_VariableAlreadyDeclared(this, sym, decl);
                return false;
            } else {
                return (*it)->insertSymbol(sym);
            }
        }
    }
    return getCurrentScope()->insertSymbol(sym);
}

node::Node* SemanticAnalyzer::lookupSymbol(node::Node* id, bool isLHSinASGN) {
    if (id == nullptr) throw runtime_error("Error in lookupSymbol(): 'id' is null.");

    #if PENDANTIC_DEBUG
    cout << "(LOOKUP DECLARATION)" << endl;
    #endif

    node::Node* decl = nullptr;

    if (getCurrentScope() == globalScope_) { // In global scope.
        #if PENDANTIC_DEBUG
        cout << "\t[Global Scope]:" << endl;
        #endif
        if (decl = id->getDeclaration()) {
            #if PENDANTIC_DEBUG
            cout << "\tID has been declared." << endl;
            #endif
            return decl;
        }
        else if (decl = getGlobalScope()->lookupSymbol(id)) {
            id->setDeclaration(decl);
            return decl;
        }
    }
    else if (getCurrentScope() != globalScope_) { // Not in global scope.
        if (decl = id->getDeclaration()) {
            #if PENDANTIC_DEBUG
            cout << "\tID has been declared." << endl;
            #endif
            return decl;
        }
        else {
            std::vector<semantic::Scope *, std::allocator<semantic::Scope *>> scopes;
            if (isLHSinASGN) scopes = utils::stackToVectorInOrder(scopes_);
            else scopes = utils::stackToVectorReverse(scopes_);
            for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
                #if PENDANTIC_DEBUG
                if (*it == getGlobalScope()) {
                    cout << "\t[Global Scope]:" << endl;
                } else if (*it == getCurrentScope()) {
                    cout << "\t[Current Scope]:" << endl;
                } else {
                    cout << "\t[Other Scope]:" << endl;
                }
                #endif
                if (decl = (*it)->lookupSymbol(id)) {
                    id->setDeclaration(decl);
                    return decl;
                }
            }
        }
    }
    logger::ERROR_VariableNotDeclared(this, id);
    return nullptr;
}

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
        #if PENDANTIC_DEBUG
        cout << "[Check Init] Found: " << id->getDeclaration()->getString() << " (" << types::literalNodeTypeStr(id->getDeclaration()->getNodeType()) << ")" << endl;
        #endif
        auto decl = id->getDeclaration();
        if (!decl->getIsInitialized()) {
            logger::WARN_VariableNotInitialized(this, id);
            decl->setIsInitialized(true);
        }
        return decl;
    }
    /* (2) No declaration, requires a symbol look up */
    else if ( auto decl = lookupSymbol(id, false) ) {
        if (!decl->getIsInitialized()) {
            logger::WARN_VariableNotInitialized(this, id);
            decl->setIsInitialized(true);
        }
        return decl;
    }
    return nullptr;
}

node::Node* SemanticAnalyzer::applyInitialization(node::Node *id, bool isLHSinASGN) {
    /* (1) Check if the ID is declared already */
    if (id->getDeclaration()) { 
        #if PENDANTIC_DEBUG
        cout << "[Apply Init] Found: " << id->getDeclaration()->getString() << " (" << types::literalNodeTypeStr(id->getDeclaration()->getNodeType()) << ")" << endl;
        #endif
        auto decl = id->getDeclaration();
        if (!decl->getIsInitialized()) {
            decl->setIsInitialized(true);
        }
        return decl;
    }
    /* (2) No declaration, requires a symbol look up */
    else if ( auto decl = lookupSymbol(id, isLHSinASGN) ) {
        if (!decl->getIsInitialized()) {
            decl->setIsInitialized(true);
        }
        return decl;
    }
    return nullptr;
}

void SemanticAnalyzer::checkBinaryTypes(node::Node *op, node::Node *lhs, node::Node *rhs) {
    #if PENDANTIC_DEBUG
    cout << "[Check Binary Types]" << endl;
    #endif

    if (op == nullptr) throw runtime_error("Error in checkBinaryTypes(): 'op' is null.");

    auto lhsDecl = lhs->getDeclaration() != nullptr ? lhs->getDeclaration() : lhs;
    auto rhsDecl = rhs->getDeclaration() != nullptr ? rhs->getDeclaration() : rhs;

    // Check the node type of the operator to determine the type and semantic checking
    switch (op->getNodeType()) {
        case NT::ASSIGNMENT: {
            switch (op->getAsgnType()) {

                /* EQUAL TYPES AND ARRAYS */
                case AT::ASGN: {
                    if (lhsDecl && rhsDecl) {
                        if (lhsDecl->getIsArray() != rhsDecl->getIsArray()) {
                            logger::ERROR_RequiresOperandsAsArrayTypes(this, op, lhsDecl, rhsDecl);
                        }
                        if (lhsDecl->getVarType() != rhsDecl->getVarType()) {
                            logger::ERROR_RequiresOperandsEqualTypes(this, op, lhs, rhs);
                        }
                        if (lhsDecl->getNodeType() == NT::FUNCTION) { logger::ERROR_VariableAsFunction(this, lhs); }
                    }
                   return;
                }
                /* INT AND NON-ARRAY ONLY */
                case AT::ADDASGN:
                case AT::SUBASGN:
                case AT::MULASGN:
                case AT::DIVASGN: {
                    if (lhsDecl) {
                        if (lhsDecl->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, lhs, logger::OperandType::LHS);
                        }
                    }
                    else {
                        if (lhs->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, lhs, logger::OperandType::LHS);
                        }
                    }
                    if (rhsDecl) {
                        if (rhsDecl->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, rhs, logger::OperandType::RHS);
                        }
                    }
                    else {
                        if (rhs->getVarType() != VT::INT) {
                            logger::ERROR_RequiresOperandIntTypes(this, op, rhs, logger::OperandType::RHS);
                        }
                    }
                    return;
                }
                return;
            }
            return;
        }
        case NT::OPERATOR: {
            switch (op->getOpType()) {

                /* EQUAL TYPES AND ARRAYS */
                case OT::EQL:
                case OT::NEQ:
                case OT::LESS:
                case OT::LEQ:
                case OT::GREATER:
                case OT::GEQ: {
                    if (lhsDecl && rhsDecl) {
                        if (lhsDecl->getVarType() != rhsDecl->getVarType()) {
                            logger::ERROR_RequiresOperandsEqualTypes(this, op, lhs, rhs);
                        }
                        if (lhsDecl->getIsArray() != rhsDecl->getIsArray()) {
                            logger::ERROR_RequiresOperandsAsArrayTypes(this, op, lhsDecl, rhsDecl);
                        }
                    }
                    return;
                }

                /* INT AND NON-ARRAY ONLY */
                case OT::ADD:
                case OT::SUB:
                case OT::MUL:
                case OT::DIV:
                case OT::MOD: {
                    if (lhsDecl || rhsDecl) {
                        if (lhsDecl) 
                        {
                            if (lhsDecl->getVarType() != VT::INT) logger::ERROR_RequiresOperandIntTypes(this, op, lhs, logger::OperandType::LHS);
                        }
                        if (rhsDecl)
                        {
                            if (rhsDecl->getVarType() != VT::INT) logger::ERROR_RequiresOperandIntTypes(this, op, rhs, logger::OperandType::RHS); 
                        }
                        if (lhsDecl) 
                        {
                            if (lhsDecl->getIsArray()) { logger::ERROR_OperationCannotUseArrays(this, op, lhs); return; }
                        }
                        if (rhsDecl)
                        {
                            if (rhsDecl->getIsArray()) { logger::ERROR_OperationCannotUseArrays(this, op, rhs); return; } 
                        }                     
                    }
                    return;
                }
                return;
            }
            return;
        }

        /* BOOL AND NON-ARRAY ONLY */
        case NT::AND:
        case NT::OR: {
            if (lhsDecl || rhsDecl) {
                if (lhsDecl) 
                {
                    if (lhsDecl->getIsArray()) { logger::ERROR_OperationCannotUseArrays(this, op, lhs); return; }
                    if (lhsDecl->getVarType() != VT::BOOL) logger::ERROR_RequiresOperandBoolTypes(this, op, lhs, logger::OperandType::LHS);
                }
                if (rhsDecl)
                {
                    if (rhsDecl->getIsArray()) { logger::ERROR_OperationCannotUseArrays(this, op, rhs); return; }
                    if (rhsDecl->getVarType() != VT::BOOL) logger::ERROR_RequiresOperandBoolTypes(this, op, rhs, logger::OperandType::RHS);
                }
            }
            return;
        }
        default:
            return;
    }
}

void SemanticAnalyzer::checkUnaryTypes(node::Node *op, node::Node *operand) {
    #if PENDANTIC_DEBUG
    cout << "[Check Unary Types]" << endl;
    #endif

    if (op == nullptr) throw runtime_error("Error in checlUnaryTypes(): 'op' is null.");

    auto operandDecl = operand->getDeclaration() != nullptr ? operand->getDeclaration() : operand;

    switch(op->getNodeType()) {
        case NT::QUES_UNARY: {
            if (operandDecl->getVarType() != VT::INT) {
                logger::ERROR_UnaryRequiresOperandSameType(this, op, operand);
            }
            if (operandDecl->getIsArray()) {
                logger::ERROR_OperationCannotUseArrays(this, op, operand);
            }
            return;
        }
        case NT::SIZEOF_UNARY: {
            if (!operandDecl->getIsArray()) {
                logger::ERROR_OperationWorksOnlyOnArrays(this, op, operand);
            }
            return;
        }
        case NT::CHSIGN_UNARY: {
            if (operandDecl->getVarType() != VT::INT) {
                logger::ERROR_UnaryRequiresOperandSameType(this, op, operand);
            }
            if (operandDecl->getIsArray()) {
                logger::ERROR_OperationCannotUseArrays(this, op, operand);
            }
            return;
        }
        case NT::NOT: {
            if (operandDecl->getVarType() != VT::BOOL) {
                logger::ERROR_UnaryRequiresOperandSameType(this, op, operand);
            }
            if (operandDecl->getIsArray()) {
                logger::ERROR_OperationCannotUseArrays(this, op, operand);
            }
            return;
        }
        case NT::ASSIGNMENT: {
            if (operandDecl->getIsArray()) {
                logger::ERROR_OperationCannotUseArrays(this, op, operand);
            }
            return;
        }
        default:
            return;
    }
}

void SemanticAnalyzer::processReturn(node::Node *ret) {
    if (ret == nullptr) throw runtime_error("Error in processReturn(): 'ret' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process Return] ";
    #endif

    ret->setIsVisited(true);

    if (ret->getChildren().size() == 1) {
        auto child = ret->getChildren()[0];

        if (child == nullptr) throw runtime_error("Error in processReturn(): 'child' is null.");

        child->setIsVisited(true);

        switch (child->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY: {
                auto decl = processIdentifier(child, false);
                if (decl) {
                    if (decl->getIsArray()) {
                        logger::ERROR_CannotReturnArray(this, child);
                    }
                }
                break;
            }
            case NT::OPERATOR:
                processOperator(child, false);
                break;
            default:
                break;
        }
    }
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
        auto idDecl = lookupSymbol(id, false);

        if (idDecl) { 
            // (2) Set the ID_ARRAY type to the ID if found
            arr->setVarType(idDecl->getVarType());  

            // (3) Error if the array declaration is a function
            if (idDecl->getNodeType() == NT::FUNCTION) {
                logger::ERROR_VariableAsFunction(this, id);
            }

            // (4) If it's the LHS of an ASGN then set initialized.
            if (isLHSinASGN) idDecl->setIsInitialized(true);
            else checkInitialization(id);

            // (5) Error if the array declaration isn't actually an array
            if (!idDecl->getIsArray())
            {
                logger::ERROR_CannotIndexNonArray(this, id);
            }
        }
        // (6) Array Declarataion is not found so there error to this
        else {
            logger::ERROR_CannotIndexNonArray(this, id);
        }

        /**
         *  Processing Child 1: Index
         */

        node::Node *indexDecl = nullptr;
        switch (index->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY: {

                indexDecl = processIdentifier(index, isLHSinASGN);

                if (indexDecl) {
                    if (indexDecl->getVarType() != VT::INT) logger::ERROR_ArrayIndexNotInt(this, arr, index);

                    if (index->getNodeType() == NT::ID) {
                        if (indexDecl->getIsArray()) logger::ERROR_ArrayIndexIsUnindexedArray(this, index);
                    }

                }
                else {
                    if (index->getVarType() != VT::INT) logger::ERROR_ArrayIndexNotInt(this, arr, index);
                }
                break;
            }
            case NT::CALL: {
                processCall(index);
                break;
            }
            case NT::OPERATOR: {
                processOperator(index, isLHSinASGN); // Might need to return the node here...
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

        if(indexDecl == nullptr) {
            if (index->getVarType() != VT::INT) logger::ERROR_ArrayIndexNotInt(this, arr, index);
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

    node::Node* decl = lookupSymbol(call, false);

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
                        if (auto id = checkInitialization(child)) {
                            if (id->getNodeType() == NT::FUNCTION) {
                                logger::ERROR_VariableAsFunction(this, child);
                            }
                        }
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
    cout << "[Process ASGN]" << endl;
    #endif

    if (asgn->getChildren().size() == 2) {
        auto lhs = asgn->getChildren()[0];
        auto rhs = asgn->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) throw runtime_error("Error in processAssignment(): LHS or RHS is null.");

        lhs->setIsVisited(true); rhs->setIsVisited(true);

        // Left-hand side of the assignment
        node::Node *lhsDecl = nullptr;
        #if PENDANTIC_DEBUG
        cout << "{LHS}" << endl;
        #endif
        switch (lhs->getNodeType()) {
            case NT::ID: {
                if (lhsDecl = lookupSymbol(lhs, true)) {}
                break;
            }
            case NT::ID_ARRAY: {
                if (lhsDecl = processArray(lhs, true)) {}
                break;
            }
            default:
                break;
        }

        // Right-hand side of the assignment
        node::Node *rhsDecl = nullptr;
        #if PENDANTIC_DEBUG
        cout << "{RHS}" << endl;
        #endif
        switch (rhs->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY: {
                if (rhsDecl = processIdentifier(rhs, false)) {}                
                break;
            }
            case NT::OPERATOR: {
                processOperator(rhs, false);
                break;
            }
            case NT::ASSIGNMENT: {
                evaluateOperation(rhs);
                break;
            }
            case NT::CALL: {
                if (rhsDecl = processCall(rhs)) {}
                break;
            }
            case NT::STRING:
                #if PEDANTIC_DEBUG
                cout << "\tCONSTANT: String" << endl;
                #endif
                break;
            default:
                break;
        }

        if (lhsDecl == nullptr) {
            if (rhsDecl) lhs->setVarType(rhsDecl->getVarType());
        }

        // Set the type of the assignment to the type of the LHS as per the semantics for ASGN
        if(lhsDecl) { 
            asgn->setVarType(lhsDecl->getVarType());
            lhsDecl->setIsInitialized(true);
        }
        else asgn->setVarType(lhs->getVarType());

        checkBinaryTypes(asgn, lhs, rhs);
    }
}

node::Node* SemanticAnalyzer::processIdentifier(node::Node *id, bool isLHSinASGN) {
    if (id == nullptr) throw runtime_error("Error in processIdentifier(): 'id' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process ";
    #endif

    id->setIsVisited(true);

    switch(id->getNodeType()) {
        case NT::ID: {
            #if PENDANTIC_DEBUG
            cout << "ID - \"" << id->getString() << "\"] -> ";
            #endif
            if (isLHSinASGN) {
                return applyInitialization(id, isLHSinASGN);
            }
            else {
                return checkInitialization(id);
            }
            return nullptr;
        }
        case NT::ID_ARRAY: {
            #if PENDANTIC_DEBUG
            cout << "ID Array \"" << id->getString() << "\"] -> ";
            #endif
            return processArray(id, isLHSinASGN);
        }
        default:
            break;
    }

    return nullptr;
}

node::Node* SemanticAnalyzer::processOperator(node::Node *op, bool isLHSinASGN) {
    #if PENDANTIC_DEBUG
    cout << "[Process Operator]" << endl;
    #endif

    op->setIsVisited(true);

    if (op->getChildren().size() == 2) {
        auto lhs = op->getChildren()[0];
        auto rhs = op->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) throw runtime_error("Error in processOperator(): LHS or RHS is null.");

        lhs->setIsVisited(true); rhs->setIsVisited(true);

        // Left-hand side of the assignment
        node::Node *lhsDecl = nullptr;
        #if PENDANTIC_DEBUG
        cout << "{LHS}" << endl;
        #endif
        switch (lhs->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY: {
                if (lhsDecl = processIdentifier(lhs, isLHSinASGN)) {}
                break;
            }
            case NT::OPERATOR:
                processOperator(lhs, isLHSinASGN);
                break;
            case NT::CALL:
                lhsDecl = processCall(lhs);
                break;
            default:
                break;
        }

        // Right-hand side of the assignment
        node::Node *rhsDecl = nullptr;
        #if PENDANTIC_DEBUG
        cout << "{RHS}" << endl;
        #endif
        switch (rhs->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY: {
                if (rhsDecl = processIdentifier(rhs, false)) {}
                break;
            }
            case NT::OPERATOR:
                processOperator(rhs, false);
                break;
            case NT::CALL:
                rhsDecl = processCall(rhs);
                break;
            default:
                break;
        }

        if (lhsDecl) {            
            // Check if the LHS is a function
            if (lhs->getNodeType() != NT::CALL) {
                if (lhsDecl->getNodeType() == NT::FUNCTION) {
                    logger::ERROR_VariableAsFunction(this, lhs);
                    return op;
                }
            }
        }

        if (rhsDecl) {
            // Check if the RHS is a function
            if (rhs->getNodeType() != NT::CALL) {
                if (rhsDecl->getNodeType() == NT::FUNCTION) {
                    logger::ERROR_VariableAsFunction(this, rhs);
                    return op;
                }
            }
        }

        checkBinaryTypes(op, lhs, rhs);

    }

    return op;
}

void SemanticAnalyzer::evaluateOperation(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in evaluateOperation(): 'op' is null.");   

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
                case AT::ASGN:
                    processAssignment(op);
                    return;
                case AT::ADDASGN:
                case AT::SUBASGN:
                case AT::MULASGN:
                case AT::DIVASGN: {
                    processOperator(op, true);
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
                case OT::EQL:
                case OT::NEQ:
                case OT::LESS:
                case OT::LEQ:
                case OT::GREATER:
                case OT::GEQ: {
                    processOperator(op, false);
                    return;
                }
                case OT::ADD:
                case OT::SUB:
                case OT::MUL:
                case OT::DIV:
                case OT::MOD: {
                    processOperator(op, false);
                    return;
                }
            }
            return;
        }
        case NT::AND:
        case NT::OR: {
            processBooleanBinaryOperators(op);
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


    node::Node *operandDecl;
    switch (operand->getNodeType()) {
        case NT::CHSIGN_UNARY:
            processUnaryOperation(operand);
            return;
        default:
            if (operandDecl = processIdentifier(operand, false)) {}
            break;
    }

    checkUnaryTypes(op, operand);

    /* switch(op->getNodeType()) {

        case NT::NOT:
            if (decl) {
                if (decl->getVarType() != VT::BOOL) {
                    logger::ERROR_UnaryRequiresOperandSameType(this, op, operand);
                }
                if (decl->getIsArray()) {
                    logger::ERROR_OperationCannotUseArrays(this, op, operand);
                }
            }
            return;

        case NT::SIZEOF_UNARY: {
            if (decl) {
                if (!decl->getIsArray()) {
                    logger::ERROR_OperationWorksOnlyOnArrays(this, op, operand);
                }
            }
            return;
        }

        case NT::CHSIGN_UNARY:
        case NT::QUES_UNARY: {
            if (decl) {
                if (operand->getNodeType() == NT::ID ) {
                    if (decl->getIsArray()) {
                        logger::ERROR_OperationCannotUseArrays(this, op, operand);
                        return;
                    }
                }
                if (decl->getVarType() != VT::INT) {
                    logger::ERROR_UnaryRequiresOperandSameType(this, op, operand);
                }
            }
            else {
                switch (operand->getNodeType()) {
                    case NT::NUMBER:
                    case NT::STRING:
                    case NT::BOOLEAN:
                    case NT::CHARACTER:
                        if (operand->getVarType() != VT::INT) {
                            logger::ERROR_UnaryRequiresOperandSameType(this, op, operand);
                        }
                }
            }
            return;
        }
        
        case NT::ASSIGNMENT: {
            if (decl) {
                if (operand->getNodeType() == NT::ID ) {
                    if (decl->getIsArray()) {
                        logger::ERROR_OperationCannotUseArrays(this, op, operand);
                        return;
                    }
                }
                if (decl->getNodeType() == NT::FUNCTION) {
                    logger::ERROR_VariableAsFunction(this, operand);
                }
            }
            return;
        }
        default:
            return;
    } */
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
                processIdentifier(cnd, false);
                break;
            default:
                evaluateOperation(cnd);
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
                processIdentifier(thenStmt, false);
                break;
            case NT::IF:
                processIf(thenStmt);
                break;
            case NT::WHILE:
                processWhile(thenStmt);
                break;
            default:
                evaluateOperation(thenStmt);
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
                processIdentifier(elseStmt, false);
                break;
            case NT::IF:
                processIf(elseStmt);
                break;
            case NT::WHILE:
                processWhile(elseStmt);
                break;
            default:
                evaluateOperation(elseStmt);
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
                processIdentifier(cnd, false);
                break;
            default:
                evaluateOperation(cnd);
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
                processIdentifier(stmt, false);
                break;
            case NT::IF:
                processIf(stmt);
                break;
            case NT::WHILE:
                processWhile(stmt);
                break;
            default:
                evaluateOperation(stmt);
                break;
        }
    }
}

void SemanticAnalyzer::processBooleanBinaryOperators(node::Node *op) {
    if (op == nullptr) throw runtime_error("Error in processBooleanBinaryOperators(): 'op' is null.");

    #if PENDANTIC_DEBUG
    cout << "[Process Boolean Operation]" << endl;
    #endif

    op->setIsVisited(true);

    if (op->getChildren().size() == 2) {
        auto lhs = op->getChildren()[0];
        auto rhs = op->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) throw runtime_error("Error in processBooleanBinaryOperators(): LHS or RHS is null.");

        lhs->setIsVisited(true); rhs->setIsVisited(true);

        // Left-hand side of the assignment
        node::Node *lhsDecl = nullptr;
        switch (lhs->getNodeType()) {
            case NT::AND:
            case NT::OR: {
                processBooleanBinaryOperators(lhs);
                break;
            }
            case NT::OPERATOR: {
                processOperator(lhs, false);
                break;
            }
            default:
                lhsDecl = processIdentifier(lhs, false);
                break;
        }

        // Right-hand side of the assignment
        node::Node *rhsDecl = nullptr;
        switch (rhs->getNodeType()) {
           case NT::AND:
            case NT::OR: {
                processBooleanBinaryOperators(rhs);
                break;
            }
            case NT::OPERATOR: {
                processOperator(rhs, false);
                break;
            }
            default:
                rhsDecl = processIdentifier(rhs, false);
                break;
        }

        checkBinaryTypes(op, lhs, rhs);
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

void SemanticAnalyzer::analyzeNode(node::Node *node) {

    // SCOPES
    switch (node->getNodeType()) {
        case NT::FUNCTION:
            #if PENDANTIC_DEBUG
            cout << endl;
            cout << "(" << node->getLine() << ") ";
            #endif
            insertSymbol(node);
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
                insertSymbol(node);
                checkForInitializer(node);
                return;

            /* DECLARATIONS (ARRAYS) */
            case NT::VARIABLE_ARRAY:
            case NT::VARIABLE_STATIC_ARRAY:
            case NT::PARAMETER_ARRAY:
                insertSymbol(node);
                return;

            /* IDENTIFIERS/VARIABLES */
            case NT::ID:
            case NT::ID_ARRAY:
                processIdentifier(node, false);
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
                evaluateOperation(node);
                return;
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

void SemanticAnalyzer::traverse(node::Node *node) {
    if (node == nullptr) { return; }

    analyzeNode(node);

    // Traverse children nodes recursively first (inner blocks)
    for (node::Node *child : node->getChildren()) {
        traverse(child);
    }

    // After children, check sibling node (for next block at the same level)
    if (node->getSibling() != nullptr) {
        if (node->getNodeType() == NT::COMPOUND || node->getNodeType() == NT::FOR) {
            compoundLevel_--; leaveScope();
        }
        traverse(node->getSibling());
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
    // (1) Traverse the three
    traverse(tree_);

    // (2) Check Linker
    checkLinker();

    // (3) Leave Global Scope
    leaveScope();
}

#pragma endregion Traversal

}