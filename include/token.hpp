#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <iostream>
#include <variant>

namespace token {

class Token;

enum class TokenType {
    ID, NUMCONST, CHARCONST, STRINGCONST, BOOLCONST,
    INT, CHAR, BOOL, STATIC,
    IF, THEN, ELSE,
    FOR, TO, BY, DO, WHILE, BREAK,
    ASGN, ADDASS, INC, DEC, GEQ, LEQ, NEQ,
    AND, OR, NOT, RETURN,
    EMPTY,
};

using TokenValue = std::variant<int, char, std::string>;

void processLexeme(Token& token);
void lexicalPrint(const Token& token);

class Token {
public:
    Token(const std::string& token, int line, int length, TokenType type) 
    : token_(token), line_(line), length_(length), type_(type), strLength_(0)
    {
        processLexeme(*this);
    }

    void print() {  lexicalPrint(*this); }
    
    // Getters
    TokenValue getValue() const { return value_; }
    TokenType getType() const { return type_; }
    std::string getToken() const { return token_; }
    int getLength() const { return length_; }
    int getLine() const { return line_; }
    int getStringLength() const { return strLength_; }

    // TokenValue Getters (int, char, string, bool?)
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
    
    // Setters
    void setValue(TokenValue value) { value_ = value; }
    void setStringLength(int length) { strLength_ = length; }    
private:
    TokenValue value_;
    TokenType type_;
    std::string token_; // Original token (lexeme) that was read by lexer
    int length_;        // Length of the token (lexeme)
    int strLength_;     // Length of the processed STRINGCONST (only used for STRINGCONST)
    int line_;
};

} // namespace token

#endif // TOKEN_HPP