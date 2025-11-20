/**
 * @file optimizer.h
 * @brief Code Optimizer - Performs dead code elimination and constant folding
 */

#pragma once

#include "ir.h"
#include <unordered_set>
#include <algorithm>

class Optimizer {
public:
    IRProgram optimize(const IRProgram& input) {
        IRProgram result = input;
        
        // Pass 1: Constant folding
        result = foldConstants(result);
        
        // Pass 2: Dead code elimination
        result = eliminateDeadCode(result);
        
        return result;
    }

private:
    IRProgram foldConstants(const IRProgram& input) {
        IRProgram result = input;

        for(auto& instr : result.instructions) {
            if(instr.type == InstrType::BIN_OP) {
                if(instr.op1.isConst() && instr.op2.isConst()) {
                    int val1 = instr.op1.value;
                    int val2 = instr.op2.value;
                    int foldedValue = 0;

                    switch(instr.binOp) {
                        case BinOp::ADD: foldedValue = val1 + val2; break;
                        case BinOp::SUB: foldedValue = val1 - val2; break;
                        case BinOp::MUL: foldedValue = val1 * val2; break;
                        case BinOp::DIV: foldedValue = (val2 != 0) ? val1 / val2 : 0; break;
                        case BinOp::MOD: foldedValue = (val2 != 0) ? val1 % val2 : 0; break;
                        case BinOp::EQ: foldedValue = (val1 == val2) ? 1 : 0; break;
                        case BinOp::NE: foldedValue = (val1 != val2) ? 1 : 0; break;
                        case BinOp::LT: foldedValue = (val1 < val2) ? 1 : 0; break;
                        case BinOp::GT: foldedValue = (val1 > val2) ? 1 : 0; break;
                        case BinOp::LE: foldedValue = (val1 <= val2) ? 1 : 0; break;
                        case BinOp::GE: foldedValue = (val1 >= val2) ? 1 : 0; break;
                        case BinOp::AND: foldedValue = (val1 && val2) ? 1 : 0; break;
                        case BinOp::OR: foldedValue = (val1 || val2) ? 1 : 0; break;
                    }

                    // Replace with constant assignment
                    instr.type = InstrType::ASSIGN;
                    instr.op1 = Operand(foldedValue);
                }
            } else if(instr.type == InstrType::UN_OP) {
                if(instr.op1.isConst()) {
                    int val = instr.op1.value;
                    int foldedValue = 0;

                    if(instr.unOp == UnOp::NEG) {
                        foldedValue = -val;
                    } else if(instr.unOp == UnOp::NOT) {
                        foldedValue = val ? 0 : 1;
                    }

                    instr.type = InstrType::ASSIGN;
                    instr.op1 = Operand(foldedValue);
                }
            }
        }

        return result;
    }

    IRProgram eliminateDeadCode(const IRProgram& input) {
        IRProgram result = input;

        // Find all variables that are used
        std::unordered_set<std::string> usedVars;
        std::unordered_set<std::string> assignedVars;

        // First pass: find all uses and assignments
        for(const auto& instr : result.instructions) {
            // Mark assignments
            if(instr.type == InstrType::ASSIGN ||
               instr.type == InstrType::BIN_OP ||
               instr.type == InstrType::UN_OP) {
                assignedVars.insert(instr.result.str());
            }

            // Mark uses
            if(instr.type == InstrType::PRINT) {
                usedVars.insert(instr.op1.str());
            } else if(instr.type == InstrType::RETURN) {
                usedVars.insert(instr.op1.str());
            } else if(instr.type == InstrType::IF_GOTO) {
                usedVars.insert(instr.op1.str());
            } else if(instr.type == InstrType::BIN_OP) {
                usedVars.insert(instr.op1.str());
                usedVars.insert(instr.op2.str());
            } else if(instr.type == InstrType::UN_OP) {
                usedVars.insert(instr.op1.str());
            } else if(instr.type == InstrType::ASSIGN) {
                usedVars.insert(instr.op1.str());
            } else if(instr.type == InstrType::CALL) {
                for(const auto& arg : instr.args) {
                    usedVars.insert(arg.str());
                }
            }
        }

        // Second pass: remove dead assignments
        // (assignments to variables that are never used)
        std::vector<Instruction> filtered;
        for(const auto& instr : result.instructions) {
            bool isDead = false;

            // Check if this is an assignment to an unused variable
            if((instr.type == InstrType::ASSIGN ||
                instr.type == InstrType::BIN_OP ||
                instr.type == InstrType::UN_OP) &&
               assignedVars.count(instr.result.str()) &&
               !usedVars.count(instr.result.str())) {
                isDead = true;
            }

            if(!isDead) {
                filtered.push_back(instr);
            }
        }

        result.instructions = filtered;
        return result;
    }
};
