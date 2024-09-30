#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "types.hpp"
#include <string>
#include <variant>

namespace token {

class Token;

void processToken(Token& token);
void lexicalPrint(const Token& token);

class Token {

private:
    types::TokenType type_;         // Name of the token class (ID, ASGN, SUB, IF, FOR, etc.)
    types::TokenValue value_;       // Value of the token (int, char, string) after processing
    int strLength_;                 // Length of the processed STRINGCONST (only used for STRINGCONST   
    std::string token_;             // Original token (lexeme) that was read by lexer
    int length_;                    // Length of the token (lexeme)
    int line_;                      // Line number where the token was found

public:
    Token(const std::string& token, int line, int length, types::TokenType type) 
    : type_(type),
      strLength_(0),
      token_(token),
      length_(length),
      line_(line)
    {
        processToken(*this);
    }
    
    // Getters
    types::TokenValue getValue() const      { return value_; }
    types::TokenType getType() const        { return type_; }
    std::string getToken() const     { return token_; }
    int getLength() const            { return length_; }
    int getLine() const              { return line_; }
    int getStrLength() const         { return strLength_; }

    // Setters
    void setValue(types::TokenValue value)  { value_ = value; }
    void setStrLength(int length)    { strLength_ = length; }

    // TokenValue Getters (int, char, string)
    int getInt() const {
        if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        throw std::bad_variant_access();
    }
    char getChar() const {
        if (std::holds_alternative<char>(value_)) {
            return std::get<char>(value_);
        }
        throw std::bad_variant_access();
    }
    std::string getString() const {
        if (std::holds_alternative<std::string>(value_)) {
            return std::get<std::string>(value_);
        }
        throw std::bad_variant_access();
    }
    int getBool() const {
        if (std::holds_alternative<int>(value_)) {
            return std::get<int>(value_);
        }
        throw std::bad_variant_access();
    } // 0 = false, 1 = true (compiler specs say boolean values are 0 or 1 integers)

    // Print
    void print() { lexicalPrint(*this); }
    std::string printType() const { return types::tknTypeToStr(type_); } 
};

} // namespace token

#endif // TOKEN_HPP