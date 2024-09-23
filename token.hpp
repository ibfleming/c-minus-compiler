#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <iostream>
#include <variant>

namespace token {

enum class TokenType {
    ID,
    NUMCONST,
    CHARCONST,
    STRINGCONST,
    BOOLCONST,
    EMPTY,
};

using TokenValue = std::variant<int, char, std::string, bool>;

TokenValue processLexeme(const std::string, TokenType type);
void lexicalPrint(const std::string& token, TokenValue value, int line, TokenType type);

class Token {
public:
    Token(const std::string& token, int line, TokenType type) 
    : token_(token), line_(line), type_(type) 
    {
        switch (type) {
            case TokenType::NUMCONST:
                value_ = std::stoi(token);
                break;
            case TokenType::CHARCONST:
                // Helper function for characters here.
                value_ = token;
                break;
            case TokenType::STRINGCONST:
                // Helper function for strings here
                value_ = token;
                break;
            case TokenType::BOOLCONST:
                // Helper function for bools here
                value_ = token;
                break;
            default:
                value_ = token;
                break;
        } 
    }

    void print() { lexicalPrint(token_, value_, line_, type_); }
    TokenValue getValue() const { return value_; }

private:
    TokenValue value_;
    TokenType type_;
    std::string token_; // Original lexeme that was read by lexer
    int line_;
};

} // namespace token

#endif