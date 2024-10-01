#ifndef UTILS_HPP
#define UTILS_HPP

#include "node.hpp"

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

} // namespace utils

#endif // UTILS_HPP