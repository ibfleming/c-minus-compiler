/*********************************************************************
 * @file logger.hpp
 *
 * @brief Header file for logging.
 *********************************************************************/

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "node.hpp"
#include "semantic.hpp"
#include <iostream>

#define SEMANTIC_DEBUG true

namespace semantic
{
class SemanticAnalyzer;
}

typedef semantic::SemanticAnalyzer SA;
typedef types::NodeType NT;
typedef types::OperatorType OT;
typedef types::AssignmentType AT;

/**
 * @namespace logger
 * @brief Contains the logging functions for semantic analysis.
 * @param analyzer The semantic analyzer.
 */
namespace logger
{

enum class OperandType
{
    LHS,
    RHS
};

void ERROR_VariableNotDeclared(SA *analyzer, node::Node *sym);
void ERROR_VariableAlreadyDeclared(SA *analyzer, node::Node *sym, node::Node *decl);
void ERROR_VariableAsFunction(SA *analyzer, node::Node *sym);
void ERROR_CannotCallSimpleVariable(SA *analyzer, node::Node *sym);
void ERROR_Linker(SA *analyzer);

void ERROR_RequiresOperandsEqualTypes(SA *analyzer, node::Node *op, node::Node *lhs, node::Node *rhs);
void ERROR_UnaryRequiresOperandSameType(SA *analyzer, node::Node *op, node::Node *operand);
void ERROR_RequiresOperandIntTypes(SA *analyzer, node::Node *op, node::Node *operand, OperandType type);
void ERROR_RequiresOperandBoolTypes(SA *analyzer, node::Node *op, node::Node *operand, OperandType type);

void ERROR_RequiresOperandsAsArrayTypes(SA *analyzer, node::Node *op, node::Node *lhs, node::Node *rhs);
void ERROR_ArrayIndexNotInt(SA *analyzer, node::Node *arr, node::Node *index);
void ERROR_ArrayIndexIsUnindexedArray(SA *analyzer, node::Node *arr);
void ERROR_CannotIndexNonArray(SA *analyzer, node::Node *arr);
void ERROR_CannotReturnArray(SA *analyzer, node::Node *sym);
void ERROR_OperationCannotUseArrays(SA *analyzer, node::Node *op, node::Node *operand);
void ERROR_OperationWorksOnlyOnArrays(SA *analyzer, node::Node *op, node::Node *operand);

void WARN_VariableNotUsed(SA *analyzer, node::Node *sym);
void WARN_VariableNotInitialized(SA *analyzer, node::Node *sym);

std::string loggerNodeTypeToStr(node::Node *sym);

} // namespace logger

#endif // LOGGER_HPP