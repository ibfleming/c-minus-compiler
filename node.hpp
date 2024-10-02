#ifndef NODE_HPP
#define NODE_HPP

#include "token.hpp"
#include "utils.hpp"
#include <vector>

/**
 * @namespace node
 * @brief Contains the Node class and related functions.
 */
namespace node {

class Node;   // forward declaration

extern Node *root;   // Root of the AST

/**
 * @fn printTree
 * @param root Pointer to the root node of the AST.
 * @param depth The depth of the node in the AST.
 * @brief Prints the AST to the console. Using DFS (recursive).
 * @return void
 */
void printTree(Node *root, int depth);

/**
 * @class Node
 * @brief Represents a node in the Abstract Syntax Tree (AST).
 */
class Node {

private:
    bool isInitialized_;                    // flag to check if the node is initialized (for VARS, etc.)
    std::vector<node::Node*> children_;     // children of the node
    int childLocation_;                     // location of the child in the parent's children vector
    Node* sibling_;                         // sibling of the node
    int siblingLocation_;                   // location of the sibling in the parent's children vector
    types::NodeType nodeType_;              // FUNCTION, VAR, etc.
    types::VarType varType_;                // INT, CHAR, BOOL, STATIC, UNKNOWN
    types::TokenValue value_;               // value of the token (int, char, string) after processing
    int line_;                              // line number of the token

public:
    /**
     * @fn Node
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     */
    Node(token::Token *token, types::NodeType nodeType) 
    : nodeType_(nodeType), varType_(types::VarType::UNKNOWN), siblingLocation_(0), childLocation_(0), sibling_(nullptr), isInitialized_(false) 
    {
        line_ = token->getLine();
        value_ = token->getValue();
    }

    /**
     * @fn Node
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     * @param varType The variable type of the node.
     */
    Node(token::Token *token, types::NodeType nodeType, types::VarType varType) 
    : nodeType_(nodeType), varType_(varType), siblingLocation_(0), childLocation_(0), sibling_(nullptr), isInitialized_(false)
    {
        line_ = token->getLine();
        value_ = token->getValue();
    }

    // Getters

    bool getIsInitialized() const { return isInitialized_; }
    std::vector<node::Node*> getChildren() const { return children_; }
    int getChildLoc() const { return childLocation_; }
    Node* getSibling() const { return sibling_; }
    int getSibLoc() const { return siblingLocation_; }
    types::NodeType getNodeType() const { return nodeType_; }
    types::VarType getVarType() const { return varType_; }
    int getLine() const { return line_; }

    // Setters

    void setIsInitialized(bool isInitialized) { isInitialized_ = isInitialized; }
    void addChild(Node* child) { children_.push_back(child); }
    void addChild(Node* child, int loc) { children_.push_back(child); child->setChildLoc(loc); }
    void setChildLoc(int loc) { childLocation_ = loc; }
    void setSibLoc(int loc) { siblingLocation_ = loc; }
    void setNodeType(types::NodeType nodeType) { nodeType_ = nodeType; }
    void setVarType(types::VarType varType) { varType_ = varType; }
    
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
     * If no string is present, check if integer and if so return the integer as a string.
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

    /**
     * @fn pendanticPrint
     * @brief Prints the node to the console for pendantic purposes.
     * @brief This will print the 
     */
    void pendanticPrint();

};

} // namespace node

#endif // NODE_HPP