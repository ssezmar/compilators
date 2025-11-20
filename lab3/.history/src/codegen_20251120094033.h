/**
 * @file codegen.h
 * @brief Code Generator - Transforms AST to Three-Address Code (TAC)
 */

#pragma once

#include "ir.h"
#include "parser.h"
#include "semantic.h"
#include <unordered_map>

class CodeGenerator {
private:
    IRProgram ir;
    int tempCounter = 0;
    int labelCounter = 0;
    SemanticAnalyzer& semanticAnalyzer;
    std::unordered_map<std::string, int> variableTypes;

public:
    CodeGenerator(SemanticAnalyzer& sem) : semanticAnalyzer(sem) {}

    IRProgram generate(const ProgramPtr& program) {
        ir.instructions.clear();
        tempCounter = 0;
        labelCounter = 0;

        if(!program) return ir;

        // First pass: collect all variable declarations
        for(const auto& stmt : program->statements) {
            if(auto decl = std::dynamic_pointer_cast<DeclStmt>(stmt)) {
                int typeCode = (decl->dataType == "int") ? 0 : 1;
                ir.variableTypes[decl->varName] = typeCode;
                variableTypes[decl->varName] = typeCode;
            }
        }

        // Second pass: generate code
        for(const auto& stmt : program->statements) {
            genStatement(stmt);
        }

        return ir;
    }

private:
    std::string genTemp() {
        return "t" + std::to_string(tempCounter++);
    }

    std::string genLabel() {
        return "L" + std::to_string(labelCounter++);
    }

    void emitInstruction(const Instruction& instr) {
        ir.addInstruction(instr);
    }

    void genStatement(const StatementPtr& stmt) {
        if(!stmt) return;

        if(auto decl = std::dynamic_pointer_cast<DeclStmt>(stmt)) {
            genDecl(decl);
        } else if(auto assign = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
            genAssign(assign);
        } else if(auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            genIfStatement(ifStmt);
        } else if(auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            genWhileStatement(whileStmt);
        } else if(auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
            genForStatement(forStmt);
        } else if(auto block = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
            for(const auto& s : block->statements) {
                genStatement(s);
            }
        } else if(auto printStmt = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
            auto result = genExpression(printStmt->value);
            emitInstruction(Instruction::createPrint(result));
        } else if(auto retStmt = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            if(retStmt->value) {
                auto result = genExpression(retStmt->value);
                emitInstruction(Instruction::createReturn(result));
            } else {
                emitInstruction(Instruction::createReturn(Operand(0)));
            }
        }
    }

    void genDecl(const std::shared_ptr<DeclStmt>& decl) {
        int typeCode = (decl->dataType == "int") ? 0 : 1;
        ir.variableTypes[decl->varName] = typeCode;
        variableTypes[decl->varName] = typeCode;

        if(decl->initializer) {
            auto result = genExpression(decl->initializer);
            emitInstruction(Instruction::createAssign(
                Operand(decl->varName, Operand::Type::VAR), result));
        }
    }

    void genAssign(const std::shared_ptr<AssignStmt>& assign) {
        auto result = genExpression(assign->value);
        emitInstruction(Instruction::createAssign(
            Operand(assign->varName, Operand::Type::VAR), result));
    }

    void genIfStatement(const std::shared_ptr<IfStmt>& ifStmt) {
        auto condition = genExpression(ifStmt->condition);
        std::string elseLabel = genLabel();
        std::string endLabel = genLabel();

        emitInstruction(Instruction::createIfGoto(condition, elseLabel));

        // Then branch
        for(const auto& stmt : ifStmt->thenBranch) {
            genStatement(stmt);
        }

        if(!ifStmt->elseBranch.empty()) {
            emitInstruction(Instruction::createGoto(endLabel));
            emitInstruction(Instruction::createLabel(elseLabel));

            // Else branch
            for(const auto& stmt : ifStmt->elseBranch) {
                genStatement(stmt);
            }

            emitInstruction(Instruction::createLabel(endLabel));
        } else {
            emitInstruction(Instruction::createLabel(elseLabel));
        }
    }

    void genWhileStatement(const std::shared_ptr<WhileStmt>& whileStmt) {
        std::string loopLabel = genLabel();
        std::string endLabel = genLabel();

        emitInstruction(Instruction::createLabel(loopLabel));

        auto condition = genExpression(whileStmt->condition);
        emitInstruction(Instruction::createIfGoto(condition, endLabel));

        // Loop body
        for(const auto& stmt : whileStmt->body) {
            genStatement(stmt);
        }

        emitInstruction(Instruction::createGoto(loopLabel));
        emitInstruction(Instruction::createLabel(endLabel));
    }

    void genForStatement(const std::shared_ptr<ForStmt>& forStmt) {
        if(forStmt->init) {
            genStatement(forStmt->init);
        }

        std::string loopLabel = genLabel();
        std::string endLabel = genLabel();

        emitInstruction(Instruction::createLabel(loopLabel));

        if(forStmt->condition) {
            auto condition = genExpression(forStmt->condition);
            emitInstruction(Instruction::createIfGoto(condition, endLabel));
        }

        // Loop body
        for(const auto& stmt : forStmt->body) {
            genStatement(stmt);
        }

        if(forStmt->update) {
            genExpression(forStmt->update);
        }

        emitInstruction(Instruction::createGoto(loopLabel));
        emitInstruction(Instruction::createLabel(endLabel));
    }

    Operand genExpression(const ExpressionPtr& expr) -> Operand {
        if(!expr) return Operand(0);

        if(auto binExpr = std::dynamic_pointer_cast<BinExpr>(expr)) {
            return genBinExpr(binExpr);
        } else if(auto unExpr = std::dynamic_pointer_cast<UnExpr>(expr)) {
            return genUnExpr(unExpr);
        } else if(auto varExpr = std::dynamic_pointer_cast<VarExpr>(expr)) {
            return Operand(varExpr->name, Operand::Type::VAR);
        } else if(auto constExpr = std::dynamic_pointer_cast<ConstExpr>(expr)) {
            if(std::holds_alternative<int>(constExpr->value)) {
                int val = std::get<int>(constExpr->value);
                return Operand(val);
            } else {
                bool val = std::get<bool>(constExpr->value);
                return Operand(val ? 1 : 0);
            }
        } else if(auto callExpr = std::dynamic_pointer_cast<CallExpr>(expr)) {
            return genCallExpr(callExpr);
        }

        return Operand(0);
    }

    Operand genBinExpr(const std::shared_ptr<BinExpr>& expr) {
        auto left = genExpression(expr->left);
        auto right = genExpression(expr->right);
        std::string result = genTemp();

        auto instr = Instruction::createBinOp(
            Operand(result, Operand::Type::TEMP),
            expr->op, left, right);
        emitInstruction(instr);

        return Operand(result, Operand::Type::TEMP);
    }

    Operand genUnExpr(const std::shared_ptr<UnExpr>& expr) {
        auto operand = genExpression(expr->operand);
        std::string result = genTemp();

        auto instr = Instruction::createUnOp(
            Operand(result, Operand::Type::TEMP),
            expr->op, operand);
        emitInstruction(instr);

        return Operand(result, Operand::Type::TEMP);
    }

    Operand genCallExpr(const std::shared_ptr<CallExpr>& expr) {
        std::vector<Operand> args;
        for(const auto& arg : expr->args) {
            args.push_back(genExpression(arg));
        }

        std::string result = genTemp();
        auto instr = Instruction::createCall(
            Operand(result, Operand::Type::TEMP),
            expr->funcName, args);
        emitInstruction(instr);

        return Operand(result, Operand::Type::TEMP);
    }
};
