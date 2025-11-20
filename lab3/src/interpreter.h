/**
 * @file interpreter.h
 * @brief TAC Interpreter - Executes Three-Address Code
 */

#pragma once

#include "ir.h"
#include <unordered_map>
#include <iostream>

class Interpreter {
private:
    std::unordered_map<std::string, int> variables;
    std::vector<std::string> output;
    int pc = 0;  // Program counter

public:
    bool execute(const IRProgram& program) {
        variables.clear();
        output.clear();
        pc = 0;

        // Initialize variables
        for(const auto& [var, type] : program.variableTypes) {
            variables[var] = 0;
        }

        try {
            while(pc < static_cast<int>(program.instructions.size())) {
                const auto& instr = program.instructions[pc];

                switch(instr.type) {
                    case InstrType::BIN_OP:
                        executeBinOp(instr);
                        break;
                    case InstrType::UN_OP:
                        executeUnOp(instr);
                        break;
                    case InstrType::ASSIGN:
                        executeAssign(instr);
                        break;
                    case InstrType::LABEL:
                        // Labels don't execute anything
                        break;
                    case InstrType::GOTO:
                        pc = findLabel(program, instr.label) - 1;
                        break;
                    case InstrType::IF_GOTO: {
                        int condValue = getValue(instr.op1);
                        if(condValue == 0) {
                            pc = findLabel(program, instr.label) - 1;
                        }
                        break;
                    }
                    case InstrType::PRINT: {
                        int value = getValue(instr.op1);
                        printf("%d\n", value);
                        output.push_back(std::to_string(value));
                        break;
                    }
                    case InstrType::RETURN:
                        return true;
                    case InstrType::CALL:
                        // Built-in print function handling
                        if(instr.funcName == "print" && !instr.args.empty()) {
                            int value = getValue(instr.args[0]);
                            printf("%d\n", value);
                            output.push_back(std::to_string(value));
                        }
                        break;
                    case InstrType::NOP:
                        break;
                }

                pc++;
            }
            return true;
        } catch(const std::exception& e) {
            fprintf(stderr, "Runtime error: %s\n", e.what());
            return false;
        }
    }

    const std::vector<std::string>& getOutput() const {
        return output;
    }

private:
    int getValue(const Operand& op) {
        if(op.isConst()) {
            return op.value;
        }
        if(op.isVar()) {
            auto it = variables.find(op.str());
            if(it != variables.end()) {
                return it->second;
            }
            throw std::runtime_error("Undefined variable: " + op.str());
        }
        return 0;
    }

    void setValue(const Operand& op, int value) {
        if(op.isVar()) {
            variables[op.str()] = value;
        }
    }

    void executeBinOp(const Instruction& instr) {
        int left = getValue(instr.op1);
        int right = getValue(instr.op2);
        int result = 0;

        switch(instr.binOp) {
            case BinOp::ADD: result = left + right; break;
            case BinOp::SUB: result = left - right; break;
            case BinOp::MUL: result = left * right; break;
            case BinOp::DIV:
                if(right == 0) throw std::runtime_error("Division by zero");
                result = left / right;
                break;
            case BinOp::MOD:
                if(right == 0) throw std::runtime_error("Modulo by zero");
                result = left % right;
                break;
            case BinOp::EQ: result = (left == right) ? 1 : 0; break;
            case BinOp::NE: result = (left != right) ? 1 : 0; break;
            case BinOp::LT: result = (left < right) ? 1 : 0; break;
            case BinOp::GT: result = (left > right) ? 1 : 0; break;
            case BinOp::LE: result = (left <= right) ? 1 : 0; break;
            case BinOp::GE: result = (left >= right) ? 1 : 0; break;
            case BinOp::AND: result = (left && right) ? 1 : 0; break;
            case BinOp::OR: result = (left || right) ? 1 : 0; break;
        }

        setValue(instr.result, result);
    }

    void executeUnOp(const Instruction& instr) {
        int operand = getValue(instr.op1);
        int result = 0;

        if(instr.unOp == UnOp::NEG) {
            result = -operand;
        } else if(instr.unOp == UnOp::NOT) {
            result = operand ? 0 : 1;
        }

        setValue(instr.result, result);
    }

    void executeAssign(const Instruction& instr) {
        int value = getValue(instr.op1);
        setValue(instr.result, value);
    }

    int findLabel(const IRProgram& program, const std::string& label) {
        for(size_t i = 0; i < program.instructions.size(); i++) {
            if(program.instructions[i].type == InstrType::LABEL &&
               program.instructions[i].label == label) {
                return i;
            }
        }
        throw std::runtime_error("Label not found: " + label);
    }
};
