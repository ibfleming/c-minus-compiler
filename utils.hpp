#ifndef UTILS_HPP
#define UTILS_HPP

#include "node.hpp"
#include <vector>
#include <stack>

/**
 * @namespace utils
 * @brief Contains utility functions for the compiler.
 */
namespace utils {

    extern bool PRINT_TYPES; // Print the types of the AST nodes

    /**
    * @fn printTree
    * @brief Prints the AST to the console.
    * @return void
    */
    void printTree();

    /**
     * @fn printIndent
     * @param times The number of times to print the indent.
     * @brief Prints an indent to the console for the AST.
     * @return string
     */
    std::string printIndent(int times);

    /**
     * @fn checkFileExtension
     * @param filename The name of the file to check.
     * @brief Checks if the file has a valid extension.
     * @return bool
     */
    bool checkFileExtension(std::string filename);

    /**
     * @fn printHelpMenu
     * @brief Prints the help menu to the console.
     * @return void
     */
    void printHelpMenu();

    /**
     * @fn stackToVector
     * @param stack The stack to convert to a vector.
     * @brief Converts a stack to a vector.
     * @return std::vector<T>
     * @tparam T The type of the stack.
     * @note The stack is not modified. Definition is inside the header file to prevent compilation errors.
     */
    template <typename T>
    std::vector<T> stackToVector(std::stack<T>& stack) {
        std::vector<T> vector;
        std::stack<T> tempStack = stack;
        while (!tempStack.empty()) {
            vector.push_back(tempStack.top());
            tempStack.pop();
        }
        std::reverse(vector.begin(), vector.end());
        return vector;
    }

} // namespace utils

#endif // UTILS_HPP