#ifndef NODE_HPP
#define NODE_HPP

#include "types.hpp"
#include "token.hpp"
#include <iostream>
#include <vector>

namespace node {

class Node {
public:
    /**
     * @fn Node
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     */
    Node(token::Token *token, types::NodeType nodeType) : nodeType_(nodeType), varType_(types::VarType::UNKNOWN) {
        line_ = token->getLine();
        value_ = token->getValue();
        tokenType_ = token->getType();
        sibling_ = nullptr;
        siblingLocation_ = 0;
    }

    /**
     * @fn Node
     * @param token The token to create a node from.
     * @param nodeType The type of node to create.
     * @param varType The variable type of the node.
     */
    Node(token::Token *token, types::NodeType nodeType, types::VarType varType) : nodeType_(nodeType), varType_(varType) {
        line_ = token->getLine();
        value_ = token->getValue();
        tokenType_ = token->getType();
        sibling_ = nullptr;
        siblingLocation_ = 0;
    }

    // Getters
    types::NodeType getNodeType() const { return nodeType_; }
    types::VarType getVarType() const { return varType_; }
    types::TokenType getTokenType() const { return tokenType_; }
    std::vector<node::Node*> getChildren() const { return children_; }
    Node* getSibling() const { return sibling_; }
    int getLine() const { return line_; }
    int getSibLoc() const { return siblingLocation_; }

    // Setters
    void setNodeType(types::NodeType nodeType) { nodeType_ = nodeType; }
    void setVarType(types::VarType varType) { varType_ = varType; }
    void setSibLoc(int loc) { siblingLocation_ = loc; } 
    void addChild(Node* child) { children_.push_back(child); }
    
    // Functions
    void printNode() {
        if ( siblingLocation_ != 0 ) {
            std::cout << "Sibling: " << siblingLocation_ << "  ";
        }
        std::cout << types::nodeTypeToStr(nodeType_) << ": ";
        std::cout << "  [line: " << line_ << "]";
        std::cout << std::endl;
    }

    void setSibling(Node* sibling) {
        if (sibling_ == nullptr) {
            sibling_ = sibling;
            sibling_->setSibLoc(siblingLocation_ + 1);
        } else {
            Node *temp = sibling_;
            int loc = temp->getSibLoc();
            while( temp->getSibling() != nullptr) {
                temp = temp->getSibling();
                loc = temp->getSibLoc();
            }
            temp->setSibling(sibling);
            sibling->setSibLoc(loc + 1);
        }
    }




private:
    std::vector<node::Node*> children_;
    Node* sibling_;
    int siblingLocation_;
    types::NodeType nodeType_;   // FUNCTION, VAR, etc.
    types::VarType varType_;     // INT, CHAR, BOOL, STATIC, UNKNOWN
    types::TokenType tokenType_; // ID, NUMCONST, CHARCONST, STRINGCONST, BOOLCONST, etc.
    types::TokenValue value_;    // value of the token (int, char, string) after processing
    int line_;
};

extern Node *root; // root node of AST

/**
 * @fn printTree
 * @param root Pointer to the root node of the AST.
 * @brief Prints the AST to the console.
 * @return void
 */
void printTree(Node *root);

} // namespace node

#endif // NODE_HPP