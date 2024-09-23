#include "token.hpp"

namespace token {

TokenValue processLexeme(const std::string, TokenType type) {
    return {};
}

void lexicalPrint(const std::string& token, TokenValue value, int line, TokenType type) {
    switch (type) {
        case TokenType::ID:
            std::cout << "Line " << line << " Token: ID Value: ";
            std::visit([](auto&& arg) {
                std::cout << arg;
            }, value);
            std::cout << std::endl;
            return;
        case TokenType::NUMCONST:
            std::cout << "Line " << line << " Token: NUMCONST Value: ";
            std::visit([](auto&& arg) {
                std::cout << arg;
            }, value);
            std::cout << "  Input: " << token << std::endl;
            return;
        case TokenType::CHARCONST:
            std::cout << "Line " << line << " Token: " << token << std::endl;
            return;
        case TokenType::STRINGCONST:
            std::cout << "Line " << line << " Token: " << token << std::endl;
            return;
        case TokenType::BOOLCONST:
            std::cout << "Line " << line << " Token: " << token << std::endl;
            return;
        default:
            std::cout << "Line " << line << " Token: " << token << std::endl;
            return;
    } 
}

} // namespace token
