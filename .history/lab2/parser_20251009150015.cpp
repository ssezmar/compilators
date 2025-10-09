#include "parser.h"
#include <iostream>

Parser::Parser(LexicalAnalyzer& lex):lex(lex){
    advance();
}

void Parser::advance() { cur=lex.nextToken(); }

void Parser::expect(TokenType t) {
    if (cur.type!=t) error("Unexpected '"+cur.text+"'");
    advance();
}

void Parser::error(const std::string& msg) {
    std::cerr<<"Parse error at "<<cur.line<<":"<<cur.pos<<": "<<msg<<"\n";
    throw std::runtime_error("Parse error");
}

std::shared_ptr<ASTNode> Parser::parseProgram(){
    auto root=std::make_shared<ASTNode>();
    root->name="Program";
    root->children.push_back(parseStmtList());
    expect(T_EOF);
    return root;
}

std::shared_ptr<ASTNode> Parser::parseStmtList(){
    auto node=std::make_shared<ASTNode>();
    node->name="StmtList";
    while (cur.type==T_KEYWORD && 
          (cur.text=="if"||cur.text=="while") ||
           cur.type==T_DELIMITER && (cur.text=="{" ) ||
           cur.type==T_IDENTIFIER || cur.type==T_INTEGER) {
        node->children.push_back(parseStmt());
    }
    return node;
}

std::shared_ptr<ASTNode> Parser::parseStmt(){
    // аналогично коду из описания
    // создаем узлы AST с именами Stmt, If, While, Block, ExprStmt
}

// parseExpr, parseTerm, parseFactor – по описанным алгоритмам
