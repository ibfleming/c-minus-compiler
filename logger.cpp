/*********************************************************************
 * @file logger.cpp
 * 
 * @brief Source file for logging.
 *********************************************************************/

#include "logger.hpp"

using namespace std;

namespace logger {

/***********************************************
*  ERRORS
***********************************************/

void ERROR_VariableNotDeclared(SA *analyzer, node::Node *sym) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << sym->getLine() << "): Symbol '" << sym->getString() << "' is not declared." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_VariableAlreadyDeclared(SA *analyzer, node::Node *sym, node::Node *decl) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << sym->getLine() << "): Symbol '" << sym->getString() << "' is already declared at line ";
    cout << decl->getLine() << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_VariableAsFunction(SA *analyzer, node::Node *sym) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << sym->getLine() << "): Cannot use function '" << sym->getString()  <<"' as a variable." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_Linker(SA *analyzer) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(LINKER): A function named 'main()' must be defined." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_RequiresOperandsEqualTypes(SA *analyzer, node::Node *op, node::Node *lhs, node::Node *rhs) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << lhs->getLine() << "): '" << op->getString() << "' requires operands of the same type but lhs is type ";
    cout << types::varTypeToStr(lhs->getVarType()) << " and rhs is type " << types::varTypeToStr(rhs->getVarType()) << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_UnaryRequiresOperandSameType(SA *analyzer, node::Node *op, node::Node *operand) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << operand->getLine() << "): Unary '" << types::displayNodeTypeToStr(op->getNodeType()) << "' requires an operand of type ";
    cout << types::varTypeToStr(op->getVarType()) << " but was given type " << types::varTypeToStr(operand->getVarType()) << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_RequiresOperandIntTypes(SA *analyzer, node::Node *op, node::Node *operand, OperandType type) {
    #if SEMANTIC_DEBUG
    if (type == OperandType::LHS) {
        cout << "ERROR(" << operand->getLine() << "): '" << op->getString() << "' requires operands of type int but lhs is of type ";
    }
    else {
        cout << "ERROR(" << operand->getLine() << "): '" << op->getString() << "' requires operands of type int but rhs is of type ";
    }
    cout << types::varTypeToStr(operand->getVarType()) << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_RequiresOperandBoolTypes(SA *analyzer, node::Node *op, node::Node *operand, OperandType type) {
    #if SEMANTIC_DEBUG
    if (type == OperandType::LHS) {
        cout << "ERROR(" << operand->getLine() << "): '" << types::displayNodeTypeToStr(op->getNodeType()) << "' requires operands of type bool but lhs is of type ";
    }
    else {
        cout << "ERROR(" << operand->getLine() << "): '" << types::displayNodeTypeToStr(op->getNodeType()) << "' requires operands of type bool but rhs is of type ";
    }
    cout << types::varTypeToStr(operand->getVarType()) << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_ArrayIndexNotInt(SA *analyzer, node::Node *arr, node::Node *index) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << index->getLine() << "): Array '" << arr->getString() << "' should be indexed by type int but got type ";
    cout << types::varTypeToStr(index->getVarType()) << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_ArrayIndexIsUnindexedArray(SA *analyzer, node::Node *arr) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << arr->getLine() << "): Array index is the unindexed array '" << arr->getString() << "'." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_CannotIndexNonArray(SA *analyzer, node::Node *arr) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << arr->getLine() << "): Cannot index nonarray '" << arr->getString() << "'." << endl;
    #endif
    analyzer->incErrors();
}

/***********************************************
*  WARNINGS
***********************************************/

void WARN_VariableNotUsed(SA *analyzer, node::Node *sym) {
    #if SEMANTIC_DEBUG
    cout << "WARNING(" << sym->getLine() << "): The variable '" << sym->getString() << "' seems not to be used." << endl;
    #endif
    analyzer->incWarnings();
}

void WARN_VariableNotInitialized(SA *analyzer, node::Node *sym) {
    #if SEMANTIC_DEBUG
    cout << "WARNING(" << sym->getLine() << "): Variable '" << sym->getString() << "' may be uninitialized when used here." << endl;
    #endif
    analyzer->incWarnings();
}

} // namespace logger