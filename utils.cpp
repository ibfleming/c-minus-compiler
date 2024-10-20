#include "utils.hpp"
#include <iomanip>
#include <iostream>

namespace utils {

bool PRINT_TYPES = false;

void printTree()
{
    node::printTree(node::root, 0);
}

std::string printIndent(int times)
{
    std::string result;
    for (int i = 0; i < times; ++i) {
        result += ".   ";
    }
    return result;
}

bool checkFileExtension(std::string filename)
{
    if (filename.find(".c-") == std::string::npos) { // check valid file type e.g. program.c-
        std::cerr << "Error: File name must contain '.c-'" << std::endl;
        return false;
    }
    return true;
}

void printHelpMenu()
{
    std::cout << "usage: -c [options] [sourcefile]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "-d" << std::string(10, ' ') << "- turn on parser debugging" << std::endl;
    std::cout << "-D" << std::string(10, ' ') << "- turn on symbol table debugging" << std::endl;
    std::cout << "-h" << std::string(10, ' ') << "- print this usage message" << std::endl;
    std::cout << "-p" << std::string(10, ' ') << "- print the abstract syntax tree" << std::endl;
    std::cout << "-P" << std::string(10, ' ') << "- print the abstract syntax tree plus type information" << std::endl;
}

bool isArray(node::Node* node)
{
    switch (node->getNodeType()) {
    case types::NodeType::VARIABLE_ARRAY:
    case types::NodeType::VARIABLE_STATIC_ARRAY:
    case types::NodeType::PARAMETER_ARRAY:
        return true;
    }
    return false;
}

void printLine(node::Node* node)
{
    std::cout << std::endl;
    std::cout << "(" << node->getLine() << ") ";
}

} // namespace utils
