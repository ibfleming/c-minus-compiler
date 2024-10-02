#include "node.hpp"
#include "types.hpp"
#include <iostream>

typedef types::NodeType NT;     // shorthand for NodeType

using namespace std;

namespace node {

Node *root = nullptr;

int Node::getInt() const {
        if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        throw std::bad_variant_access();
    }

char Node::getChar() const {
        if (std::holds_alternative<char>(value_)) {
            return std::get<char>(value_);
        }
        throw std::bad_variant_access();
    }

std::string Node::getString() const {
        if (std::holds_alternative<std::string>(value_)) {
            return std::get<std::string>(value_);
        }
        if (std::holds_alternative<int>(value_)) {
            return std::to_string(std::get<int>(value_));
        }
        throw std::bad_variant_access();
    }

int Node::getBool() const {
        if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        throw std::bad_variant_access();
    }

void Node::setSibling(Node* sibling) {
        if( sibling_ == nullptr ) {
            sibling_ = sibling;
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

            // These nodes are no different from the rest other than having an extra space after them in the output. Sigh...

            case NT::FUNCTION:
            case NT::PARAMETER:
            case NT::VARIABLE:
            case NT::VARIABLE_STATIC:
                cout << types::nodeTypeToStr(nodeType_) << getString();
                if( !utils::PRINT_TYPES ) { cout << " "; }
                return;

            case NT::PARAMETER_ARRAY:
            case NT::VARIABLE_ARRAY:
            case NT::VARIABLE_STATIC_ARRAY:
                cout << types::nodeTypeToStr(nodeType_) << getString();
                if( !utils::PRINT_TYPES ) { cout << " "; }
                return;

            // Print these nodes expect for their stored value.

            case NT::COMPOUND:
            case NT::ID_ARRAY:
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

void Node::printType() {
        switch (nodeType_) {

            case NT::COMPOUND:
            case NT::WHILE:
                return;

            case NT::VARIABLE_ARRAY:
            case NT::PARAMETER_ARRAY:
            case NT::VARIABLE_STATIC_ARRAY:
                cout << " is array of type ";
                cout << types::varTypeToStr(varType_);
                return;

            default:
                cout << " of type ";
                cout << types::varTypeToStr(varType_);
                return;

        }
    }

void Node::printNode(int depth = 0) {
        if (siblingLocation_ != 0) {
            cout << utils::printIndent(depth) << "Sibling: " << siblingLocation_ << "  ";
        }
        printValue();
        if( utils::PRINT_TYPES ) { printType(); }
        cout << " [line: " << line_ << "]";
        cout << endl;
        std::flush(cout);
    }

void Node::pendanticPrint() {
    cout << types::pendaticNodeTypeToStr(nodeType_) << "(" << line_ << "): " << getString()  << endl;
}

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