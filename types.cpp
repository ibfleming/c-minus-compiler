#include "types.hpp"

namespace types {

std::string tknTypeToStr(TokenType type)
{
    switch (type) {

    // CONSTANTS
    case TokenType::ID_CONST:
        return "ID";
    case TokenType::NUM_CONST:
        return "NUMCONST";
    case TokenType::CHAR_CONST:
        return "CHARCONST";
    case TokenType::STRING_CONST:
        return "STRINGCONST";
    case TokenType::BOOL_CONST:
        return "BOOLCONST";

    // TYPES
    case TokenType::INT_TYPE:
        return "INT";
    case TokenType::CHAR_TYPE:
        return "CHAR";
    case TokenType::BOOL_TYPE:
        return "BOOL";
    case TokenType::STATIC_TYPE:
        return "STATIC";

    // CONTROL
    case TokenType::IF_CONTROL:
        return "IF";
    case TokenType::THEN_CONTROL:
        return "THEN";
    case TokenType::ELSE_CONTROL:
        return "ELSE";

    // LOOPS (ITERATION)
    case TokenType::FOR_LOOP:
        return "FOR";
    case TokenType::TO_LOOP:
        return "TO";
    case TokenType::BY_LOOP:
        return "BY";
    case TokenType::DO_LOOP:
        return "DO";
    case TokenType::WHILE_LOOP:
        return "WHILE";
    case TokenType::BREAK_LOOP:
        return "BREAK";

    // BINARY OPERATORS
    case TokenType::AND_OP:
        return "AND";
    case TokenType::OR_OP:
        return "OR";
    case TokenType::EQL_OP:
        return "EQL";
    case TokenType::NEQ_OP:
        return "NEQ";
    case TokenType::LESS_OP:
        return "<";
    case TokenType::LEQ_OP:
        return "LEQ";
    case TokenType::GREATER_OP:
        return ">";
    case TokenType::GEQ_OP:
        return "GEQ";
    case TokenType::ASGN_OP:
        return "ASGN";
    case TokenType::ADDASGN_OP:
        return "ADDASS";
    case TokenType::SUBASGN_OP:
        return "SUBASS";
    case TokenType::MULASGN_OP:
        return "MULASS";
    case TokenType::DIVASGN_OP:
        return "DIVASS";
    case TokenType::ADD_OP:
        return "+";
    case TokenType::SUB_OP:
        return "-";
    case TokenType::MUL_OP:
        return "*";
    case TokenType::DIV_OP:
        return "/";
    case TokenType::MOD_OP:
        return "%";

    // UNARY OPERATORS
    case TokenType::DEC_OP:
        return "--";
    case TokenType::INC_OP:
        return "++";
    case TokenType::NOT_OP:
        return "NOT";
    case TokenType::QUES_OP:
        return "?";

    // COMPOUND
    case TokenType::LBRACE:
        return "{";
    case TokenType::RBRACE:
        return "}";

    // RETURN
    case TokenType::RETURN:
        return "RETURN";
    default:
        return "UNKNOWN";
    }
}

std::string treeNodeTypeToStr(NodeType type)
{
    switch (type) {
    case NodeType::BOOLEAN:
    case NodeType::CHARACTER:
    case NodeType::NUMBER:
    case NodeType::STRING:
        return "Const";

    case NodeType::VARIABLE:
    case NodeType::VARIABLE_ARRAY:
    case NodeType::VARIABLE_STATIC:
    case NodeType::VARIABLE_STATIC_ARRAY:
        return "Var: ";

    case NodeType::PARAMETER:
    case NodeType::PARAMETER_ARRAY:
        return "Parm: ";

    case NodeType::FUNCTION:
        return "Func: ";
    case NodeType::CALL:
        return "Call: ";
    case NodeType::ID:
        return "Id: ";
    case NodeType::ID_ARRAY:
        return "Op: [";
    case NodeType::QUES_UNARY:
        return "Op: ?";
    case NodeType::CHSIGN_UNARY:
        return "Op: chsign";
    case NodeType::SIZEOF_UNARY:
        return "Op: sizeof";
    case NodeType::OPERATOR:
        return "Op: ";
    case NodeType::NOT:
        return "Op: not";
    case NodeType::AND:
        return "Op: and";
    case NodeType::OR:
        return "Op: or";
    case NodeType::ASSIGNMENT:
        return "Assign: ";
    case NodeType::BREAK:
        return "Break";
    case NodeType::RANGE:
        return "Range";
    case NodeType::FOR:
        return "For";
    case NodeType::WHILE:
        return "While";
    case NodeType::IF:
        return "If";
    case NodeType::COMPOUND:
        return "Compound";
    case NodeType::RETURN:
        return "Return";
    case NodeType::UNKNOWN:
        return "Unknown";
    default:
        return "Invalid Node Type";
    }
}

std::string literalNodeTypeStr(NodeType type)
{
    switch (type) {
    case NodeType::BOOLEAN:
        return "BOOLEAN";
    case NodeType::CHARACTER:
        return "CHARACTER";
    case NodeType::NUMBER:
        return "NUMBER";
    case NodeType::STRING:
        return "STRING";
    case NodeType::VARIABLE:
        return "VARIABLE";
    case NodeType::VARIABLE_ARRAY:
        return "VARIABLE_ARRAY";
    case NodeType::VARIABLE_STATIC:
        return "VARIABLE_STATIC";
    case NodeType::VARIABLE_STATIC_ARRAY:
        return "VARIABLE_STATIC_ARRAY";
    case NodeType::PARAMETER:
        return "PARAMETER";
    case NodeType::PARAMETER_ARRAY:
        return "PARAMETER_ARRAY";
    case NodeType::FUNCTION:
        return "FUNCTION";
    case NodeType::CALL:
        return "CALL";
    case NodeType::ID:
        return "ID";
    case NodeType::ID_ARRAY:
        return "ID_ARRAY";
    case NodeType::QUES_UNARY:
        return "QUES_UNARY";
    case NodeType::CHSIGN_UNARY:
        return "CHSIGN_UNARY";
    case NodeType::SIZEOF_UNARY:
        return "SIZEOF_UNARY";
    case NodeType::OPERATOR:
        return "OPERATOR";
    case NodeType::NOT:
        return "NOT";
    case NodeType::AND:
        return "AND";
    case NodeType::OR:
        return "OR";
    case NodeType::ASSIGNMENT:
        return "ASSIGNMENT";
    case NodeType::BREAK:
        return "BREAK";
    case NodeType::RANGE:
        return "RANGE";
    case NodeType::FOR:
        return "FOR";
    case NodeType::WHILE:
        return "WHILE";
    case NodeType::IF:
        return "IF";
    case NodeType::COMPOUND:
        return "COMPOUND";
    case NodeType::RETURN:
        return "RETURN";
    case NodeType::UNKNOWN:
        return "UNKNOWN";
    default:
        return "INVALID";
    }
}

std::string varTypeToStr(VarType type)
{
    switch (type) {
    case VarType::INT:
        return "int";
    case VarType::CHAR:
        return "char";
    case VarType::STRING:
        return "char";
    case VarType::BOOL:
        return "bool";
    case VarType::VOID:
        return "void";
    case VarType::UNDEFINED:
        return "undefined";
    default:
        return "Invalid Var Type";
    }
}

} // namespace types