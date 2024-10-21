#ifndef UTILS_HPP
#define UTILS_HPP

#include "node.hpp"
#include <iomanip>
#include <stack>
#include <vector>

namespace node {
class Node;
}

/**
 * @brief Contains utility functions for the compiler.
 */
namespace utils {

extern bool PRINT_TYPES; // Print the types of the AST nodes

/**
 * @brief Prints the AST to the console.
 * @return void
 */
void printTree();

/**
 * @param times The number of times to print the indent.
 * @brief Prints an indent to the console for the AST.
 * @return string
 */
std::string printIndent(int times);

/**
 * @param filename The name of the file to check.
 * @brief Checks if the file has a valid extension.
 * @return bool
 */
bool checkFileExtension(std::string filename);

/**
 * @brief Prints the help menu to the console.
 * @return void
 */
void printHelpMenu();

/**
 * @param stack The stack to convert to a vector in reverse order.
 * @brief Converts a stack to a vector.
 * @return std::vector<T>
 * @tparam T The type of the stack.
 * @note The stack is not modified. Definition is inside the header file to prevent compilation errors.
 */
template <typename T>
std::vector<T> stackToVectorReverse(std::stack<T>& stack)
{
    std::vector<T> vector;
    std::stack<T> tempStack = stack;
    while (!tempStack.empty()) {
        vector.push_back(tempStack.top());
        tempStack.pop();
    }
    std::reverse(vector.begin(), vector.end());
    return vector;
}

/**
 * @param stack The stack to convert to a vector in order.
 * @brief Converts a stack to a vector.
 * @return std::vector<T>
 * @tparam T The type of the stack.
 * @note The stack is not modified. Definition is inside the header file to prevent compilation errors.
 */
template <typename T>
std::vector<T> stackToVectorInOrder(std::stack<T>& stack)
{
    std::vector<T> vector;
    std::stack<T> tempStack = stack;
    while (!tempStack.empty()) {
        vector.push_back(tempStack.top());
        tempStack.pop();
    }
    return vector;
}

/**
 * @param node The node to check if it is an array.
 * @brief Checks if the node is an array.
 * @return bool
 */
bool isArray(node::Node* node);

void printLine(node::Node* node);

} // namespace utils

#endif // UTILS_HPP