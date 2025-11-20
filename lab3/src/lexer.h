/**
 * @file lexer.h
 * @brief Lexical Analyzer (Tokenizer)
 */

#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <unordered_set>

enum class TokenType {
    // Literals and identifiers
    INT_LIT, BOOL_LIT, IDENT,
    
    // Keywords
    INT_KW, BOOL_KW, IF, ELSE, WHILE, FOR, RETURN, PRINT,
    
    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, NE, LT, GT, LE, GE,
    AND, OR, NOT,
    ASSIGN,
    
    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON, COMMA,
    
    // Special
    NEWLINE, END_OF_FILE, ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    int intValue = 0;
    bool boolValue = false;

    Token() : type(TokenType::ERROR), line(0), column(0) {}
    Token(TokenType t, const std::string& lex, int l, int c)
        : type(t), lexeme(lex), line(l), column(c) {}

    std::string typeString() const {
        static const std::unordered_map<TokenType, std::string> typeNames = {
            {TokenType::INT_LIT, "INT_LIT"},
            {TokenType::BOOL_LIT, "BOOL_LIT"},
            {TokenType::IDENT, "IDENT"},
            {TokenType::INT_KW, "INT"},
            {TokenType::BOOL_KW, "BOOL"},
            {TokenType::IF, "IF"},
            {TokenType::ELSE, "ELSE"},
            {TokenType::WHILE, "WHILE"},
            {TokenType::FOR, "FOR"},
            {TokenType::RETURN, "RETURN"},
            {TokenType::PRINT, "PRINT"},
            {TokenType::PLUS, "PLUS"},
            {TokenType::MINUS, "MINUS"},
            {TokenType::STAR, "STAR"},
            {TokenType::SLASH, "SLASH"},
            {TokenType::PERCENT, "PERCENT"},
            {TokenType::EQ, "EQ"},
            {TokenType::NE, "NE"},
            {TokenType::LT, "LT"},
            {TokenType::GT, "GT"},
            {TokenType::LE, "LE"},
            {TokenType::GE, "GE"},
            {TokenType::AND, "AND"},
            {TokenType::OR, "OR"},
            {TokenType::NOT, "NOT"},
            {TokenType::ASSIGN, "ASSIGN"},
            {TokenType::LPAREN, "LPAREN"},
            {TokenType::RPAREN, "RPAREN"},
            {TokenType::LBRACE, "LBRACE"},
            {TokenType::RBRACE, "RBRACE"},
            {TokenType::SEMICOLON, "SEMICOLON"},
            {TokenType::COMMA, "COMMA"},
            {TokenType::END_OF_FILE, "EOF"}
        };
        return typeNames.count(type) ? typeNames.at(type) : "UNKNOWN";
    }
};

class Lexer {
private:
    std::string source;
    size_t current = 0;
    int line = 1;
    int column = 1;
    
    static const std::unordered_set<std::string> KEYWORDS;

public:
    Lexer(const std::string& src) : source(src) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while(current < source.length()) {
            skipWhitespaceAndComments();
            
            if(current >= source.length()) break;
            
            Token token = nextToken();
            if(token.type != TokenType::ERROR || !token.lexeme.empty()) {
                tokens.push_back(token);
            }
        }
        
        tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
        return tokens;
    }

private:
    char peek(int offset = 0) const {
        if(current + offset >= source.length()) return '\0';
        return source[current + offset];
    }

    char advance() {
        char ch = source[current++];
        if(ch == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        return ch;
    }

    void skipWhitespaceAndComments() {
        while(current < source.length()) {
            if(isspace(peek())) {
                advance();
            } else if(peek() == '/' && peek(1) == '/') {
                // Line comment
                while(peek() != '\n' && peek() != '\0') advance();
            } else if(peek() == '/' && peek(1) == '*') {
                // Block comment
                advance(); advance(); // skip /*
                while(!(peek() == '*' && peek(1) == '/')) {
                    if(peek() == '\0') break;
                    advance();
                }
                if(peek() == '*' && peek(1) == '/') {
                    advance(); advance(); // skip */
                }
            } else {
                break;
            }
        }
    }

    Token nextToken() {
        int startLine = line, startCol = column;
        char ch = peek();

        // Numbers
        if(isdigit(ch)) {
            std::string num;
            while(isdigit(peek())) {
                num += advance();
            }
            Token token(TokenType::INT_LIT, num, startLine, startCol);
            token.intValue = std::stoi(num);
            return token;
        }

        // Identifiers and keywords
        if(isalpha(ch) || ch == '_') {
            std::string ident;
            while(isalnum(peek()) || peek() == '_') {
                ident += advance();
            }
            
            // Check for keywords
            if(ident == "int") return Token(TokenType::INT_KW, ident, startLine, startCol);
            if(ident == "bool") return Token(TokenType::BOOL_KW, ident, startLine, startCol);
            if(ident == "if") return Token(TokenType::IF, ident, startLine, startCol);
            if(ident == "else") return Token(TokenType::ELSE, ident, startLine, startCol);
            if(ident == "while") return Token(TokenType::WHILE, ident, startLine, startCol);
            if(ident == "for") return Token(TokenType::FOR, ident, startLine, startCol);
            if(ident == "return") return Token(TokenType::RETURN, ident, startLine, startCol);
            if(ident == "print") return Token(TokenType::PRINT, ident, startLine, startCol);
            if(ident == "true") {
                Token token(TokenType::BOOL_LIT, ident, startLine, startCol);
                token.boolValue = true;
                return token;
            }
            if(ident == "false") {
                Token token(TokenType::BOOL_LIT, ident, startLine, startCol);
                token.boolValue = false;
                return token;
            }
            
            return Token(TokenType::IDENT, ident, startLine, startCol);
        }

        // Operators and delimiters
        advance();
        
        switch(ch) {
            case '+': return Token(TokenType::PLUS, "+", startLine, startCol);
            case '-': return Token(TokenType::MINUS, "-", startLine, startCol);
            case '*': return Token(TokenType::STAR, "*", startLine, startCol);
            case '/': return Token(TokenType::SLASH, "/", startLine, startCol);
            case '%': return Token(TokenType::PERCENT, "%", startLine, startCol);
            case '(': return Token(TokenType::LPAREN, "(", startLine, startCol);
            case ')': return Token(TokenType::RPAREN, ")", startLine, startCol);
            case '{': return Token(TokenType::LBRACE, "{", startLine, startCol);
            case '}': return Token(TokenType::RBRACE, "}", startLine, startCol);
            case ';': return Token(TokenType::SEMICOLON, ";", startLine, startCol);
            case ',': return Token(TokenType::COMMA, ",", startLine, startCol);
            
            case '=':
                if(peek() == '=') {
                    advance();
                    return Token(TokenType::EQ, "==", startLine, startCol);
                }
                return Token(TokenType::ASSIGN, "=", startLine, startCol);
                
            case '!':
                if(peek() == '=') {
                    advance();
                    return Token(TokenType::NE, "!=", startLine, startCol);
                }
                return Token(TokenType::NOT, "!", startLine, startCol);
                
            case '<':
                if(peek() == '=') {
                    advance();
                    return Token(TokenType::LE, "<=", startLine, startCol);
                }
                return Token(TokenType::LT, "<", startLine, startCol);
                
            case '>':
                if(peek() == '=') {
                    advance();
                    return Token(TokenType::GE, ">=", startLine, startCol);
                }
                return Token(TokenType::GT, ">", startLine, startCol);
                
            case '&':
                if(peek() == '&') {
                    advance();
                    return Token(TokenType::AND, "&&", startLine, startCol);
                }
                break;
                
            case '|':
                if(peek() == '|') {
                    advance();
                    return Token(TokenType::OR, "||", startLine, startCol);
                }
                break;
        }

        return Token(TokenType::ERROR, std::string(1, ch), startLine, startCol);
    }
};

const std::unordered_set<std::string> Lexer::KEYWORDS = {
    "int", "bool", "if", "else", "while", "for", "return", "print"
};
