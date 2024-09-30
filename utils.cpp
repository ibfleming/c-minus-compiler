#include "utils.hpp"

namespace utils {

/**
 * @fn printTree
 * @param root Pointer to the root node of the AST.
 * @brief Prints the AST to the console.
 * @return void
 */
void printTree() {
    node::printTree(node::root, 0);
}

/**
 * @fn printIndent
 * @param times The number of times to print the indent.
 * @brief Prints an indent to the console for the AST.
 * @return std::string
 */
std::string printIndent(int times) {
    std::string result;
    for (int i = 0; i < times; ++i) {
        result += ".   ";
    }
    return result;
}

} // namespace utils

