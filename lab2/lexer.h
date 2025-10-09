#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <iostream>

enum TokenType {
    T_KEYWORD,      // Ключевые слова (int, float, if, else, etc.)
    T_IDENTIFIER,   // Идентификаторы
    T_INTEGER,      // Целые числа
    T_FLOAT,        // Вещественные числа
    T_STRING,       // Строковые литералы
    T_OPERATOR,     // Операторы (+, -, *, /, ==, etc.)
    T_DELIMITER,    // Разделители ((), {}, [], ;, ,, .)
    T_COMMENT,      // Комментарии
    T_ERROR,        // Ошибки
    T_EOF           // Конец файла
};

struct Token {
    TokenType type;
    std::string text;
    int line, col;

    Token() : type(T_EOF), text(""), line(0), col(0) {}
    Token(TokenType t, const std::string& txt, int l, int c) 
        : type(t), text(txt), line(l), col(c) {}

    // Для красивого вывода токенов
    std::string toString() const;
    std::string typeToString() const;
};

class Lexer {
public:
    explicit Lexer(const std::string& src);
    Token next();
    Token peek(); // Просмотр следующего токена без его извлечения
    void reset(); // Сброс позиции в начало

    // Для отладки и логирования
    size_t getPosition() const { return pos_; }
    int getLine() const { return line_; }
    int getCol() const { return col_; }

private:
    std::string src_;
    size_t pos_;
    int line_, col_;
    bool peeked_;
    Token peekedToken_;

    void skipWhitespace();
    bool lexComment(Token& tok);
    Token lexIdentifier();
    Token lexNumber();
    Token lexString();
    Token lexOperatorOrDelimiter();
    Token errorToken(const std::string& msg);
    bool isKeyword(const std::string& word);

    static const std::vector<std::string> keywords_;
    static const std::vector<std::string> twoCharOps_;
};

#endif // LEXER_H