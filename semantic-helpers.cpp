/*********************************************************************
 * @file semantic.cpp
 * 
 * @brief Source file for semantic analysis.
 *********************************************************************/

#include "semantic.hpp"

using namespace std;

namespace semantic {

void SemanticAnalyzer::analyzeNode(node::Node *node) {

    // Scopes
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

    // Statements/Variables/Operations
    if( !node->getIsVisited() ) {
        #if PENDANTIC_DEBUG
        cout << endl;
        cout << "(" << node->getLine() << ") ";
        #endif
        switch (node->getNodeType()) {
            
            // Variable Declarations
            case NT::VARIABLE:
            case NT::VARIABLE_STATIC:
            case NT::PARAMETER:
            case NT::VARIABLE_ARRAY:
            case NT::VARIABLE_STATIC_ARRAY:
            case NT::PARAMETER_ARRAY: {
                insertSymbol(node);
                checkForInitializer(node);
                return;
            }

            // Identifiers
            case NT::ID:
            case NT::ID_ARRAY:
            case NT::CALL:
                processIdentifier(node);
                return;

            // Binary Operations
            case NT::ASSIGNMENT: {
                processAssignment(node);
                return;
            }
            case NT::OPERATOR: {
                processBinaryOperator(node);
                return;
            }
            case NT::AND:
            case NT::OR: {
                processBooleanBinaryOperator(node);
                return;
            }

            // Unary Operations
            case NT::NOT:     
            case NT::SIZEOF_UNARY:
            case NT::CHSIGN_UNARY:
            case NT::QUES_UNARY: {
                processUnaryOperation(node);
                return;
            }

            // If
            case NT::IF:
                //processIf(node);
                return;

            // While
            case NT::WHILE:
                //processWhile(node);
                return;

            /* RETURN */
            case NT::RETURN:
                processReturn(node);
                return;

            default:
                return;
        }
    }
}

void SemanticAnalyzer::processAssignment(node::Node *node) {
    #if PENDANTIC_DEBUG
    cout << "[Process Assign - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
    #endif

    if (node == nullptr) { throw runtime_error("Error in processAssignment(): 'node' is null."); }

    // (1) Determine the number of children to see if assignment is binary or unary

    // BINARY
    if(node->getChildren().size() == 2) {

        auto lhs = node->getChildren()[0];
        auto rhs = node->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) { throw runtime_error("Error in processAssignment(): LHS or RHS is null."); }

        // (2) Set the nodes as visited if they are not nullptr
        lhs->setIsVisited(true); rhs->setIsVisited(true);

        // (3) Process the LHS
        auto lhsDecl = processIdentifier(lhs, false);   // false, we don't check the initialization but apply it to the LHS of assignments

        // (4) Process the RHS
        node::Node* rhsDecl = nullptr;
        switch (rhs->getNodeType()) {
            case NT::ID:
            case NT::ID_ARRAY:
            case NT::CALL:
                rhsDecl = processIdentifier(rhs);
                break;
            case NT::ASSIGNMENT:
                processAssignment(rhs);
                break;
            case NT::OPERATOR:
                processBinaryOperator(rhs);
                break;
            default:
                break;
        }

        if(lhsDecl) node->setVarType(lhsDecl->getVarType());
        else node->setVarType(lhs->getVarType());

        checkBinaryTypes(node, lhs, rhs);

        return;
    }
    
    // UNARY
    if(node->getChildren().size() == 1) {

        auto lhs = node->getChildren()[0];

        if (lhs == nullptr) { throw runtime_error("Error in processAssignment(): LHS is null."); }

        // (2) Set the node as visited if it is not nullptr
        lhs->setIsVisited(true);

        auto lhsDecl = processIdentifier(lhs);
        
        checkUnaryTypes(node, lhs);

        return;
    }
}

void SemanticAnalyzer::processBinaryOperator(node::Node *node) {
    #if PENDANTIC_DEBUG
    cout << "[Process Binary - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
    #endif

    if (node == nullptr) { throw runtime_error("Error in processBinaryOperator(): 'node' is null."); }

    auto lhs = node->getChildren()[0];
    auto rhs = node->getChildren()[1];

    if (lhs == nullptr || rhs == nullptr) { throw runtime_error("Error in processBinaryOperator(): LHS or RHS is null."); }

    // (1) Set the nodes as visited if they are not nullptr
    lhs->setIsVisited(true); rhs->setIsVisited(true);

    // (2) Process the LHS
    switch (lhs->getNodeType()) {
        default:
            processIdentifier(lhs);
            break;
    }

    // (3) Process the RHS
    switch (rhs->getNodeType()) {
        default:
            processIdentifier(rhs);
            break;
    }

    checkBinaryTypes(node, lhs, rhs);
}

void SemanticAnalyzer::processBooleanBinaryOperator(node::Node *node) {
    #if PENDANTIC_DEBUG
    cout << "[Process Boolean Binary - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
    #endif

    if (node == nullptr) { throw runtime_error("Error in processBooleanBinaryOperator(): 'node' is null."); }

    auto lhs = node->getChildren()[0];
    auto rhs = node->getChildren()[1];

    if (lhs == nullptr || rhs == nullptr) { throw runtime_error("Error in processBooleanBinaryOperator(): LHS or RHS is null."); }

    // (1) Set the nodes as visited if they are not nullptr
    lhs->setIsVisited(true); rhs->setIsVisited(true);

    // (2) Process the LHS
    switch (lhs->getNodeType()) {
        default:
            processIdentifier(lhs);
            break;
    }

    // (3) Process the RHS
    switch (rhs->getNodeType()) {
        default:
            processIdentifier(rhs);
            break;
    }

    checkBinaryTypes(node, lhs, rhs);
}

void SemanticAnalyzer::processUnaryOperator(node::Node *node) {
    #if PENDANTIC_DEBUG
    cout << "[Process Unary - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
    #endif

    if (node == nullptr) { throw runtime_error("Error in processUnaryOperator(): 'node' is null."); }

    auto operand = node->getChildren()[0];

    if (operand == nullptr) { throw runtime_error("Error in processUnaryOperator(): 'operand' is null."); }

    // (1) Set the node as visited if it is not nullptr
    operand->setIsVisited(true);
}

}