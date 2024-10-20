#include "token.hpp"
#include <iostream>

using namespace types;

namespace token {

int Token::getInt() const
{
    if (std::holds_alternative<int>(value_)) {
        return std::get<int>(value_);
    }
    throw std::bad_variant_access();
}

char Token::getChar() const
{
    if (std::holds_alternative<char>(value_)) {
        return std::get<char>(value_);
    }
    throw std::bad_variant_access();
}

std::string Token::getString() const
{
    if (std::holds_alternative<std::string>(value_)) {
        return std::get<std::string>(value_);
    }
    throw std::bad_variant_access();
}

int Token::getBool() const
{
    if (std::holds_alternative<int>(value_)) {
        return std::get<int>(value_);
    }
    throw std::bad_variant_access();
}

char processCharConst(Token& token)
{
    std::string lexeme = token.getToken();
    int length = token.getLength() - 2; // remove single quotes

    if (length > 1) { // ex: 'abc', '\0abc'
        if (lexeme[1] == '\\') { // if token is '\'
            switch (lexeme[2]) {
            case 'n':
                return '\n';
            case '0':
                return '\0';
            case '\\':
                return '\\';
            case '\'':
                return '\'';
            default:
                return lexeme[2];
            }
        }
        std::cout << "WARNING(" << token.getLine() << "): character is " << length
                  << " characters long and not a single character: '";
        std::cout << lexeme << "'.  The first char will be used." << std::endl;
        return lexeme[1]; // first char is not a '\'
    }

    if (length == 0) {
        std::cout << "WARNING(" << token.getLine() << "): character is empty: '";
        std::cout << lexeme << "'.  The first char will be used." << std::endl;
        return lexeme[1]; // empty char, might need to handle behavior differently
    }

    return lexeme[1]; // return single char by default
}

int processBoolConst(const std::string& token)
{
    if (token == "true" || token == "True") {
        return 1;
    } else {
        return 0;
    }
}

std::string processStringConst(Token& token)
{
    const std::string& lexeme = token.getToken();
    const int length = token.getLength();

    if (length == 2) { // ""
        token.setStrLength(0);
        return "";
    }

    std::string result;
    result.reserve(length - 2); // reserve space for the string w/o quotes

    for (size_t i = 1; i < length - 1; ++i) {
        if (lexeme[i] == '\\') {
            ++i;
            if (i < length - 1) {
                switch (lexeme[i]) {
                case 'n':
                    result += '\n';
                    break;
                case '0':
                    result += '\0';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case '\'':
                    result += '\'';
                    break;
                case '"':
                    result += '\"';
                    break;
                default:
                    result += lexeme[i];
                    break;
                }
            }
        } else {
            result += lexeme[i];
        }
    }

    token.setStrLength(result.length());
    return result; // returning the processed string into value_
}

void processToken(Token& token)
{
    switch (token.getType()) {
    case TokenType::ID_CONST:
        token.setValue(token.getToken());
        break;
    case TokenType::NUM_CONST:
        token.setValue(std::stoi(token.getToken()));
        break;
    case TokenType::CHAR_CONST:
        token.setValue(processCharConst(token));
        break;
    case TokenType::STRING_CONST:
        token.setValue(processStringConst(token));
        break;
    case TokenType::BOOL_CONST:
        token.setValue(processBoolConst(token.getToken()));
        break;
    default:
        token.setValue(token.getToken());
        break;
    }
}

void lexicalPrint(const Token& token)
{
    std::cout << "Line " << token.getLine() << " Token: ";
    switch (token.getType()) {
    case TokenType::ID_CONST:
        std::cout << token.printType() << " Value: " << token.getToken() << std::endl;
        break;
    case TokenType::NUM_CONST:
        std::cout << token.printType() << " Value: " << token.getInt();
        std::cout << "  Input: " << token.getToken() << std::endl;
        break;
    case TokenType::CHAR_CONST:
        std::cout << token.printType() << " Value: '" << token.getChar();
        std::cout << "'  Input: " << token.getToken() << std::endl;
        break;
    case TokenType::STRING_CONST:
        std::cout << token.printType() << " Value: \"" << token.getString();
        std::cout << "\"  Len: " << token.getStrLength() << "  Input: " << token.getToken() << std::endl;
        break;
    case TokenType::BOOL_CONST:
        std::cout << token.printType() << " Value: " << token.getBool();
        std::cout << "  Input: " << token.getToken() << std::endl;
        break;
    default:
        std::cout << token.printType() << std::endl;
        break;
    }
    return;
}

} // namespace token