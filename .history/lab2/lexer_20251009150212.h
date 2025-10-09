#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum TokenType {
    T_KEYWORD, T_IDENTIFIER, T_INTEGER, T_FLOAT,
    T_STRING, T_OPERATOR, T_DELIMITER, T_COMMENT,
    T_ERROR, T_EOF
};

struct Token {
    TokenType type;
    std::string text;
    int line, col;
};

class Lexer {
public:
    Lexer(const std::string& src);
    Token next();
private:
    std::string src_;
    size_t pos_;
    int line_, col_;
    void skipWhitespace();
    bool lexComment(Token&);
    Token lexIdentifier();
    Token lexNumber();
    Token lexString();
    Token lexOperatorOrDelimiter();
    Token errorToken(char);
    bool isKeyword(const std::string&);
    static const std::vector<std::string> keywords_;
};

#endif // LEXER_H