#include "parser.h"
#include <iostream>

// Конструктор
Parser::Parser(Lexer& lex)
  : lex_(lex)
{
    advance();
}

// Получить следующий токен
void Parser::advance() {
    cur_ = lex_.next();
}

// Проверить, что текущий токен имеет тип t, иначе — ошибка
void Parser::expect(TokenType t) {
    if (cur_.type != t) {
        error("Expected token type " + std::to_string(t) +
              ", got '" + cur_.text + "'");
    }
    advance();
}

// Попытаться принять токен типа t, вернуть true если совпало
bool Parser::accept(TokenType t) {
    if (cur_.type == t) {
        advance();
        return true;
    }
    return false;
}

// Сообщение об ошибке разбора
void Parser::error(const std::string& msg) {
    std::cerr << "Parse error at " << cur_.line << ":" << cur_.col
              << ": " << msg << std::endl;
    throw std::runtime_error("Parse error");
}

// <program> ::= <stmt_list> EOF
std::shared_ptr<AST> Parser::parseProgram() {
    auto root = std::make_shared<AST>("Program");
    root->children.push_back(parseStmtList());
    expect(T_EOF);
    return root;
}

// <stmt_list> ::= { <stmt> }
std::shared_ptr<AST> Parser::parseStmtList() {
    auto node = std::make_shared<AST>("StmtList");
    while (true) {
        if (cur_.type == T_KEYWORD && 
           (cur_.text == "if" || cur_.text == "while")) { }
        else if (cur_.type == T_DELIMITER && cur_.text == "{") { }
        else if (cur_.type == T_IDENTIFIER || cur_.type == T_INTEGER) { }
        else break;
        node->children.push_back(parseStmt());
    }
    return node;
}

// <stmt> ::= if '(' <expr> ')' <stmt> [ else <stmt> ]
//          | while '(' <expr> ')' <stmt>
//          | '{' <stmt_list> '}'
//          | <expr> ';'
std::shared_ptr<AST> Parser::parseStmt() {
    auto node = std::make_shared<AST>("Stmt");
    if (cur_.type == T_KEYWORD && cur_.text == "if") {
        node->name = "If";
        advance();            // if
        expect(T_DELIMITER);  // (
        node->children.push_back(parseExpr());
        expect(T_DELIMITER);  // )
        node->children.push_back(parseStmt());
        if (cur_.type == T_KEYWORD && cur_.text == "else") {
            advance();
            node->children.push_back(parseStmt());
        }
    }
    else if (cur_.type == T_KEYWORD && cur_.text == "while") {
        node->name = "While";
        advance();            // while
        expect(T_DELIMITER);  // (
        node->children.push_back(parseExpr());
        expect(T_DELIMITER);  // )
        node->children.push_back(parseStmt());
    }
    else if (cur_.type == T_DELIMITER && cur_.text == "{") {
        node->name = "Block";
        advance();            // {
        node->children.push_back(parseStmtList());
        expect(T_DELIMITER);  // }
    }
    else {
        node->name = "ExprStmt";
        node->children.push_back(parseExpr());
        expect(T_DELIMITER);  // ;
    }
    return node;
}

// <expr> ::= <term> { ('+'|'-') <term> }
std::shared_ptr<AST> Parser::parseExpr() {
    auto node = std::make_shared<AST>("Expr");
    node->children.push_back(parseTerm());
    while (cur_.type == T_OPERATOR && 
          (cur_.text == "+" || cur_.text == "-")) {
        auto op = std::make_shared<AST>("Op:" + cur_.text);
        advance();
        op->children.push_back(node->children.back());
        op->children.push_back(parseTerm());
        node->children.back() = op;
    }
    return node;
}

// <term> ::= <factor> { ('*'|'/') <factor> }
std::shared_ptr<AST> Parser::parseTerm() {
    auto node = std::make_shared<AST>("Term");
    node->children.push_back(parseFactor());
    while (cur_.type == T_OPERATOR && 
          (cur_.text == "*" || cur_.text == "/")) {
        auto op = std::make_shared<AST>("Op:" + cur_.text);
        advance();
        op->children.push_back(node->children.back());
        op->children.push_back(parseFactor());
        node->children.back() = op;
    }
    return node;
}

// <factor> ::= IDENTIFIER | INTEGER | '(' <expr> ')'
std::shared_ptr<AST> Parser::parseFactor() {
    if (cur_.type == T_IDENTIFIER) {
        auto node = std::make_shared<AST>("Id:" + cur_.text);
        advance();
        return node;
    }
    if (cur_.type == T_INTEGER) {
        auto node = std::make_shared<AST>("Num:" + cur_.text);
        advance();
        return node;
    }
    if (cur_.type == T_DELIMITER && cur_.text == "(") {
        advance();
        auto node = parseExpr();
        expect(T_DELIMITER);  // )
        return node;
    }
    error("Expected identifier, number, or '('");
    return nullptr;  // никогда не выполнится
}