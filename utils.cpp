#include "utils.hpp"

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

} // namespace utils

