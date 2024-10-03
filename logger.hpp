/*********************************************************************
 * @file logger.hpp
 * 
 * @brief Header file for logging.
 *********************************************************************/

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "semantic.hpp"
#include "node.hpp"
#include <iostream>

namespace semantic {
    class SemanticAnalyzer;
}

typedef semantic::SemanticAnalyzer SA;

namespace logger {

void ERROR_VariableNotDeclared(SA *analyzer, node::Node *sym);
void ERROR_VariableAlreadyDeclared(SA *analyzer, node::Node *sym, node::Node *decl);

} // namespace logger

#endif // LOGGER_HPP