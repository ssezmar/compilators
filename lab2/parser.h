#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

// Узел дерева синтаксического разбора
struct ASTNode {
    std::string name;
    std::string value;
    int line, col;
    std::vector<std::shared_ptr<ASTNode>> children;

    ASTNode(const std::string& n, const std::string& v = "", int l = 0, int c = 0)
        : name(n), value(v), line(l), col(c) {}

    // Вывод дерева с отступами
    void print(int depth = 0, std::ostream& out = std::cout) const;

    // Вывод в формате для лог-файла
    void printToLog(std::ofstream& log, int depth = 0) const;

    // Добавить дочерний узел
    void addChild(std::shared_ptr<ASTNode> child) {
        children.push_back(child);
    }
};

// Класс для обработки ошибок парсинга
class ParseError : public std::runtime_error {
public:
    int line, col;
    std::string expected, found;

    ParseError(const std::string& msg, int l, int c, 
               const std::string& exp = "", const std::string& fnd = "")
        : std::runtime_error(msg), line(l), col(c), expected(exp), found(fnd) {}
};

// Рекурсивный нисходящий парсер
class Parser {
public:
    explicit Parser(Lexer& lex, const std::string& logFile = "parse.log");
    ~Parser();

    // Основная функция парсинга
    std::shared_ptr<ASTNode> parseProgram();

    // Статистика парсинга
    struct Stats {
        int totalNodes = 0;
        int errors = 0;
        double parseTime = 0.0;
    } stats;

private:
    Lexer& lex_;
    Token current_;
    std::ofstream logFile_;
    bool loggingEnabled_;
    int depth_;

    // Вспомогательные функции
    void advance();
    void expect(TokenType type);
    bool accept(TokenType type);
    void error(const std::string& msg);
    void logEntry(const std::string& rule, const std::string& msg = "");
    void logExit(const std::string& rule, bool success = true);
    bool isTypeKeyword(const std::string& text); // ДОБАВЛЕНО

    // Грамматические правила
    // <program> ::= <declaration_list>
    std::shared_ptr<ASTNode> parseProgram_();

    // <declaration_list> ::= <declaration> | <declaration_list> <declaration>
    std::shared_ptr<ASTNode> parseDeclarationList();

    // <declaration> ::= <var_declaration> | <function_declaration>
    std::shared_ptr<ASTNode> parseDeclaration();

    // <var_declaration> ::= <type> <identifier> [ "=" <expression> ] ";"
    std::shared_ptr<ASTNode> parseVarDeclaration();

    // <function_declaration> ::= <type> <identifier> "(" [<parameter_list>] ")" <compound_stmt>
    std::shared_ptr<ASTNode> parseFunctionDeclaration();

    // <parameter_list> ::= <parameter> | <parameter_list> "," <parameter>
    std::shared_ptr<ASTNode> parseParameterList();

    // <parameter> ::= <type> <identifier>
    std::shared_ptr<ASTNode> parseParameter();

    // <type> ::= "int" | "float" | "double" | "char" | "bool" | "void" | "string"
    std::shared_ptr<ASTNode> parseType();

    // <compound_stmt> ::= "{" <statement_list> "}"
    std::shared_ptr<ASTNode> parseCompoundStatement();

    // <statement_list> ::= <statement> | <statement_list> <statement>
    std::shared_ptr<ASTNode> parseStatementList();

    // <statement> ::= <expression_stmt> | <compound_stmt> | <if_stmt> | <while_stmt> | <return_stmt> | <var_declaration>
    std::shared_ptr<ASTNode> parseStatement();

    // <expression_stmt> ::= <expression> ";"
    std::shared_ptr<ASTNode> parseExpressionStatement();

    // <if_stmt> ::= "if" "(" <expression> ")" <statement> ["else" <statement>]
    std::shared_ptr<ASTNode> parseIfStatement();

    // <while_stmt> ::= "while" "(" <expression> ")" <statement>
    std::shared_ptr<ASTNode> parseWhileStatement();

    // <return_stmt> ::= "return" [<expression>] ";"
    std::shared_ptr<ASTNode> parseReturnStatement();

    // <expression> ::= <logical_or_expr>
    std::shared_ptr<ASTNode> parseExpression();

    // <logical_or_expr> ::= <logical_and_expr> { "||" <logical_and_expr> }
    std::shared_ptr<ASTNode> parseLogicalOrExpression();

    // <logical_and_expr> ::= <equality_expr> { "&&" <equality_expr> }
    std::shared_ptr<ASTNode> parseLogicalAndExpression();

    // <equality_expr> ::= <relational_expr> { ("==" | "!=") <relational_expr> }
    std::shared_ptr<ASTNode> parseEqualityExpression();

    // <relational_expr> ::= <additive_expr> { ("<" | ">" | "<=" | ">=") <additive_expr> }
    std::shared_ptr<ASTNode> parseRelationalExpression();

    // <additive_expr> ::= <multiplicative_expr> { ("+" | "-") <multiplicative_expr> }
    std::shared_ptr<ASTNode> parseAdditiveExpression();

    // <multiplicative_expr> ::= <unary_expr> { ("*" | "/" | "%") <unary_expr> }
    std::shared_ptr<ASTNode> parseMultiplicativeExpression();

    // <unary_expr> ::= ["!" | "-" | "+"] <primary_expr>
    std::shared_ptr<ASTNode> parseUnaryExpression();

    // <primary_expr> ::= <identifier> | <number> | <string> | "(" <expression> ")" | <function_call>
    std::shared_ptr<ASTNode> parsePrimaryExpression();

    // <function_call> ::= <identifier> "(" [<argument_list>] ")"
    std::shared_ptr<ASTNode> parseFunctionCall();

    // <argument_list> ::= <expression> | <argument_list> "," <expression>
    std::shared_ptr<ASTNode> parseArgumentList();
};

#endif // PARSER_H