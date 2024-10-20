#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "types.hpp"
#include <string>
#include <variant>

/**
 * @namespace token
 * @brief Contains the Token class and related functions.
 */
namespace token {

class Token; // forward declaration

/**
 * @fn processCharConst
 * @param token The character token to process.
 * @return char
 * @brief Processes the character token and returns the character.
 */
char processCharConst(Token& token);

/**
 * @fn processBoolConst
 * @param token The boolean token to process.
 * @return int
 * @brief Processes the boolean token and returns the integer value.
 */
int processBoolConst(const std::string& token);

/**
 * @fn processStringConst
 * @param token The string token to process.
 * @return string
 * @brief Processes the string token and returns the string.
 */
std::string processStringConst(Token& token);

/**
 * @fn processToken
 * @param token The token to process.
 * @return void
 * @brief Processes the token and sets the value based on the token type.
 */
void processToken(Token& token);

/**
 * @fn lexicalPrint
 * @param token The token to print.
 * @return void
 * @brief Prints the token to the console (used in hw1).
 */
void lexicalPrint(const Token& token);

/**
 * @class Token
 * @brief Represents a token (lexeme) that was read by the lexer.
 */
class Token {

private:
    types::TokenType type_; // Name of the token class (ID, ASGN, SUB, IF, FOR, etc.)
    types::TokenValue value_; // Value of the token (int, char, string) after processing
    int strLength_; // Length of the processed STRINGCONST (only used for STRINGCONST
    std::string token_; // Original token (lexeme) that was read by lexer
    int length_; // Length of the token (lexeme)
    int line_; // Line number where the token was found

public:
    /**
     * @fn Token
     * @param token The token (lexeme) that was read by the lexer.
     * @param line The line number where the token was found.
     * @param length The length of the token (lexeme).
     * @param type The name of the token class (ID, ASGN, SUB, IF, FOR, etc.).
     * @brief Constructor for the Token class.
     */
    Token(const std::string& token, int line, int length, types::TokenType type)
        : type_(type)
        , strLength_(0)
        , token_(token)
        , length_(length)
        , line_(line)
    {
        processToken(*this);
    }

    // Getters

    types::TokenValue getValue() const
    {
        return value_;
    }
    types::TokenType getType() const
    {
        return type_;
    }
    std::string getToken() const
    {
        return token_;
    }
    int getLength() const
    {
        return length_;
    }
    int getLine() const
    {
        return line_;
    }
    int getStrLength() const
    {
        return strLength_;
    }

    // Setters

    void setValue(types::TokenValue value)
    {
        value_ = value;
    }
    void setStrLength(int length)
    {
        strLength_ = length;
    }

    // TokenValue Getters (int, char, string)

    /**
     * @fn getInt
     * @brief Returns integer variant of the value.
     * @return int
     */
    int getInt() const;

    /**
     * @fn getChar
     * @brief Returns character variant of the value.
     * @return char
     */
    char getChar() const;

    /**
     * @fn getString
     * @brief Returns string variant of the value.
     * @return std::string
     */
    std::string getString() const;

    /**
     * @fn getBool
     * @brief Returns boolean variant of the value.
     * @return int
     */
    int getBool() const;

    /**
     * @fn print
     * @brief Prints the token to the console using lexcalPrint().
     */
    void print()
    {
        lexicalPrint(*this);
    }

    /**
     * @fn printType
     * @brief Returns the name of the token class (ID, ASGN, SUB, IF, FOR, etc.).
     * @return std::string
     */
    std::string printType() const
    {
        return types::tknTypeToStr(type_);
    }
};

} // namespace token

#endif // TOKEN_HPP