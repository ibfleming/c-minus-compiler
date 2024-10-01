#ifndef UTILS_HPP
#define UTILS_HPP

#include "node.hpp"

/**
 * @namespace utils
 * @brief Contains utility functions for the compiler.
 */
namespace utils {

    /**
    * @fn printTree
    * @param root Pointer to the root node of the AST.
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

} // namespace utils

#endif // UTILS_HPP