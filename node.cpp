#include "node.hpp"
#include "types.hpp"
#include <iostream>

typedef types::NodeType NT;

using namespace std;

namespace node {

    // Root of the AST
    Node *root = nullptr;

    /**
     * @fn getInt
     * @brief Returns integer variant of the value.
     * @return int
     */
    int Node::getInt() const {
        if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        throw std::bad_variant_access();
    }

    /**
     * @fn getChar
     * @brief Returns character variant of the value.
     * @return char
     */
    char Node::getChar() const {
        if (std::holds_alternative<char>(value_)) {
            return std::get<char>(value_);
        }
        throw std::bad_variant_access();
    }

    /**
     * @fn getString
     * @brief Returns string variant of the value.
     * @return std::string
     */
    std::string Node::getString() const {
        if (std::holds_alternative<std::string>(value_)) {
            return std::get<std::string>(value_);
        }
        throw std::bad_variant_access();
    }

    /**
     * @fn getBool
     * @brief Returns boolean variant of the value.
     * @return int
     */
    int Node::getBool() const {
        if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        throw std::bad_variant_access();
    }

    /**
     * @fn setSibling
     * @param sibling The sibling node to set.
     * @brief Sets the sibling node of the current node.
     * @return void
     */
    void Node::setSibling(Node* sibling) {
        if( sibling_ == nullptr ) {
            sibling_ = sibling; // Setting the sibling node of the current node
            sibling_->setSibLoc(siblingLocation_ + 1);

            Node *current = sibling_->getSibling();
            int loc = sibling_->getSibLoc();
            while (current != nullptr) {
                current->setSibLoc(loc + 1);
                loc = current->getSibLoc();
                current = current->getSibling();
            }
            return;
        }
        if( sibling_ != nullptr ) {
            Node *current = sibling_;
            while (current->getSibling() != nullptr) {
                current = current->getSibling();
            }
            current->setSibling(sibling);
            sibling->setSibLoc(current->getSibLoc() + 1);
        } 
    }

    /**
     * @fn printValue
     * @brief Prints the value of the node to the console.
     * @return void
     */
    void Node::printValue() {
        switch (nodeType_) {

            // Print these specific CONSTANT nodes.

            case NT::CHARACTER:
                cout << "Const ";
                cout << "'" << getChar() << "'";
                return;
            case NT::BOOLEAN:
                cout << "Const ";
                getBool() == 1 ? cout << "true" : cout << "false";
                return;
            case NT::NUMBER:
                cout << "Const ";
                cout << getInt();
                return;
            case NT::STRING:
                cout << "Const ";
                cout << "\"" << getString() << "\"";
                return;

            // These nodes are no different from the rest other than having an extra space after them in the output.

            case NT::FUNCTION:
            case NT::PARAMETER:
            case NT::PARAMETER_ARRAY:
            case NT::VARIABLE:
            case NT::VARIABLE_ARRAY:
            case NT::STATIC_VARIABLE:
                cout << types::nodeTypeToStr(nodeType_) << getString() << " ";
                return;

            // Print these nodes expect for their stored value.

            case NT::COMPOUND:
            case NT::ARRAY:
            case NT::CHSIGN_UNARY:
            case NT::RETURN:
            case NT::FOR:
            case NT::SIZEOF_UNARY:
            case NT::RANGE:
            case NT::WHILE:
            case NT::IF:
            case NT::BREAK:
            case NT::OR:
            case NT::AND:
            case NT::NOT:
            case NT::QUES_UNARY:
                cout << types::nodeTypeToStr(nodeType_);
                return;

            // By default, print the nodetype and the value.

            default:
                cout << types::nodeTypeToStr(nodeType_) << getString();
                return;
        }
    }

    /**
     * @fn printNode
     * @param depth The depth of the node in the AST.
     * @brief Prints the node to the console.
     * @return void
     */
    void Node::printNode(int depth = 0) {
        if (siblingLocation_ != 0) {
            cout << utils::printIndent(depth) << "Sibling: " << siblingLocation_ << "  ";
        }
        printValue();
        cout << " [line: " << line_ << "]";
        cout << endl;
        std::flush(cout);
    }

    /**
     * @fn printTree
     * @param root Pointer to the root node of the AST.
     * @param depth The depth of the node in the AST.
     * @brief Prints the AST to the console. Recursive.
     * @return void
     */
    void printTree(Node *root, int depth = 0) {

        if (root == nullptr) {
            return;
        }

        /**
         * @note Children and Siblings
         * 
         * If the: 
         * 
         * - first child of a function is a compound statement then the child location will be 1.
         * 
         * - first child of a compound statement is anything but a VARIABLE or VARIABLE_ARRAY then the child location will start be 1.
         * 
         * These were very well addressed in the 'parser.y' wherein adding a child has a parameter to which location (i.e. childLocation)
         */
        Node *current = root;

        while (current != nullptr) {
            current->printNode(depth);
            if (current->getChildren().size() > 0) {
                for (auto child : current->getChildren()) {
                    cout << utils::printIndent(depth + 1) << "Child: " << child->getChildLoc() << "  ";
                    printTree(child, depth + 1);
                }
            }
            current = current->getSibling();
        }
    }

} // namespace node