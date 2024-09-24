#include <cstdio>
#include "../include/token.hpp"

namespace token {

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::ID:              return "ID";
        case TokenType::NUMCONST:        return "NUMCONST";
        case TokenType::CHARCONST:       return "CHARCONST";
        case TokenType::STRINGCONST:     return "STRINGCONST";
        case TokenType::BOOLCONST:       return "BOOLCONST";
        case TokenType::INT:             return "INT";
        case TokenType::BOOL:            return "BOOL";
        case TokenType::CHAR:            return "CHAR";
        case TokenType::STATIC:          return "STATIC";
        case TokenType::IF:              return "IF";
        case TokenType::THEN:            return "THEN";
        case TokenType::ELSE:            return "ELSE";
        case TokenType::FOR:             return "FOR";
        case TokenType::TO:              return "TO";
        case TokenType::BY:              return "BY";
        case TokenType::DO:              return "DO";
        case TokenType::WHILE:           return "WHILE";
        case TokenType::BREAK:           return "BREAK";
        case TokenType::ASGN:            return "ASGN";
        case TokenType::ADDASS:          return "ADDASS";
        case TokenType::INC:             return "INC";
        case TokenType::DEC:             return "DEC";
        case TokenType::GEQ:             return "GEQ";
        case TokenType::LEQ:             return "LEQ";
        case TokenType::NEQ:             return "NEQ";
        case TokenType::AND:             return "AND";
        case TokenType::OR:              return "OR";
        case TokenType::NOT:             return "NOT";
        case TokenType::RETURN:          return "RETURN";
        case TokenType::EMPTY:           return "EMPTY";
        default:                         return "UNKNOWN";
    }
}

char processCharConst(Token& token) {
    std::string lexeme = token.getToken();
    int length = token.getLength() - 2; // remove single quotes

    if (length > 1) { // ex: 'abc', '\0abc' 
        if (lexeme[1] == '\\') {  // if token is '\'
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
        std::cout << "WARNING(" << token.getLine() << "): character is " << length << " characters long and not a single character: '"; 
        std::cout << lexeme << "'.  The first char will be used." << std::endl;
        return lexeme[1]; // first char is not a '\'
    }

    if ( length == 0 ) {
        std::cout << "WARNING(" << token.getLine() << "): character is empty: '"; 
        std::cout << lexeme << "'.  The first char will be used." << std::endl;
        return lexeme[1]; // empty char, might need to handle behavior differently
    }

    return lexeme[1]; // return single char by default
}

int processBoolConst(const std::string& token) {
    if (token == "true" || token == "True") {
        return 1;
    } else {
        return 0;
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

void processLexeme(Token& token) {
    switch (token.getType()) {
        case TokenType::ID:
            token.setValue(token.getToken());
            break;
        case TokenType::NUMCONST:
            token.setValue(std::stoi(token.getToken()));
            break;
        case TokenType::CHARCONST:
            token.setValue(processCharConst(token));
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
            std::cout << tokenTypeToString(token.getType()) << " Value: " << token.getToken() << std::endl;
            break;
        case TokenType::NUMCONST:
            std::cout << tokenTypeToString(token.getType()) << " Value: " << token.getInt();
            std::cout << "  Input: " << token.getToken() << std::endl;
            break;
        case TokenType::CHARCONST:
            std::cout << tokenTypeToString(token.getType()) << " Value: '" << token.getChar(); 
            std::cout << "'  Input: " << token.getToken() << std::endl;
            break;
        case TokenType::STRINGCONST:
            std::cout << tokenTypeToString(token.getType()) << " Value: \"" << token.getString(); 
            std::cout << "\"  Len: " << token.getStringLength() << "  Input: " << token.getToken() << std::endl;
            break;
        case TokenType::BOOLCONST:
            std::cout << tokenTypeToString(token.getType()) << " Value: " << token.getBool(); 
            std::cout << "  Input: " << token.getToken() << std::endl;
            break;
        default:
            std::cout << tokenTypeToString(token.getType()) << std::endl;
            break;
    } 
    return;
}

} // namespace token
