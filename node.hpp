#ifndef NODE_HPP
#define NODE_HPP

#include "types.hpp"
#include "token.hpp"
#include <iostream>
#include <vector>

namespace node {

class Node {
public:
    Node(token::Token *token, types::NodeType nodeType) : nodeType(nodeType), varType(types::VarType::UNKNOWN), arrayType(types::ArrayType::NOT_ARRAY) {
        line = token->getLine();
        value = token->getValue();
        tokenType = token->getType();
    }
    Node(token::Token *token, types::NodeType nodeType, types::VarType varType) : nodeType(nodeType), varType(varType), arrayType(types::ArrayType::NOT_ARRAY) {
        line = token->getLine();
        value = token->getValue();
        tokenType = token->getType();
    }

    std::vector<node::Node*> children;
    Node* sibling;
    types::NodeType nodeType;   // FUNCTION, VAR, etc.
    types::VarType varType;     // INT, CHAR, BOOL, STATIC, UNKNOWN
    types::ArrayType arrayType; // optional?
    types::TokenType tokenType; // ID, NUMCONST, CHARCONST, STRINGCONST, BOOLCONST, etc.
    types::TokenValue value;    // value of the token (int, char, string) after processing
    int line;

    void setSibling(node::Node* sibling) { this->sibling = sibling; }
    Node* getSibling() { return sibling; }
    void addChild(node::Node* child) { children.push_back(child); }
    void setType(types::VarType varType) { this->varType = varType; }
    void setNodeType(types::NodeType nodeType) { this->nodeType = nodeType; }
    types::VarType getType() { return varType; }
private:
};

extern Node *root; // root node of AST

void printTree(Node *root);

} // namespace node

#endif // NODE_HPP