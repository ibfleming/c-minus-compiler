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
        std::cout << "Created Node: " << token->getToken() << std::endl;
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
    void setChild(node::Node* child) { children.push_back(child); }
private:
};

extern Node* root;

} // namespace node

#endif // NODE_HPP