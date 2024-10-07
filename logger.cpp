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

void ERROR_CannotCallSimpleVariable(SA *analyzer, node::Node *sym) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << sym->getLine() << "): '" << sym->getString() << "' is a simple variable and cannot be called." << endl;
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
    cout << "ERROR(" << operand->getLine() << "): Unary '" << logger::loggerNodeTypeToStr(op) << "' requires an operand of type ";
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
        cout << "ERROR(" << operand->getLine() << "): '" << logger::loggerNodeTypeToStr(op) << "' requires operands of type bool but lhs is of type ";
    }
    else {
        cout << "ERROR(" << operand->getLine() << "): '" << logger::loggerNodeTypeToStr(op) << "' requires operands of type bool but rhs is of type ";
    }
    cout << types::varTypeToStr(operand->getVarType()) << "." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_RequiresOperandsAsArrayTypes(SA *analyzer, node::Node *op, node::Node *lhs, node::Node *rhs) {
    #if SEMANTIC_DEBUG
    if (lhs->getIsArray() && !rhs->getIsArray()) {
        cout << "ERROR(" << op->getLine() << "): '" << logger::loggerNodeTypeToStr(op) << "' requires boths operands be arrays but lhs is an array ";
        cout << "and rhs is not an array." << endl;
    }
    if (!lhs->getIsArray() && rhs->getIsArray()) {
        cout << "ERROR(" << op->getLine() << "): '" << logger::loggerNodeTypeToStr(op) << "' requires boths operands be arrays but lhs is not an array ";
        cout << "and rhs is an array." << endl;
    }
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

void ERROR_CannotReturnArray(SA *analyzer, node::Node *sym) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << sym->getLine() << "): Cannot return an array." << endl;
    #endif
    analyzer->incErrors();
}

void ERROR_OperationCannotUseArrays(SA *analyzer, node::Node *op, node::Node *operand) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << operand->getLine() << "): The operation '" << logger::loggerNodeTypeToStr(op) << "' does not work with arrays." << endl;
    #endif
    analyzer->incErrors();    
}

void ERROR_OperationWorksOnlyOnArrays(SA *analyzer, node::Node *op, node::Node *operand) {
    #if SEMANTIC_DEBUG
    cout << "ERROR(" << operand->getLine() << "): The operation '" << logger::loggerNodeTypeToStr(op) << "' only works with arrays." << endl;
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

/***********************************************
*  HELPER FUNCTIONS
***********************************************/

std::string loggerNodeTypeToStr(node::Node *sym) {
    switch (sym->getNodeType()) {
        case NT::BOOLEAN:                 return "boolean";
        case NT::CHARACTER:               return "character";
        case NT::NUMBER:                  return "number";
        case NT::STRING:                  return "string";
        case NT::VARIABLE:                return "variable";
        case NT::VARIABLE_ARRAY:          return "variable array";
        case NT::VARIABLE_STATIC:         return "variable static";
        case NT::VARIABLE_STATIC_ARRAY:   return "variable static array";
        case NT::PARAMETER:               return "parameter";
        case NT::PARAMETER_ARRAY:         return "parameter array";
        case NT::FUNCTION:                return "function";
        case NT::CALL:                    return "call";
        case NT::ID:                      return "identifier";
        case NT::ID_ARRAY:                return "array identifer";
        case NT::QUES_UNARY:              return "?";
        case NT::CHSIGN_UNARY:            return "chsign";
        case NT::SIZEOF_UNARY:            return "sizeof";
        case NT::OPERATOR:
        {
            switch(sym->getOpType()) {
                case OT::ADD:                return "+";
                case OT::SUB:                return "-";
                case OT::MUL:                return "*";
                case OT::DIV:                return "/";
                case OT::MOD:                return "%";
                default:                     return "operator";
            }
        }
        case NT::NOT:                     return "not";
        case NT::AND:                     return "and";
        case NT::OR:                      return "or";
        case NT::ASSIGNMENT:
        {
            switch(sym->getAsgnType()) {
                case AT::ASGN:               return ":=";
                case AT::ADDASGN:            return "+=";
                case AT::SUBASGN:            return "-=";
                case AT::MULASGN:            return "*=";
                case AT::DIVASGN:            return "/=";
                case AT::INC:                return "++";
                case AT::DEC:                return "--";
                default:                     return "assignment";
            }
        }
        case NT::BREAK:                   return "break";
        case NT::RANGE:                   return "range";
        case NT::FOR:                     return "for";
        case NT::WHILE:                   return "while";
        case NT::IF:                      return "if";
        case NT::COMPOUND:                return "compound";
        case NT::RETURN:                  return "return";
        case NT::UNKNOWN:                 return "unknown";
        default:                          return "invalid";
    }
}

} // namespace logger