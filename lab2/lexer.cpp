#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <iterator>

// Определение списка ключевых слов
const std::vector<std::string> Lexer::keywords_ = {
    "if","else","while","for","do","switch","case","default",
    "break","continue","return","int","float","double","char","bool","void"
};

Lexer::Lexer(const std::string& src)
  : src_(src), pos_(0), line_(1), col_(1)
{}

void Lexer::skipWhitespace() {
    while (pos_ < src_.size() && std::isspace(src_[pos_])) {
        if (src_[pos_] == '\n') { ++line_; col_ = 1; }
        else ++col_;
        ++pos_;
    }
}

bool Lexer::isKeyword(const std::string& w) {
    return std::find(keywords_.begin(), keywords_.end(), w) != keywords_.end();
}

Token Lexer::errorToken(char c) {
    std::string s(1, c);
    Token t{T_ERROR, s, line_, col_};
    ++pos_; ++col_;
    return t;
}

bool Lexer::lexComment(Token& tok) {
    if (pos_ + 1 < src_.size() && src_[pos_] == '/') {
        if (src_[pos_+1] == '/') {
            int l = line_, c = col_;
            size_t start = pos_;
            pos_ += 2; col_ += 2;
            while (pos_ < src_.size() && src_[pos_] != '\n') {
                ++pos_; ++col_;
            }
            tok = {T_COMMENT, src_.substr(start, pos_ - start), l, c};
            return true;
        }
        if (src_[pos_+1] == '*') {
            int l = line_, c = col_;
            size_t start = pos_;
            pos_ += 2; col_ += 2;
            while (pos_ + 1 < src_.size()) {
                if (src_[pos_] == '*' && src_[pos_+1] == '/') {
                    pos_ += 2; col_ += 2;
                    tok = {T_COMMENT, src_.substr(start, pos_ - start), l, c};
                    return true;
                }
                if (src_[pos_] == '\n') { ++line_; col_ = 1; }
                else ++col_;
                ++pos_;
            }
            // незавершён
            tok = {T_ERROR, "Unclosed comment", l, c};
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
    std::string w = src_.substr(start, pos_ - start);
    return { isKeyword(w) ? T_KEYWORD : T_IDENTIFIER, w, l, c };
}

Token Lexer::lexNumber() {
    int l = line_, c = col_;
    size_t start = pos_;
    while (pos_ < src_.size() && std::isdigit(src_[pos_])) {
        ++pos_; ++col_;
    }
    if (pos_ < src_.size() && src_[pos_] == '.') {
        ++pos_; ++col_;
        while (pos_ < src_.size() && std::isdigit(src_[pos_])) {
            ++pos_; ++col_;
        }
        return {T_FLOAT, src_.substr(start, pos_ - start), l, c};
    }
    return {T_INTEGER, src_.substr(start, pos_ - start), l, c};
}

Token Lexer::lexString() {
    int l = line_, c = col_;
    size_t start = pos_;
    ++pos_; ++col_; // пропустить "
    while (pos_ < src_.size() && src_[pos_] != '"') {
        if (src_[pos_] == '\\' && pos_ + 1 < src_.size()) {
            pos_ += 2; col_ += 2;
        } else {
            if (src_[pos_] == '\n') break;
            ++pos_; ++col_;
        }
    }
    if (pos_ < src_.size() && src_[pos_] == '"') {
        ++pos_; ++col_;
        return {T_STRING, src_.substr(start, pos_ - start), l, c};
    }
    return {T_ERROR, "Unterminated string", l, c};
}

Token Lexer::lexOperatorOrDelimiter() {
    int l = line_, c = col_;
    char ch = src_[pos_];
    std::string two;
    if (pos_ + 1 < src_.size()) {
        two = std::string()+ch+src_[pos_+1];
    }
    static const std::vector<std::string> ops = {
        "==","!=","<=",">=","++","--","&&","||","+=", "-=", "*=", "/=", "%="
    };
    if (!two.empty() && std::find(ops.begin(), ops.end(), two) != ops.end()) {
        pos_ += 2; col_ += 2;
        return {T_OPERATOR, two, l, c};
    }
    ++pos_; ++col_;
    std::string s(1,ch);
    const std::string opchars = "+-*/%=<>!&|^~";
    const std::string delims = "(){}[];,.";
    if (opchars.find(ch) != std::string::npos)
        return {T_OPERATOR, s, l, c};
    if (delims.find(ch) != std::string::npos)
        return {T_DELIMITER, s, l, c};
    return {T_ERROR, s, l, c};
}

Token Lexer::next() {
    skipWhitespace();
    if (pos_ >= src_.size()) return {T_EOF, "", line_, col_};

    Token tok;
    if (lexComment(tok)) return tok;

    char ch = src_[pos_];
    if (std::isalpha(ch) || ch == '_')   return lexIdentifier();
    if (std::isdigit(ch))                return lexNumber();
    if (ch == '"')                       return lexString();
    return lexOperatorOrDelimiter();
}