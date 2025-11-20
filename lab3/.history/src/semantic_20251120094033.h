/**
 * @file semantic.h
 * @brief Semantic Analysis (Type Checking, Scope Management)
 */

#pragma once

#include "parser.h"
#include <unordered_map>
#include <set>

class SemanticAnalyzer {
private:
    struct Symbol {
        std::string name;
        std::string type;  // "int", "bool", "func"
        int definedLine;
        bool initialized = false;
    };

    std::unordered_map<std::string, Symbol> symbolTable;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

public:
    bool analyze(const ProgramPtr& program) {
        if(!program) return false;
        
        try {
            for(const auto& stmt : program->statements) {
                visitStatement(stmt);
            }
        } catch(const std::exception& e) {
            errors.push_back(std::string(e.what()));
            return false;
        }
        
        return errors.empty();
    }

    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }

    bool isVariableDefined(const std::string& name) const {
        return symbolTable.count(name) > 0;
    }

    std::string getVariableType(const std::string& name) const {
        auto it = symbolTable.find(name);
        return (it != symbolTable.end()) ? it->second.type : "";
    }

private:
    void visitStatement(const StatementPtr& stmt) {
        if(!stmt) return;

        if(auto decl = std::dynamic_pointer_cast<DeclStmt>(stmt)) {
            visitDecl(decl);
        } else if(auto assign = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
            visitAssign(assign);
        } else if(auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            visitIfStatement(ifStmt);
        } else if(auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            visitWhileStatement(whileStmt);
        } else if(auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
            visitForStatement(forStmt);
        } else if(auto block = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
            for(const auto& s : block->statements) {
                visitStatement(s);
            }
        } else if(auto printStmt = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
            visitExpression(printStmt->value);
        } else if(auto retStmt = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            if(retStmt->value) visitExpression(retStmt->value);
        }
    }

    void visitDecl(const std::shared_ptr<DeclStmt>& decl) {
        if(symbolTable.count(decl->varName)) {
            errors.push_back("Variable '" + decl->varName + "' already defined");
            return;
        }

        Symbol sym;
        sym.name = decl->varName;
        sym.type = decl->dataType;
        sym.definedLine = decl->line;
        sym.initialized = (decl->initializer != nullptr);

        if(decl->initializer) {
            auto exprType = visitExpression(decl->initializer);
            if(exprType != decl->dataType) {
                warnings.push_back("Type mismatch in initialization of '" + 
                    decl->varName + "': expected " + decl->dataType + 
                    ", got " + exprType);
            }
        }

        symbolTable[decl->varName] = sym;
    }

    void visitAssign(const std::shared_ptr<AssignStmt>& assign) {
        if(!symbolTable.count(assign->varName)) {
            errors.push_back("Variable '" + assign->varName + "' is not defined");
            return;
        }

        auto exprType = visitExpression(assign->value);
        const auto& varType = symbolTable[assign->varName].type;
        
        if(exprType != varType) {
            warnings.push_back("Type mismatch in assignment to '" + 
                assign->varName + "': expected " + varType + ", got " + exprType);
        }

        symbolTable[assign->varName].initialized = true;
    }

    void visitIfStatement(const std::shared_ptr<IfStmt>& ifStmt) {
        auto condType = visitExpression(ifStmt->condition);
        if(condType != "bool") {
            warnings.push_back("If condition should be boolean, got " + condType);
        }

        for(const auto& stmt : ifStmt->thenBranch) {
            visitStatement(stmt);
        }
        for(const auto& stmt : ifStmt->elseBranch) {
            visitStatement(stmt);
        }
    }

    void visitWhileStatement(const std::shared_ptr<WhileStmt>& whileStmt) {
        auto condType = visitExpression(whileStmt->condition);
        if(condType != "bool") {
            warnings.push_back("While condition should be boolean, got " + condType);
        }

        for(const auto& stmt : whileStmt->body) {
            visitStatement(stmt);
        }
    }

    void visitForStatement(const std::shared_ptr<ForStmt>& forStmt) {
        if(forStmt->init) visitStatement(forStmt->init);
        
        if(forStmt->condition) {
            auto condType = visitExpression(forStmt->condition);
            if(condType != "bool") {
                warnings.push_back("For condition should be boolean, got " + condType);
            }
        }
        
        if(forStmt->update) visitExpression(forStmt->update);
        
        for(const auto& stmt : forStmt->body) {
            visitStatement(stmt);
        }
    }

    std::string visitExpression(const ExpressionPtr& expr) -> std::string {
        if(!expr) return "int";

        if(auto binExpr = std::dynamic_pointer_cast<BinExpr>(expr)) {
            return visitBinExpr(binExpr);
        } else if(auto unExpr = std::dynamic_pointer_cast<UnExpr>(expr)) {
            return visitUnExpr(unExpr);
        } else if(auto varExpr = std::dynamic_pointer_cast<VarExpr>(expr)) {
            return visitVarExpr(varExpr);
        } else if(auto constExpr = std::dynamic_pointer_cast<ConstExpr>(expr)) {
            return constExpr->dataType;
        } else if(auto callExpr = std::dynamic_pointer_cast<CallExpr>(expr)) {
            return visitCallExpr(callExpr);
        }

        return "int";
    }

    std::string visitBinExpr(const std::shared_ptr<BinExpr>& expr) {
        auto leftType = visitExpression(expr->left);
        auto rightType = visitExpression(expr->right);

        // Comparison operators return bool
        if(expr->op == BinOp::EQ || expr->op == BinOp::NE ||
           expr->op == BinOp::LT || expr->op == BinOp::GT ||
           expr->op == BinOp::LE || expr->op == BinOp::GE) {
            return "bool";
        }

        // Logical operators require bool operands
        if(expr->op == BinOp::AND || expr->op == BinOp::OR) {
            if(leftType != "bool") {
                warnings.push_back("Logical operator expects boolean, got " + leftType);
            }
            if(rightType != "bool") {
                warnings.push_back("Logical operator expects boolean, got " + rightType);
            }
            return "bool";
        }

        // Arithmetic operators require int operands
        if(leftType != rightType) {
            warnings.push_back("Type mismatch in binary operation");
        }

        return leftType;
    }

    std::string visitUnExpr(const std::shared_ptr<UnExpr>& expr) {
        auto opType = visitExpression(expr->operand);

        if(expr->op == UnOp::NEG) {
            if(opType != "int") {
                warnings.push_back("Unary minus expects int, got " + opType);
            }
            return "int";
        } else if(expr->op == UnOp::NOT) {
            if(opType != "bool") {
                warnings.push_back("Logical not expects bool, got " + opType);
            }
            return "bool";
        }

        return opType;
    }

    std::string visitVarExpr(const std::shared_ptr<VarExpr>& expr) {
        if(!symbolTable.count(expr->name)) {
            errors.push_back("Undefined variable '" + expr->name + "'");
            return "int";
        }

        if(!symbolTable[expr->name].initialized) {
            warnings.push_back("Variable '" + expr->name + "' may be uninitialized");
        }

        return symbolTable[expr->name].type;
    }

    std::string visitCallExpr(const std::shared_ptr<CallExpr>& expr) {
        // Built-in functions
        if(expr->funcName == "print") return "int";  // print returns nothing effectively
        
        // User-defined functions (simplified)
        return "int";
    }
};
