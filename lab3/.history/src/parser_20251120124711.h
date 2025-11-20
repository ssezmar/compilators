/**
 * @file parser.h
 * @brief Syntax Analyzer (Parser) - Builds AST from tokens
 */

#pragma once

#include "lexer.h"
#include <memory>
#include "ir.h" 
#include <iostream>

// Forward declarations
struct ASTNode;
struct Program;
struct Statement;
struct Expression;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ProgramPtr = std::shared_ptr<Program>;
using StatementPtr = std::shared_ptr<Statement>;
using ExpressionPtr = std::shared_ptr<Expression>;

enum class ASTNodeType {
    PROGRAM, DECL, ASSIGN, IF_STMT, WHILE_STMT, FOR_STMT, 
    BLOCK, RETURN_STMT, PRINT_STMT,
    BIN_EXPR, UN_EXPR, VAR_EXPR, CONST_EXPR, CALL_EXPR
};

struct ASTNode {
    ASTNodeType type;
    int line, column;
    
    ASTNode(ASTNodeType t, int l = 0, int c = 0) 
        : type(t), line(l), column(c) {}
    virtual ~ASTNode() = default;
};

struct Expression : public ASTNode {
    std::string dataType;  // "int", "bool"
    
    Expression(ASTNodeType t, int l = 0, int c = 0) 
        : ASTNode(t, l, c), dataType("int") {}
};

struct BinExpr : public Expression {
    ExpressionPtr left, right;
    BinOp op;
    
    BinExpr(ExpressionPtr l, BinOp o, ExpressionPtr r, int line = 0)
    : Expression(ASTNodeType::BIN_EXPR, line), left(l), right(r), op(o) {}
};

struct UnExpr : public Expression {
    ExpressionPtr operand;
    UnOp op;
    
    UnExpr(UnOp o, ExpressionPtr e, int line = 0)
        : Expression(ASTNodeType::UN_EXPR, line), op(o), operand(e) {}
};

struct VarExpr : public Expression {
    std::string name;
    
    VarExpr(const std::string& n, int line = 0)
        : Expression(ASTNodeType::VAR_EXPR, line), name(n) {}
};

struct ConstExpr : public Expression {
    std::variant<int, bool> value;
    
    ConstExpr(int v, int line = 0)
        : Expression(ASTNodeType::CONST_EXPR, line), value(v) {}
    ConstExpr(bool v, int line = 0)
        : Expression(ASTNodeType::CONST_EXPR, line), value(v) { dataType = "bool"; }
};

struct CallExpr : public Expression {
    std::string funcName;
    std::vector<ExpressionPtr> args;
    
    CallExpr(const std::string& fn, int line = 0)
        : Expression(ASTNodeType::CALL_EXPR, line), funcName(fn) {}
};

struct Statement : public ASTNode {
    Statement(ASTNodeType t, int l = 0, int c = 0) : ASTNode(t, l, c) {}
};

struct DeclStmt : public Statement {
    std::string varName;
    std::string dataType;  // "int" or "bool"
    ExpressionPtr initializer;
    
    DeclStmt(const std::string& name, const std::string& type, int line = 0)
        : Statement(ASTNodeType::DECL, line), varName(name), dataType(type) {}
};

struct AssignStmt : public Statement {
    std::string varName;
    ExpressionPtr value;
    
    AssignStmt(const std::string& name, ExpressionPtr expr, int line = 0)
        : Statement(ASTNodeType::ASSIGN, line), varName(name), value(expr) {}
};

struct IfStmt : public Statement {
    ExpressionPtr condition;
    std::vector<StatementPtr> thenBranch;
    std::vector<StatementPtr> elseBranch;
    
    IfStmt(ExpressionPtr cond, int line = 0)
        : Statement(ASTNodeType::IF_STMT, line), condition(cond) {}
};

struct WhileStmt : public Statement {
    ExpressionPtr condition;
    std::vector<StatementPtr> body;
    
    WhileStmt(ExpressionPtr cond, int line = 0)
        : Statement(ASTNodeType::WHILE_STMT, line), condition(cond) {}
};

struct ForStmt : public Statement {
    StatementPtr init;
    ExpressionPtr condition;
    ExpressionPtr update;
    std::vector<StatementPtr> body;
    
    ForStmt(int line = 0)
        : Statement(ASTNodeType::FOR_STMT, line), init(nullptr),
          condition(nullptr), update(nullptr) {}
};

struct BlockStmt : public Statement {
    std::vector<StatementPtr> statements;
    
    BlockStmt(int line = 0) : Statement(ASTNodeType::BLOCK, line) {}
};

struct ReturnStmt : public Statement {
    ExpressionPtr value;
    
    ReturnStmt(ExpressionPtr v = nullptr, int line = 0)
        : Statement(ASTNodeType::RETURN_STMT, line), value(v) {}
};

struct PrintStmt : public Statement {
    ExpressionPtr value;
    
    PrintStmt(ExpressionPtr v, int line = 0)
        : Statement(ASTNodeType::PRINT_STMT, line), value(v) {}
};

struct Program : public ASTNode {
    std::vector<StatementPtr> statements;
    
    Program() : ASTNode(ASTNodeType::PROGRAM) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current = 0;
    std::string lastError;

public:
    Parser(const std::vector<Token>& toks) : tokens(toks) {}

    ProgramPtr parse() {
        auto program = std::make_shared<Program>();
        
        while(!isAtEnd()) {
            try {
                auto stmt = statement();
                if(stmt) program->statements.push_back(stmt);
            } catch(const std::exception& e) {
                fprintf(stderr, "Parse error: %s\n", e.what());
                // Skip to next statement
                while(!isAtEnd() && !check(TokenType::SEMICOLON)) advance();
                if(match(TokenType::SEMICOLON)) advance();
            }
        }
        
        return program;
    }

private:
    const Token& peek() const { return tokens[current]; }
    const Token& previous() const { return tokens[current - 1]; }
    bool check(TokenType type) const { return peek().type == type; }
    bool isAtEnd() const { return peek().type == TokenType::END_OF_FILE; }
    
    Token advance() {
        if(!isAtEnd()) current++;
        return previous();
    }

    bool match(TokenType type) {
        if(check(type)) {
            advance();
            return true;
        }
        return false;
    }

    Token consume(TokenType type, const char* message) {
        if(check(type)) return advance();
        throw std::runtime_error(message);
    }

    StatementPtr statement() {
        if(match(TokenType::INT_KW) || match(TokenType::BOOL_KW)) {
            return declaration();
        }
        if(match(TokenType::IF)) return ifStatement();
        if(match(TokenType::WHILE)) return whileStatement();
        if(match(TokenType::FOR)) return forStatement();
        if(match(TokenType::RETURN)) return returnStatement();
        if(match(TokenType::PRINT)) return printStatement();
        if(match(TokenType::LBRACE)) return blockStatement();
        
        return expressionStatement();
    }

    StatementPtr declaration() {
        std::string typeStr = previous().lexeme;
        std::string varName = consume(TokenType::IDENT, "Expected variable name").lexeme;
        
        auto decl = std::make_shared<DeclStmt>(varName, typeStr, previous().line);
        
        if(match(TokenType::ASSIGN)) {
            decl->initializer = expression();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' after declaration");
        return decl;
    }

    StatementPtr ifStatement() {
        consume(TokenType::LPAREN, "Expected '(' after 'if'");
        auto condition = expression();
        consume(TokenType::RPAREN, "Expected ')' after if condition");
        
        auto ifStmt = std::make_shared<IfStmt>(condition, previous().line);
        
        if(match(TokenType::LBRACE)) {
            do {
                if(check(TokenType::RBRACE)) break;
                ifStmt->thenBranch.push_back(statement());
            } while(!isAtEnd());
            consume(TokenType::RBRACE, "Expected '}' after if body");
        } else {
            ifStmt->thenBranch.push_back(statement());
        }
        
        if(match(TokenType::ELSE)) {
            if(match(TokenType::LBRACE)) {
                do {
                    if(check(TokenType::RBRACE)) break;
                    ifStmt->elseBranch.push_back(statement());
                } while(!isAtEnd());
                consume(TokenType::RBRACE, "Expected '}' after else body");
            } else {
                ifStmt->elseBranch.push_back(statement());
            }
        }
        
        return ifStmt;
    }

    StatementPtr whileStatement() {
        consume(TokenType::LPAREN, "Expected '(' after 'while'");
        auto condition = expression();
        consume(TokenType::RPAREN, "Expected ')' after while condition");
        
        auto whileStmt = std::make_shared<WhileStmt>(condition, previous().line);
        
        if(match(TokenType::LBRACE)) {
            do {
                if(check(TokenType::RBRACE)) break;
                whileStmt->body.push_back(statement());
            } while(!isAtEnd());
            consume(TokenType::RBRACE, "Expected '}' after while body");
        } else {
            whileStmt->body.push_back(statement());
        }
        
        return whileStmt;
    }

    StatementPtr forStatement() {
        consume(TokenType::LPAREN, "Expected '(' after 'for'");
        
        auto forStmt = std::make_shared<ForStmt>(previous().line);
        
        // Init
        if(!check(TokenType::SEMICOLON)) {
            if(check(TokenType::INT_KW) || check(TokenType::BOOL_KW)) {
                advance();
                std::string type = previous().lexeme;
                std::string name = consume(TokenType::IDENT, "Expected variable name").lexeme;
                forStmt->init = std::make_shared<DeclStmt>(name, type);
                
                if(match(TokenType::ASSIGN)) {
                    auto decl = std::dynamic_pointer_cast<DeclStmt>(forStmt->init);
                    decl->initializer = expression();
                }
            } else {
                forStmt->init = expressionStatement();
            }
        }
        consume(TokenType::SEMICOLON, "Expected ';' after for init");
        
        // Condition
        if(!check(TokenType::SEMICOLON)) {
            forStmt->condition = expression();
        }
        consume(TokenType::SEMICOLON, "Expected ';' after for condition");
        
        // Update
        if(!check(TokenType::RPAREN)) {
            forStmt->update = expression();
        }
        consume(TokenType::RPAREN, "Expected ')' after for clauses");
        
        // Body
        if(match(TokenType::LBRACE)) {
            do {
                if(check(TokenType::RBRACE)) break;
                forStmt->body.push_back(statement());
            } while(!isAtEnd());
            consume(TokenType::RBRACE, "Expected '}' after for body");
        } else {
            forStmt->body.push_back(statement());
        }
        
        return forStmt;
    }

    StatementPtr returnStatement() {
        ExpressionPtr value = nullptr;
        if(!check(TokenType::SEMICOLON)) {
            value = expression();
        }
        consume(TokenType::SEMICOLON, "Expected ';' after return");
        return std::make_shared<ReturnStmt>(value, previous().line);
    }

    StatementPtr printStatement() {
        consume(TokenType::LPAREN, "Expected '(' after 'print'");
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after print argument");
        consume(TokenType::SEMICOLON, "Expected ';' after print");
        return std::make_shared<PrintStmt>(expr, previous().line);
    }

    StatementPtr blockStatement() {
        auto block = std::make_shared<BlockStmt>(previous().line);
        
        while(!check(TokenType::RBRACE) && !isAtEnd()) {
            block->statements.push_back(statement());
        }
        
        consume(TokenType::RBRACE, "Expected '}' after block");
        return block;
    }

    StatementPtr expressionStatement() {
        if(check(TokenType::IDENT)) {
            Token ident = advance();
            
            if(match(TokenType::ASSIGN)) {
                auto expr = expression();
                consume(TokenType::SEMICOLON, "Expected ';' after assignment");
                return std::make_shared<AssignStmt>(ident.lexeme, expr, ident.line);
            }
        }
        
        current--;  // Back up
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after expression");
        
        // Create dummy statement if needed
        return std::make_shared<Statement>(ASTNodeType::BLOCK);
    }

    ExpressionPtr expression() {
        return orExpression();
    }

    ExpressionPtr orExpression() {
        auto expr = andExpression();
        
        while(match(TokenType::OR)) {
            auto right = andExpression();
            expr = std::make_shared<BinExpr>(expr, BinOp::OR, right, previous().line);
        }
        
        return expr;
    }

    ExpressionPtr andExpression() {
        auto expr = equalityExpression();
        
        while(match(TokenType::AND)) {
            auto right = equalityExpression();
            expr = std::make_shared<BinExpr>(expr, BinOp::AND, right, previous().line);
        }
        
        return expr;
    }

    ExpressionPtr equalityExpression() {
        auto expr = relationalExpression();
        
        while(true) {
            if(match(TokenType::EQ)) {
                auto right = relationalExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::EQ, right, previous().line);
            } else if(match(TokenType::NE)) {
                auto right = relationalExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::NE, right, previous().line);
            } else {
                break;
            }
        }
        
        return expr;
    }

    ExpressionPtr relationalExpression() {
        auto expr = additiveExpression();
        
        while(true) {
            if(match(TokenType::LT)) {
                auto right = additiveExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::LT, right, previous().line);
            } else if(match(TokenType::GT)) {
                auto right = additiveExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::GT, right, previous().line);
            } else if(match(TokenType::LE)) {
                auto right = additiveExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::LE, right, previous().line);
            } else if(match(TokenType::GE)) {
                auto right = additiveExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::GE, right, previous().line);
            } else {
                break;
            }
        }
        
        return expr;
    }

    ExpressionPtr additiveExpression() {
        auto expr = multiplicativeExpression();
        
        while(true) {
            if(match(TokenType::PLUS)) {
                auto right = multiplicativeExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::ADD, right, previous().line);
            } else if(match(TokenType::MINUS)) {
                auto right = multiplicativeExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::SUB, right, previous().line);
            } else {
                break;
            }
        }
        
        return expr;
    }

    ExpressionPtr multiplicativeExpression() {
        auto expr = unaryExpression();
        
        while(true) {
            if(match(TokenType::STAR)) {
                auto right = unaryExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::MUL, right, previous().line);
            } else if(match(TokenType::SLASH)) {
                auto right = unaryExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::DIV, right, previous().line);
            } else if(match(TokenType::PERCENT)) {
                auto right = unaryExpression();
                expr = std::make_shared<BinExpr>(expr, BinOp::MOD, right, previous().line);
            } else {
                break;
            }
        }
        
        return expr;
    }

    ExpressionPtr unaryExpression() {
        if(match(TokenType::MINUS)) {
            auto expr = unaryExpression();
            return std::make_shared<UnExpr>(UnOp::NEG, expr, previous().line);
        }
        
        if(match(TokenType::NOT)) {
            auto expr = unaryExpression();
            return std::make_shared<UnExpr>(UnOp::NOT, expr, previous().line);
        }
        
        return primaryExpression();
    }

    ExpressionPtr primaryExpression() {
        if(match(TokenType::INT_LIT)) {
            return std::make_shared<ConstExpr>(previous().intValue, previous().line);
        }
        
        if(match(TokenType::BOOL_LIT)) {
            return std::make_shared<ConstExpr>(previous().boolValue, previous().line);
        }
        
        if(match(TokenType::IDENT)) {
            std::string name = previous().lexeme;
            int line = previous().line;
            
            if(match(TokenType::LPAREN)) {
                auto call = std::make_shared<CallExpr>(name, line);
                if(!check(TokenType::RPAREN)) {
                    do {
                        call->args.push_back(expression());
                    } while(match(TokenType::COMMA));
                }
                consume(TokenType::RPAREN, "Expected ')' after function arguments");
                return call;
            }
            
            return std::make_shared<VarExpr>(name, line);
        }
        
        if(match(TokenType::LPAREN)) {
            auto expr = expression();
            consume(TokenType::RPAREN, "Expected ')' after expression");
            return expr;
        }
        
        throw std::runtime_error("Expected expression");
    }
};
