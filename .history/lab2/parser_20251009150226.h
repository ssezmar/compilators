#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <stdexcept>
#include <vector>

struct AST {
    std::string name;
    std::vector<std::shared_ptr<AST>> children;
    AST(const std::string& n): name(n) {}
    void print(int d) const {
        std::cout << std::string(d*2,' ') << name << "\n";
        for (auto& c : children) c->print(d+1);
    }
};

class Parser {
public:
    Parser(Lexer&);
    std::shared_ptr<AST> parseProgram();
private:
    Lexer& lex_;
    Token cur_;
    void advance();
    void expect(TokenType);
    bool accept(TokenType);
    void error(const std::string&);
    std::shared_ptr<AST> parseStmtList();
    std::shared_ptr<AST> parseStmt();
    std::shared_ptr<AST> parseExpr();
    std::shared_ptr<AST> parseTerm();
    std::shared_ptr<AST> parseFactor();
};

#endif // PARSER_H