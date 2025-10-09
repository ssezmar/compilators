#include "lexer.h"
#include <cctype>
#include <algorithm>
#include <sstream>

// Определение статических членов
const std::vector<std::string> Lexer::keywords_ = {
    "int", "float", "double", "char", "bool", "void", "string",
    "if", "else", "while", "for", "do", "switch", "case", "default",
    "break", "continue", "return", "const", "static", "extern",
    "struct", "class", "public", "private", "protected",
    "true", "false", "null", "this", "new", "delete"
};

const std::vector<std::string> Lexer::twoCharOps_ = {
    "==", "!=", "<=", ">=", "++", "--", "&&", "||", 
    "+=", "-=", "*=", "/=", "%=", "<<", ">>", "->"
};

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "[" << line << ":" << col << "] " << typeToString() << " \"" << text << "\"";
    return oss.str();
}

std::string Token::typeToString() const {
    switch(type) {
        case T_KEYWORD: return "KEYWORD";
        case T_IDENTIFIER: return "IDENTIFIER";
        case T_INTEGER: return "INTEGER";
        case T_FLOAT: return "FLOAT";
        case T_STRING: return "STRING";
        case T_OPERATOR: return "OPERATOR";
        case T_DELIMITER: return "DELIMITER";
        case T_COMMENT: return "COMMENT";
        case T_ERROR: return "ERROR";
        case T_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}

Lexer::Lexer(const std::string& src)
    : src_(src), pos_(0), line_(1), col_(1), peeked_(false) {}

void Lexer::skipWhitespace() {
    while (pos_ < src_.size() && std::isspace(src_[pos_])) {
        if (src_[pos_] == '\n') { 
            ++line_; 
            col_ = 1; 
        } else {
            ++col_;
        }
        ++pos_;
    }
}

bool Lexer::isKeyword(const std::string& word) {
    return std::find(keywords_.begin(), keywords_.end(), word) != keywords_.end();
}

Token Lexer::errorToken(const std::string& msg) {
    Token t(T_ERROR, msg, line_, col_);
    if (pos_ < src_.size()) {
        ++pos_; 
        ++col_;
    }
    return t;
}

bool Lexer::lexComment(Token& tok) {
    if (pos_ + 1 < src_.size() && src_[pos_] == '/') {
        if (src_[pos_+1] == '/') {
            // Однострочный комментарий
            int l = line_, c = col_;
            size_t start = pos_;
            pos_ += 2; col_ += 2;
            while (pos_ < src_.size() && src_[pos_] != '\n') {
                ++pos_; ++col_;
            }
            tok = Token(T_COMMENT, src_.substr(start, pos_ - start), l, c);
            return true;
        }
        if (src_[pos_+1] == '*') {
            // Многострочный комментарий
            int l = line_, c = col_;
            size_t start = pos_;
            pos_ += 2; col_ += 2;
            while (pos_ + 1 < src_.size()) {
                if (src_[pos_] == '*' && src_[pos_+1] == '/') {
                    pos_ += 2; col_ += 2;
                    tok = Token(T_COMMENT, src_.substr(start, pos_ - start), l, c);
                    return true;
                }
                if (src_[pos_] == '\n') { 
                    ++line_; 
                    col_ = 1; 
                } else {
                    ++col_;
                }
                ++pos_;
            }
            // Незавершенный комментарий
            tok = errorToken("Unclosed block comment");
            return true;
        }
    }
    return false;
}

Token Lexer::lexIdentifier() {
    int l = line_, c = col_;
    size_t start = pos_;
    while (pos_ < src_.size() && (std::isalnum(src_[pos_]) || src_[pos_] == '_')) {
        ++pos_; ++col_;
    }
    std::string word = src_.substr(start, pos_ - start);
    return Token(isKeyword(word) ? T_KEYWORD : T_IDENTIFIER, word, l, c);
}

Token Lexer::lexNumber() {
    int l = line_, c = col_;
    size_t start = pos_;
    bool isFloat = false;

    // Целая часть
    while (pos_ < src_.size() && std::isdigit(src_[pos_])) {
        ++pos_; ++col_;
    }

    // Дробная часть
    if (pos_ < src_.size() && src_[pos_] == '.') {
        isFloat = true;
        ++pos_; ++col_;
        while (pos_ < src_.size() && std::isdigit(src_[pos_])) {
            ++pos_; ++col_;
        }
    }

    // Экспоненциальная часть
    if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
        isFloat = true;
        ++pos_; ++col_;
        if (pos_ < src_.size() && (src_[pos_] == '+' || src_[pos_] == '-')) {
            ++pos_; ++col_;
        }
        while (pos_ < src_.size() && std::isdigit(src_[pos_])) {
            ++pos_; ++col_;
        }
    }

    return Token(isFloat ? T_FLOAT : T_INTEGER, src_.substr(start, pos_ - start), l, c);
}

Token Lexer::lexString() {
    int l = line_, c = col_;
    size_t start = pos_;
    ++pos_; ++col_; // Пропустить открывающую кавычку

    while (pos_ < src_.size() && src_[pos_] != '\"') {
        if (src_[pos_] == '\\' && pos_ + 1 < src_.size()) {
            pos_ += 2; col_ += 2; // Экранированный символ
        } else {
            if (src_[pos_] == '\n') {
                return errorToken("Unterminated string literal");
            }
            ++pos_; ++col_;
        }
    }

    if (pos_ < src_.size() && src_[pos_] == '\"') {
        ++pos_; ++col_; // Пропустить закрывающую кавычку
        return Token(T_STRING, src_.substr(start, pos_ - start), l, c);
    }

    return errorToken("Unterminated string literal");
}

Token Lexer::lexOperatorOrDelimiter() {
    int l = line_, c = col_;
    char ch = src_[pos_];

    // Проверяем двухсимвольные операторы
    if (pos_ + 1 < src_.size()) {
        std::string twoChar = std::string() + ch + src_[pos_+1];
        if (std::find(twoCharOps_.begin(), twoCharOps_.end(), twoChar) != twoCharOps_.end()) {
            pos_ += 2; col_ += 2;
            return Token(T_OPERATOR, twoChar, l, c);
        }
    }

    ++pos_; ++col_;
    std::string singleChar(1, ch);

    // Односимвольные операторы
    const std::string opChars = "+-*/%=<>!&|^~";
    if (opChars.find(ch) != std::string::npos) {
        return Token(T_OPERATOR, singleChar, l, c);
    }

    // Разделители
    const std::string delimChars = "(){}[];,.";
    if (delimChars.find(ch) != std::string::npos) {
        return Token(T_DELIMITER, singleChar, l, c);
    }

    return errorToken("Unknown character: " + singleChar);
}

Token Lexer::next() {
    if (peeked_) {
        peeked_ = false;
        return peekedToken_;
    }

    skipWhitespace();
    if (pos_ >= src_.size()) {
        return Token(T_EOF, "", line_, col_);
    }

    Token tok;
    if (lexComment(tok)) return tok;

    char ch = src_[pos_];
    if (std::isalpha(ch) || ch == '_') return lexIdentifier();
    if (std::isdigit(ch)) return lexNumber();
    if (ch == '\"') return lexString();
    return lexOperatorOrDelimiter();
}

Token Lexer::peek() {
    if (!peeked_) {
        peekedToken_ = next();
        peeked_ = true;
    }
    return peekedToken_;
}

void Lexer::reset() {
    pos_ = 0;
    line_ = 1;
    col_ = 1;
    peeked_ = false;
}