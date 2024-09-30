#include "types.hpp"

namespace types {

std::string tknTypeToStr(TokenType type) {
    switch (type) {
        case TokenType::ID_CONST:       return "ID";
        case TokenType::NUM_CONST:      return "NUMCONST";
        case TokenType::CHAR_CONST:     return "CHARCONST";
        case TokenType::STRING_CONST:   return "STRINGCONST";
        case TokenType::BOOL_CONST:     return "BOOLCONST";
        case TokenType::INT_TYPE:       return "INT";
        case TokenType::CHAR_TYPE:      return "CHAR";
        case TokenType::BOOL_TYPE:      return "BOOL";
        case TokenType::STATIC_TYPE:    return "STATIC";
        case TokenType::IF_CONTROL:     return "IF";
        case TokenType::THEN_CONTROL:   return "THEN";
        case TokenType::ELSE_CONTROL:   return "ELSE";
        case TokenType::FOR_LOOP:       return "FOR";
        case TokenType::TO_LOOP:        return "TO";
        case TokenType::BY_LOOP:        return "BY";
        case TokenType::DO_LOOP:        return "DO";
        case TokenType::WHILE_LOOP:     return "WHILE";
        case TokenType::BREAK_LOOP:     return "BREAK";
        case TokenType::AND_OP:         return "AND";
        case TokenType::OR_OP:          return "OR";
        case TokenType::EQL_OP:         return "EQL";
        case TokenType::NEQ_OP:         return "NEQ";
        case TokenType::LESS_OP:        return "<";
        case TokenType::LEQ_OP:         return "LEQ";
        case TokenType::GREATER_OP:     return ">";
        case TokenType::GEQ_OP:         return "GEQ";
        case TokenType::ASGN_OP:        return "ASGN";
        case TokenType::ADDASGN_OP:     return "ADDASS";
        case TokenType::SUBASGN_OP:     return "SUBASS";
        case TokenType::MULASGN_OP:     return "MULASS";
        case TokenType::DIVASGN_OP:     return "DIVASS";
        case TokenType::ADD_OP:         return "+";
        case TokenType::SUB_OP:         return "-";
        case TokenType::MUL_OP:         return "*";
        case TokenType::DIV_OP:         return "/";
        case TokenType::MOD_OP:         return "%";
        case TokenType::DEC_OP:         return "--";
        case TokenType::INC_OP:         return "++";
        case TokenType::NOT_OP:         return "NOT";
        case TokenType::QUES_OP:        return "?";
        case TokenType::RETURN:         return "RETURN";
        default:                        return "UNKNOWN";
    }
}

std::string nodeTypeToStr(NodeType type) {
    switch (type) {
        case NodeType::FUNCTION:            return "Func";
        case NodeType::CHARCONST:           return "Const";
        case NodeType::BOOLCONST:           return "Const";
        case NodeType::NUMCONST:            return "Const";
        case NodeType::STRINGCONST:         return "Const";
        case NodeType::CALL:                return "Call";
        case NodeType::ID:                  return "Id";
        case NodeType::ARRAY:               return "Op: [";
        case NodeType::QUES_UNARY:          return "Op: ?";
        case NodeType::CHSIGN_UNARY:        return "Op: chsign";
        case NodeType::SIZEOF_UNARY:        return "Op: sizeof";
        case NodeType::OPERATOR:            return "Op";
        case NodeType::NOT:                 return "Op: not";
        case NodeType::AND:                 return "Op: and";
        case NodeType::OR:                  return "Op: or";
        case NodeType::ASSIGNMENT:          return "Assign";
        case NodeType::BREAK:               return "Break";
        case NodeType::RANGE:               return "Range";
        case NodeType::FOR:                 return "For";
        case NodeType::WHILE:               return "While";
        case NodeType::VARIABLE:            return "Var";
        case NodeType::VARIABLE_ARRAY:      return "Var"; // Var Array...
        case NodeType::IF:                  return "If";
        case NodeType::COMPOUND:            return "Compound";
        case NodeType::PARAMETER:           return "Parm";
        case NodeType::PARAMETER_ARRAY:     return "Parm";
        case NodeType::STATIC_VARIABLE:     return "Var";
        case NodeType::RETURN:              return "Return";
        case NodeType::UNKNOWN:             return "UNKNOWN";
        default:                            return "INVALID_NODE_TYPE";
    }
}

std::string varTypeToStr(VarType type) {
    switch (type) {
        case VarType::INT:      return "INT";
        case VarType::CHAR:     return "CHAR";
        case VarType::STRING:   return "STRING";
        case VarType::BOOL:     return "BOOL";
        case VarType::STATIC:   return "STATIC";
        case VarType::VOID:     return "VOID";
        default:                return "UNKNOWN";
    }
}

} // namespace types