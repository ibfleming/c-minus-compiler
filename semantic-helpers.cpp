/*********************************************************************
 * @file semantic.cpp
 *
 * @brief Source file for semantic analysis.
 *********************************************************************/

#include "semantic.hpp"

using namespace std;

namespace semantic {

void SemanticAnalyzer::analyzeNode(node::Node* node)
{

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
    if (!node->getIsVisited()) {
        // Set the node as visited
        node->setIsVisited(true);
        switch (node->getNodeType()) {

        // Variable Declarations
        case NT::VARIABLE:
        case NT::VARIABLE_STATIC:
        case NT::PARAMETER:
        case NT::VARIABLE_ARRAY:
        case NT::VARIABLE_STATIC_ARRAY:
        case NT::PARAMETER_ARRAY: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            insertSymbol(node);
            checkForInitializer(node);
            return;
        }

        // Identifiers
        case NT::ID:
        case NT::ID_ARRAY:
        case NT::CALL: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            processIdentifier(node);
            return;
        }

        // Binary Operations
        case NT::ASSIGNMENT: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            processAssignment(node);
            return;
        }
        case NT::OPERATOR: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            processBinaryOperator(node);
            return;
        }
        case NT::AND:
        case NT::OR: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            processBooleanBinaryOperator(node);
            return;
        }

        // Unary Operations
        case NT::NOT:
        case NT::SIZEOF_UNARY:
        case NT::CHSIGN_UNARY:
        case NT::QUES_UNARY: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            processUnaryOperator(node);
            return;
        }

        // If
        case NT::IF: {
            // processIf(node);
            return;
        }

        // While
        case NT::WHILE: {
            // processWhile(node);
            return;
        }

        // Return
        case NT::RETURN: {
#if PENDANTIC_DEBUG
            utils::printLine(node);
#endif
            processReturn(node);
            return;
        }

        default:
            return;
        }
    }
}

void SemanticAnalyzer::processAssignment(node::Node* node)
{
#if PENDANTIC_DEBUG
    cout << "[Process Assign - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
#endif

    if (node == nullptr) {
        throw runtime_error("Error in processAssignment(): 'node' is null.");
    }

    // (1) Determine the number of children to see if assignment is binary or unary

    // BINARY
    if (node->getChildren().size() == 2) {

        auto lhs = node->getChildren()[0];
        auto rhs = node->getChildren()[1];

        if (lhs == nullptr || rhs == nullptr) {
            throw runtime_error("Error in processAssignment(): LHS or RHS is null.");
        }

        // (2) Set the nodes as visited if they are not nullptr
        lhs->setIsVisited(true);
        rhs->setIsVisited(true);

        // (3) Process the RHS
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

        // (4) Process the LHS
        auto lhsDecl = processIdentifier(lhs, false);

        if (lhsDecl)
            node->setVarType(lhsDecl->getVarType());
        else {
            node->setVarType(lhs->getVarType());
            return;
        }

        checkBinaryTypes(node, lhs, rhs);

        return;
    }

    // UNARY
    if (node->getChildren().size() == 1) {

        auto lhs = node->getChildren()[0];

        if (lhs == nullptr) {
            throw runtime_error("Error in processAssignment(): LHS is null.");
        }

        // (2) Set the node as visited if it is not nullptr
        lhs->setIsVisited(true);

        auto lhsDecl = processIdentifier(lhs);

        checkUnaryTypes(node, lhs);

        return;
    }
}

void SemanticAnalyzer::processBinaryOperator(node::Node* node)
{
#if PENDANTIC_DEBUG
    cout << "[Process Binary - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
#endif

    if (node == nullptr) {
        throw runtime_error("Error in processBinaryOperator(): 'node' is null.");
    }

    auto lhs = node->getChildren()[0];
    auto rhs = node->getChildren()[1];

    if (lhs == nullptr || rhs == nullptr) {
        throw runtime_error("Error in processBinaryOperator(): LHS or RHS is null.");
    }

    // (1) Set the nodes as visited if they are not nullptr
    lhs->setIsVisited(true);
    rhs->setIsVisited(true);

    // (2) Process the LHS
    node::Node* lhsDecl = nullptr;
    switch (lhs->getNodeType()) {
    case NT::OPERATOR:
        processBinaryOperator(lhs);
        break;
    default:
        lhsDecl = processIdentifier(lhs);
        break;
    }

    // (3) Process the RHS
    node::Node* rhsDecl = nullptr;
    switch (rhs->getNodeType()) {
    case NT::OPERATOR:
        processBinaryOperator(rhs);
        break;
    default:
        rhsDecl = processIdentifier(rhs);
        break;
    }

    // (4) Check if LHS & RHS are functions not being called but as variables
    if (isDeclarationFunctionAsVariable(lhs, lhsDecl))
        return;
    if (isDeclarationFunctionAsVariable(rhs, rhsDecl))
        return;

    // (5) Check Types
    checkBinaryTypes(node, lhs, rhs);
}

void SemanticAnalyzer::processBooleanBinaryOperator(node::Node* node)
{
#if PENDANTIC_DEBUG
    cout << "[Process Boolean Binary - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
#endif

    if (node == nullptr) {
        throw runtime_error("Error in processBooleanBinaryOperator(): 'node' is null.");
    }

    auto lhs = node->getChildren()[0];
    auto rhs = node->getChildren()[1];

    if (lhs == nullptr || rhs == nullptr) {
        throw runtime_error("Error in processBooleanBinaryOperator(): LHS or RHS is null.");
    }

    // (1) Set the nodes as visited if they are not nullptr
    lhs->setIsVisited(true);
    rhs->setIsVisited(true);

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

void SemanticAnalyzer::processUnaryOperator(node::Node* node)
{
#if PENDANTIC_DEBUG
    cout << "[Process Unary - " << types::literalNodeTypeStr(node->getNodeType()) << "]" << endl;
#endif

    if (node == nullptr) {
        throw runtime_error("Error in processUnaryOperator(): 'node' is null.");
    }

    auto operand = node->getChildren()[0];

    if (operand == nullptr) {
        throw runtime_error("Error in processUnaryOperator(): 'operand' is null.");
    }

    // (1) Set the node as visited if it is not nullptr
    operand->setIsVisited(true);

    // (2) Process the operand
    node::Node* operandDecl = nullptr;
    switch (operand->getNodeType()) {
    case NT::OPERATOR:
        processBinaryOperator(operand);
        break;
    case NT::CHSIGN_UNARY:
        processUnaryOperator(operand);
        break;
    default:
        operandDecl = processIdentifier(operand);
        break;
    }

    checkUnaryTypes(node, operand);
}

bool SemanticAnalyzer::isDeclarationFunctionAsVariable(node::Node* id, node::Node* decl)
{
    if (id && decl) {
        if (id->getNodeType() != NT::CALL) {
            if (decl->getNodeType() == NT::FUNCTION) {
                logger::ERROR_VariableAsFunction(this, id);
                return true;
            }
        }
    }
    return false;
}

node::Node* SemanticAnalyzer::processArray(node::Node* arr, bool init)
{
    if (arr == nullptr)
        throw runtime_error("Error in processArray(): 'arr' is null.");

#if PENDANTIC_DEBUG
    cout << "[Process Array]" << endl;
#endif

    if (arr->getChildren().size() == 2) {

        auto id = arr->getChildren()[0];
        auto index = arr->getChildren()[1];

        if (id == nullptr || index == nullptr)
            throw runtime_error("Error in processArray(): 'id' or 'index' is null.");

        id->setIsVisited(true);
        index->setIsVisited(true);

        /**
         *  Processing Child 0: ID
         */

        // (1) Process Child 0 (the ID)
        auto arrDecl = processIdentifier(id, init);

        if (arrDecl) {
            // (2) Set the ID_ARRAY type to the ID if found
            arr->setVarType(arrDecl->getVarType());

            // (3) Error if the array declaration isn't actually an array
            if (!arrDecl->getIsArray()) {
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

        node::Node* indexDecl = nullptr;
        switch (index->getNodeType()) {
        case NT::OPERATOR: {
            processBinaryOperator(index);
            break;
        }
        case NT::ASSIGNMENT: {
            processAssignment(index);
            break;
        }
        default: {
            indexDecl = processIdentifier(index, init);
            if (indexDecl) {
                if (indexDecl->getVarType() != VT::INT) {
                    logger::ERROR_ArrayIndexNotInt(this, arr, index);
                }
                if (index->getNodeType() == NT::ID) {
                    if (indexDecl->getIsArray())
                        logger::ERROR_ArrayIndexIsUnindexedArray(this, index);
                }
            } else {
                if (index->getVarType() != VT::INT) {
                    logger::ERROR_ArrayIndexNotInt(this, arr, index);
                }
            }
            break;
        }
        }

        if (indexDecl == nullptr) {
            if (index->getVarType() != VT::INT)
                logger::ERROR_ArrayIndexNotInt(this, arr, index);
        }

        // Last: return the ID declaration
        return arrDecl;
    }
    return nullptr;
}

} // namespace semantic