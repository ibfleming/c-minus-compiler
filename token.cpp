#include "token.hpp"


std::string typeToString(token::TokenType type) {
    switch (type) {
        case token::TokenType::ID:              return "ID";
        case token::TokenType::NUMCONST:        return "NUMCONST";
        case token::TokenType::CHARCONST:       return "CHARCONST";
        case token::TokenType::STRINGCONST:     return "STRINGCONST";
        case token::TokenType::BOOLCONST:       return "BOOLCONST";
        case token::TokenType::ADDASS:          return "ADDASS";
        case token::TokenType::RETURN:          return "RETURN";
        case token::TokenType::IF:              return "IF";
        case token::TokenType::INT:             return "INT";
        case token::TokenType::BOOL:            return "BOOL";
        case token::TokenType::CHAR:            return "CHAR";
        case token::TokenType::STATIC:          return "STATIC";
        case token::TokenType::EMPTY:           return "EMPTY";
        default:                                return "UNKNOWN";
    }
}

char processCharConst(const std::string& token, int length) {
    if (length > 3) { // ex: 'abc', '\0abc' 
        if (token[1] == '\\') {  // if token is '\'
            switch (token[2]) {
                case 'n':
                    return '\n';
                case '0':
                    return '\0';
                case '\\':
                    return '\\';
                case '\'':
                    return '\'';
                default:
                    return token[1];
            }
        }
        return token[1]; // first char is not a '\'
    }
    return token[1]; // return single char by default
}

bool processBoolConst(const std::string& token) {
    if (token == "true" || token == "True") {
        return true;
    } else {
        return false;
    }
}

std::string processStringConst(token::Token& token) {
    // token = "abc" || "a" || "a\nb" || "a\\b" || "a\"b" || "a\0b"

    const std::string& lexeme = token.getToken();
    const int length = token.getLength();

    if (length == 2) { // ""
        token.setStringLength(0);
        return "";
    }

    std::string result;
    result.reserve(length - 2); // reserve space for the string w/o quotes

    for (size_t i = 1; i < length - 1; ++i) {
        if (lexeme[i] == '\\') {
            ++i;
            if( i < length - 1 ) {
                switch (lexeme[i]) {
                    case 'n':  result += '\n'; break;
                    case '0':  result += '\0'; break;
                    case '\\': result += '\\'; break;
                    case '\'': result += '\''; break;
                    case '"':  result += '\"'; break;
                    default:   result += lexeme[i]; break;
                }
            } 
        }
        else {
            result += lexeme[i];
        }
    }

    token.setStringLength(result.length());
    return result; // returning the processed string into value_
}

namespace token {

void processLexeme(Token& token) {
    switch (token.getType()) {
        case TokenType::ID:
            token.setValue(token.getToken());
            break;
        case TokenType::NUMCONST:
            token.setValue(std::stoi(token.getToken()));
            break;
        case TokenType::CHARCONST:
            token.setValue(processCharConst(token.getToken(), token.getLength()));
            break;
        case TokenType::STRINGCONST:
            token.setValue(processStringConst(token));
            break;
        case TokenType::BOOLCONST:
            token.setValue(processBoolConst(token.getToken()));  
            break;
        default:
            token.setValue(token.getToken());
            break;
    }
}

void lexicalPrint(const Token& token) {
    std::cout << "Line " << token.getLine() << " Token: ";
    switch (token.getType()) {
        case TokenType::ID:
            std::cout << typeToString(token.getType()) << " Value: " << token.getToken() << std::endl;
            break;
        case TokenType::NUMCONST:
            std::cout << typeToString(token.getType()) << " Value: " << token.getInt();
            std::cout << "  Input: " << token.getToken() << std::endl;
            break;
        case TokenType::CHARCONST:
            std::cout << typeToString(token.getType()) << " Value: '" << token.getChar(); 
            std::cout << "'  Input: " << token.getToken() << std::endl;
            break;
        case TokenType::STRINGCONST:
            std::cout << typeToString(token.getType()) << " Value: \"" << token.getString(); 
            std::cout << "\"  Len: " << token.getStringLength() << "  Input: " << token.getToken() << std::endl;
            break;
        case TokenType::BOOLCONST:
            std::cout << typeToString(token.getType()) << " Value: " << std::boolalpha << token.getBool(); 
            std::cout << "  Input: " << token.getToken() << std::endl;
            break;
/*         case TokenType::ADDASS:
            std::cout << typeToString(token.getType()) << std::endl;
            break; */
        default:
            std::cout << typeToString(token.getType()) << std::endl;
            break;
    } 
    return;
}

} // namespace token
