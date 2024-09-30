#ifndef NODE_HPP
#define NODE_HPP

#include "token.hpp"
#include "utils.hpp"
#include <iostream>
#include <vector>

namespace node {

class Node {
private:
    std::vector<node::Node*> children_;
    int childLocation_;
    Node* sibling_;
    int siblingLocation_;
    types::NodeType nodeType_;   // FUNCTION, VAR, etc.
    types::VarType varType_;     // INT, CHAR, BOOL, STATIC, UNKNOWN
    types::TokenType tokenType_; // ID, NUMCONST, CHARCONST, STRINGCONST, BOOLCONST, etc.
    types::TokenValue value_;    // value of the token (int, char, string) after processing
    int line_;
public:
    /**
     * @fn Node
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     */
    Node(token::Token *token, types::NodeType nodeType) 
    : nodeType_(nodeType), varType_(types::VarType::UNKNOWN), siblingLocation_(0), childLocation_(0), sibling_(nullptr) 
    {
        line_ = token->getLine();
        value_ = token->getValue();
        tokenType_ = token->getType();
    }

    /**
     * @fn Node
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     * @param varType The variable type of the node.
     */
    Node(token::Token *token, types::NodeType nodeType, types::VarType varType) 
    : nodeType_(nodeType), varType_(varType), siblingLocation_(0), childLocation_(0), sibling_(nullptr) 
    {
        line_ = token->getLine();
        value_ = token->getValue();
        tokenType_ = token->getType();
    }

    // Getters

    types::NodeType getNodeType() const { return nodeType_; }
    types::VarType getVarType() const { return varType_; }
    types::TokenType getTokenType() const { return tokenType_; }
    std::vector<node::Node*> getChildren() const { return children_; }
    Node* getSibling() const { return sibling_; }
    int getLine() const { return line_; }
    int getSibLoc() const { return siblingLocation_; }
    int getChildLoc() const { return childLocation_; }

    // TokenValue Getters (int, char, string)

    int getInt() const;
    char getChar() const;
    std::string getString() const;
    int getBool() const;

    // Setters

    void setNodeType(types::NodeType nodeType) { nodeType_ = nodeType; }
    void setVarType(types::VarType varType) { varType_ = varType; }
    void setSibLoc(int loc) { siblingLocation_ = loc; }
    void setChildLoc(int loc) { childLocation_ = loc; }

    void addChild(Node* child) { children_.push_back(child); }
    void addChild(Node* child, int loc) { children_.push_back(child); child->setChildLoc(loc); }
    
    // Print Functions

    void printValue();
    void printNode(int depth);

    // Functions

    void setSibling(Node* sibling);
};

extern Node *root;
void printTree(Node *root, int depth);

} // namespace node

#endif // NODE_HPP