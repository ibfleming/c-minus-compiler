/*********************************************************************
 * @file semantic.cpp
 * 
 * @brief Source file for semantic analysis.
 *********************************************************************/

#include "semantic.hpp"

namespace semantic {

    void SemanticAnalyzer::printWarnings() {
        std::cout << "Number of warnings: " << warnings_ << std::endl;
        std::flush(std::cout);
    }

    void SemanticAnalyzer::printErrors() {
        std::cout << "Number of errors: " << errors_ << std::endl;
        std::flush(std::cout);
    }

} // namespace semantic