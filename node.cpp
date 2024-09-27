#include "node.hpp"

#include <iostream>

using namespace std;

namespace node {

    Node *root = nullptr;

    void printTree(Node *root) {

        if (root == nullptr) {
            return;
        }

        Node *current = root;

        while (current != nullptr) {
            current->printNode();
            current = current->getSibling();
        }
    }

} // namespace node