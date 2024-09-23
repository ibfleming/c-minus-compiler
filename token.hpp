#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <iostream>

namespace token {

enum class TokenType {
    NUMCONST,
    EMPTY,
};

class Token {
public:
    Token(const std::string& lexeme) : _value(lexeme) {}
    void print() { std::cout << _value << std::endl; }
private:
    std::string _value;
    int _line;
    TokenType _type;
};

}

#endif