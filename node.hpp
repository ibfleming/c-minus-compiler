#ifndef NODE_HPP
#define NODE_HPP

#include "token.hpp"
#include "utils.hpp"
#include <vector>

/**
 * @brief Contains the Node class and related functions.
 */
namespace node {

class Node; // forward declaration

extern Node* root; // Root of the AST

/**
 * @param root Pointer to the root node of the AST.
 * @param depth The depth of the node in the AST.
 * @brief Prints the AST to the console. Using DFS (recursive).
 */
void printTree(Node* root, int depth);

/**
 * @brief Represents a node in the Abstract Syntax Tree (AST).
 */
class Node {

private:
    bool isInitialized_; // flag to check if the node is initialized (for VARS, etc.)
    bool isVisited_; // flag to check if the node has been visited (for particular traversals)
    bool isUsed_; // flag to check if the node is used (for VARS, etc.)
    bool isArray_; // flag to check if the node is an array
    bool isConstant_; // flag to check if the node is a constant
    Node* declaration_; // declaration node for the variable for ID/ID_ARRAY nodes
    std::vector<node::Node*> children_; // children of the node
    int childLocation_; // location of the child in the parent's children vector
    Node* sibling_; // sibling of the node
    int siblingLocation_; // location of the sibling in the parent's children vector
    Node* function_; // FUNCTION node for COMPOUNDS that are the function's body
    types::NodeType nodeType_; // FUNCTION, VAR, etc.
    types::OperatorType opType_; // ADD, SUB, MUL, DIV, MOD, UNKNOWN
    types::AssignmentType asgnType_; // ASGN, ADDASGN, SUBASGN, MULASGN, DIVASGN, UNKNOWN
    types::VarType varType_; // INT, CHAR, BOOL, STATIC, UNKNOWN
    types::TokenValue value_; // value of the token (int, char, string) after processing
    int line_; // line number of the token

public:
    /**
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     */
    Node(token::Token* token, types::NodeType nodeType)
        : value_(token->getValue())
        , line_(token->getLine())
        , nodeType_(nodeType)
        , opType_(types::OperatorType::UNKNOWN)
        , asgnType_(types::AssignmentType::UNKNOWN)
        , varType_(types::VarType::UNDEFINED)
        , sibling_(nullptr)
        , function_(nullptr)
        , declaration_(nullptr)
        , isInitialized_(false)
        , isVisited_(false)
        , isUsed_(false)
        , siblingLocation_(0)
        , childLocation_(0)
    {
        if (nodeType == types::NodeType::VARIABLE_ARRAY
            || nodeType == types::NodeType::PARAMETER_ARRAY
            || nodeType == types::NodeType::VARIABLE_STATIC_ARRAY
            || nodeType == types::NodeType::STRING) {
            isArray_ = true;
        } else {
            isArray_ = false;
        }
    }

    /**
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     * @param varType The variable type of the node.
     */
    Node(token::Token* token, types::NodeType nodeType, types::VarType varType)
        : value_(token->getValue())
        , line_(token->getLine())
        , nodeType_(nodeType)
        , opType_(types::OperatorType::UNKNOWN)
        , asgnType_(types::AssignmentType::UNKNOWN)
        , varType_(varType)
        , sibling_(nullptr)
        , function_(nullptr)
        , declaration_(nullptr)
        , isInitialized_(false)
        , isVisited_(false)
        , isUsed_(false)
        , siblingLocation_(0)
        , childLocation_(0)
    {
        if (nodeType == types::NodeType::VARIABLE_ARRAY
            || nodeType == types::NodeType::PARAMETER_ARRAY
            || nodeType == types::NodeType::VARIABLE_STATIC_ARRAY
            || nodeType == types::NodeType::STRING) {
            isArray_ = true;
        } else {
            isArray_ = false;
        }
    }

    /**
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     * @param opType The operator type of the node.
     * @param varType The variable type of the node.
     */
    Node(token::Token* token, types::NodeType nodeType,
        types::OperatorType opType, types::VarType varType)
        : value_(token->getValue())
        , line_(token->getLine())
        , nodeType_(nodeType)
        , opType_(opType)
        , asgnType_(types::AssignmentType::UNKNOWN)
        , varType_(varType)
        , sibling_(nullptr)
        , function_(nullptr)
        , declaration_(nullptr)
        , isInitialized_(false)
        , isVisited_(false)
        , isUsed_(false)
        , siblingLocation_(0)
        , childLocation_(0)
    {
        if (nodeType == types::NodeType::VARIABLE_ARRAY
            || nodeType == types::NodeType::PARAMETER_ARRAY
            || nodeType == types::NodeType::VARIABLE_STATIC_ARRAY
            || nodeType == types::NodeType::STRING) {
            isArray_ = true;
        } else {
            isArray_ = false;
        }
    }

    /**
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     * @param asgnType The assignment type of the node.
     * @param varType The variable type of the node.
     */
    Node(token::Token* token, types::NodeType nodeType,
        types::AssignmentType asgnType, types::VarType varType)
        : value_(token->getValue())
        , line_(token->getLine())
        , nodeType_(nodeType)
        , opType_(types::OperatorType::UNKNOWN)
        , asgnType_(asgnType)
        , varType_(varType)
        , sibling_(nullptr)
        , function_(nullptr)
        , declaration_(nullptr)
        , isInitialized_(false)
        , isVisited_(false)
        , isUsed_(false)
        , siblingLocation_(0)
        , childLocation_(0)
    {
        if (nodeType == types::NodeType::VARIABLE_ARRAY
            || nodeType == types::NodeType::PARAMETER_ARRAY
            || nodeType == types::NodeType::VARIABLE_STATIC_ARRAY
            || nodeType == types::NodeType::STRING) {
            isArray_ = true;
        } else {
            isArray_ = false;
        }
    }

    // Getters

    bool getIsInitialized() const
    {
        return isInitialized_;
    }
    bool getIsVisited() const
    {
        return isVisited_;
    }
    bool getIsUsed() const
    {
        return isUsed_;
    }
    bool getIsArray() const
    {
        return isArray_;
    }
    bool getIsConst() const
    {
        return isConstant_;
    }
    Node* getFunctionNode() const
    {
        return function_;
    }
    std::vector<node::Node*> getChildren() const
    {
        return children_;
    }
    int getChildLoc() const
    {
        return childLocation_;
    }
    Node* getSibling() const
    {
        return sibling_;
    }
    int getSibLoc() const
    {
        return siblingLocation_;
    }
    types::NodeType getNodeType() const
    {
        return nodeType_;
    }
    types::OperatorType getOpType() const
    {
        return opType_;
    }
    types::AssignmentType getAsgnType() const
    {
        return asgnType_;
    }
    types::VarType getVarType() const
    {
        return varType_;
    }
    int getLine() const
    {
        return line_;
    }

    // Setters

    void setIsInitialized(bool isInitialized)
    {
        isInitialized_ = isInitialized;
    }
    void setIsVisited(bool isVisited)
    {
        isVisited_ = isVisited;
    }
    void setIsUsed(bool isUsed)
    {
        isUsed_ = isUsed;
    }
    void setIsArray(bool isArray)
    {
        isArray_ = isArray;
    }
    void setIsConst(bool isConst)
    {
        isConstant_ = isConst;
    }
    void setDeclaration(Node* declaration)
    {
        if (declaration != nullptr) {
            declaration->setIsUsed(true);
            isInitialized_ = declaration->getIsInitialized();
            isArray_ = declaration->getIsArray();
            varType_ = declaration->getVarType();
        }
    }
    void setFunctionNode(Node* function)
    {
        function_ = function;
    }
    void addChild(Node* child)
    {
        children_.push_back(child);
    }
    void addChild(Node* child, int loc)
    {
        children_.push_back(child);
        child->setChildLoc(loc);
    }
    void setChildLoc(int loc)
    {
        childLocation_ = loc;
    }
    void setSibLoc(int loc)
    {
        siblingLocation_ = loc;
    }
    void setNodeType(types::NodeType nodeType)
    {
        nodeType_ = nodeType;
    }
    void setOpType(types::OperatorType opType)
    {
        opType_ = opType;
    }
    void setAsgnType(types::AssignmentType asgnType)
    {
        asgnType_ = asgnType;
    }
    void setVarType(types::VarType varType)
    {
        varType_ = varType;
    }

    /**
     * @fn setSibling
     * @param sibling The sibling to set.
     * @brief Sets the sibling of the node.
     */
    void setSibling(Node* sibling);

    // TokenValue Getters (int, char, string)

    /**
     * @fn getInt
     * @brief Returns integer variant of the value.
     * @return int
     */
    int getInt() const;

    /**
     * @fn getChar
     * @brief Returns character variant of the value.
     * @return char
     */
    char getChar() const;

    /**
     * @fn getString
     * @brief Returns string variant of the value.
     *
     * If no string is present, check if integer and if so return the integer as
     * a string.
     * @return std::string
     */
    std::string getString() const;

    /**
     * @fn getBool
     * @brief Returns boolean variant of the value.
     * @return int
     */
    int getBool() const;

    /**
     * @fn printValue
     * @brief Prints the value of the node to the console (for the AST).
     * @brief The node type does determine the formatting of the output.
     */
    void printValue();

    /**
     * @fn printType
     * @brief Prints the value of the node type to the console (for the AST).
     * @brief The node type does determine the formatting of the output.
     */
    void printType();

    /**
     * @fn printNode
     * @param depth The depth of the node in the AST.
     * @brief Prints the node to the console (for the AST).
     */
    void printNode(int depth);
};

} // namespace node

#endif // NODE_HPP