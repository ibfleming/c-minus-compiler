#include "utils.hpp"
#include <iostream>

namespace utils {

void printTree() {
    node::printTree(node::root, 0);
}

std::string printIndent(int times) {
    std::string result;
    for (int i = 0; i < times; ++i) {
        result += ".   ";
    }
    return result;
}

bool checkFileExtension(std::string filename) {
    if (filename.find(".c-") == std::string::npos) { // check valid file type e.g. program.c-
        std::cerr << "Error: File name must contain '.c-'" << std::endl;
        return false;
    }
    return true;
}

} // namespace utils

