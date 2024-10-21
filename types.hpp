#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <variant>

/**
 * @brief Contains the various types used in the compiler.
 */
namespace types {

/**
 * @brief Represents the value of a token.
 * @note A token can have a value of type int, char, or string.
 */
using TokenValue = std::variant<int, char, std::string>;

/**
 * @brief Represents the various types of tokens that can be encountered in the compiler.
 *
 * @note These are assigned in the "lexical analysis phase".
 * This enumeration defines the different categories of tokens that the compiler can recognize.
 * Tokens are categorized into constants, types, control structures, loops, binary operators,
 * unary operators, and return statements.
 */
enum class TokenType {

    // CONSTANTS

    ID_CONST,
    NUM_CONST,
    CHAR_CONST,
    STRING_CONST,
    BOOL_CONST,

    // TYPES

    INT_TYPE,
    CHAR_TYPE,
    BOOL_TYPE,
    STATIC_TYPE,

    // CONTROL

    IF_CONTROL,
    THEN_CONTROL,
    ELSE_CONTROL,

    // LOOPS (ITERATION)

    FOR_LOOP,
    TO_LOOP,
    BY_LOOP,
    DO_LOOP,
    WHILE_LOOP,
    BREAK_LOOP,

    // BINARY OPERATORS

    AND_OP,
    OR_OP,
    EQL_OP,
    NEQ_OP,
    LESS_OP,
    LEQ_OP,
    GREATER_OP,
    GEQ_OP,
    ASGN_OP, // assignment 'x := 0' => 'x = 0' OR 'x is 0'
    ADDASGN_OP, // assignment
    SUBASGN_OP, // assignment
    MULASGN_OP, // assignment
    DIVASGN_OP, // assignment
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    MOD_OP,

    // UNARY OPERATORS

    DEC_OP, // -- = are assignment operators
    INC_OP, // ++ = are assignment operators
    NOT_OP,
    // MUL_OP -- defined already but maybe differentiated
    // SUB_OP -- defined already but maybe differentiated
    QUES_OP,

    // COMPOUND

    LBRACE,
    RBRACE,

    // RETURN

    RETURN,

    // UNKNOWN

    UNKNOWN
};

/**
 * @brief Represents the various variable types that can be encountered in the compiler.
 * @note Some nodes in the AST will have a variable type associated with them directly or indirectly from a token.
 */
enum class VarType {
    INT,
    CHAR,
    STRING,
    BOOL,
    VOID,
    UNDEFINED
};

/**
 * @brief Represents the node type in the AST.
 */
enum class NodeType {

    // Data Types
    BOOLEAN,
    CHARACTER,
    NUMBER,
    STRING,

    // Expressions
    FUNCTION,
    CALL,
    ID,
    ID_ARRAY,
    QUES_UNARY,
    CHSIGN_UNARY,
    SIZEOF_UNARY,
    OPERATOR,
    NOT,
    AND,
    OR,

    // Control Flow
    BREAK,
    FOR,
    WHILE,
    IF,
    COMPOUND,
    RETURN,

    // Variables
    VARIABLE,
    VARIABLE_ARRAY,
    PARAMETER,
    PARAMETER_ARRAY,
    VARIABLE_STATIC,
    VARIABLE_STATIC_ARRAY,

    // Assignment
    ASSIGNMENT,

    // Loops (Iteration)
    RANGE,

    // Misc
    UNKNOWN
};

/**
 * @brief Represents the various types of operators that can be encountered in the compiler.
 */
enum class OperatorType {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LESS,
    LEQ,
    GREATER,
    GEQ,
    EQL,
    NEQ,
    UNKNOWN
};

/**
 * @brief Represents the various types of assignments that can be encountered in the compiler.
 */
enum class AssignmentType {
    ASGN,
    ADDASGN,
    SUBASGN,
    MULASGN,
    DIVASGN,
    INC,
    DEC,
    UNKNOWN
};

/**
 * @param type The type of token to convert to a string.
 * @brief Converts token type to a printable string.
 */
std::string tknTypeToStr(TokenType type);

/**
 * @param type The type of node to convert to a string.
 * @brief Converts node type to a printable string for the AST.
 */
std::string treeNodeTypeToStr(NodeType type);

/**
 * @param type The type of node to convert to a string.
 * @brief Converts node type to a printable string as the literal enum name.
 */
std::string literalNodeTypeStr(NodeType type);

/**
 * @param type The type of node to convert to a string.
 * @brief Converts node type to a printable string.
 */
std::string varTypeToStr(VarType type);

} // namespace types

#endif // TYPES_HPP