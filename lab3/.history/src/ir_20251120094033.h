/**
 * @file ir.h
 * @brief Intermediate Representation (Three-Address Code)
 * @author Lab3
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>

// Enumerations for IR operations
enum class BinOp {
    ADD, SUB, MUL, DIV, MOD,          // Arithmetic
    EQ, NE, LT, GT, LE, GE,          // Comparison
    AND, OR                            // Logical
};

enum class UnOp {
    NEG,                              // Unary minus
    NOT                               // Logical not
};

enum class InstrType {
    BIN_OP,                           // t1 = a + b
    UN_OP,                            // t1 = -a
    ASSIGN,                           // a = b
    CONST,                            // a = 10
    LABEL,                            // LABEL:
    GOTO,                             // goto LABEL
    IF_GOTO,                          // if cond goto LABEL
    CALL,                             // t1 = func(args)
    RETURN,                           // return value
    PRINT,                            // print(value)
    NOP                               // No operation
};

struct Operand {
    enum class Type { VAR, CONST, TEMP } type;
    std::string name;
    int value = 0;

    Operand() : type(Type::VAR), name("") {}
    Operand(const std::string& n, Type t = Type::VAR) 
        : type(t), name(n) {}
    Operand(int v) 
        : type(Type::CONST), name(std::to_string(v)), value(v) {}

    std::string str() const {
        return name;
    }

    bool isConst() const { return type == Type::CONST; }
    bool isTemp() const { return type == Type::TEMP; }
    bool isVar() const { return type == Type::VAR; }
};

struct Instruction {
    InstrType type;
    Operand result;
    Operand op1, op2;
    std::string label;
    BinOp binOp;
    UnOp unOp;
    std::vector<Operand> args;
    std::string funcName;

    Instruction() : type(InstrType::NOP), binOp(BinOp::ADD), unOp(UnOp::NEG) {}

    static Instruction createBinOp(const Operand& result, BinOp op, 
                                   const Operand& op1, const Operand& op2) {
        Instruction instr;
        instr.type = InstrType::BIN_OP;
        instr.result = result;
        instr.binOp = op;
        instr.op1 = op1;
        instr.op2 = op2;
        return instr;
    }

    static Instruction createUnOp(const Operand& result, UnOp op, 
                                  const Operand& operand) {
        Instruction instr;
        instr.type = InstrType::UN_OP;
        instr.result = result;
        instr.unOp = op;
        instr.op1 = operand;
        return instr;
    }

    static Instruction createAssign(const Operand& dst, const Operand& src) {
        Instruction instr;
        instr.type = InstrType::ASSIGN;
        instr.result = dst;
        instr.op1 = src;
        return instr;
    }

    static Instruction createLabel(const std::string& lbl) {
        Instruction instr;
        instr.type = InstrType::LABEL;
        instr.label = lbl;
        return instr;
    }

    static Instruction createGoto(const std::string& lbl) {
        Instruction instr;
        instr.type = InstrType::GOTO;
        instr.label = lbl;
        return instr;
    }

    static Instruction createIfGoto(const Operand& cond, const std::string& lbl) {
        Instruction instr;
        instr.type = InstrType::IF_GOTO;
        instr.op1 = cond;
        instr.label = lbl;
        return instr;
    }

    static Instruction createCall(const Operand& result, const std::string& func,
                                  const std::vector<Operand>& arguments) {
        Instruction instr;
        instr.type = InstrType::CALL;
        instr.result = result;
        instr.funcName = func;
        instr.args = arguments;
        return instr;
    }

    static Instruction createReturn(const Operand& value) {
        Instruction instr;
        instr.type = InstrType::RETURN;
        instr.op1 = value;
        return instr;
    }

    static Instruction createPrint(const Operand& value) {
        Instruction instr;
        instr.type = InstrType::PRINT;
        instr.op1 = value;
        return instr;
    }

    std::string toString() const;
};

// IR Program
struct IRProgram {
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, int> variableTypes;  // 0: int, 1: bool
    std::unordered_map<std::string, int> symbolTable;    // var -> line defined

    void addInstruction(const Instruction& instr) {
        instructions.push_back(instr);
    }

    void print() const;
    void saveToFile(const std::string& filename) const;
};

inline std::string BinOpToString(BinOp op) {
    switch(op) {
        case BinOp::ADD: return "+";
        case BinOp::SUB: return "-";
        case BinOp::MUL: return "*";
        case BinOp::DIV: return "/";
        case BinOp::MOD: return "%";
        case BinOp::EQ: return "==";
        case BinOp::NE: return "!=";
        case BinOp::LT: return "<";
        case BinOp::GT: return ">";
        case BinOp::LE: return "<=";
        case BinOp::GE: return ">=";
        case BinOp::AND: return "&&";
        case BinOp::OR: return "||";
    }
    return "?";
}

inline std::string UnOpToString(UnOp op) {
    return (op == UnOp::NEG) ? "-" : "!";
}

inline std::string Instruction::toString() const {
    std::string s;
    switch(type) {
        case InstrType::BIN_OP:
            s = result.str() + " = " + op1.str() + " " + 
                BinOpToString(binOp) + " " + op2.str();
            break;
        case InstrType::UN_OP:
            s = result.str() + " = " + UnOpToString(unOp) + op1.str();
            break;
        case InstrType::ASSIGN:
            s = result.str() + " = " + op1.str();
            break;
        case InstrType::LABEL:
            s = label + ":";
            break;
        case InstrType::GOTO:
            s = "goto " + label;
            break;
        case InstrType::IF_GOTO:
            s = "ifz " + op1.str() + " goto " + label;
            break;
        case InstrType::CALL: {
            s = result.str() + " = " + funcName + "(";
            for(size_t i = 0; i < args.size(); i++) {
                if(i > 0) s += ", ";
                s += args[i].str();
            }
            s += ")";
            break;
        }
        case InstrType::RETURN:
            s = "return " + op1.str();
            break;
        case InstrType::PRINT:
            s = "print(" + op1.str() + ")";
            break;
        case InstrType::NOP:
            s = "nop";
            break;
    }
    return s;
}

inline void IRProgram::print() const {
    std::cout << "\n=== THREE-ADDRESS CODE (TAC) ===\n";
    for(size_t i = 0; i < instructions.size(); i++) {
        printf("%3zu:  %s\n", i, instructions[i].toString().c_str());
    }
    std::cout << "\n=== VARIABLE TABLE ===\n";
    for(const auto& [var, type] : variableTypes) {
        std::string typeName = (type == 0) ? "int" : "bool";
        printf("  %s : %s\n", var.c_str(), typeName.c_str());
    }
    std::cout << "\n";
}

inline void IRProgram::saveToFile(const std::string& filename) const {
    FILE* f = fopen(filename.c_str(), "w");
    if(!f) return;
    
    fprintf(f, "=== THREE-ADDRESS CODE (TAC) ===\n\n");
    for(size_t i = 0; i < instructions.size(); i++) {
        fprintf(f, "%3zu:  %s\n", i, instructions[i].toString().c_str());
    }
    fprintf(f, "\n=== VARIABLE TABLE ===\n");
    for(const auto& [var, type] : variableTypes) {
        std::string typeName = (type == 0) ? "int" : "bool";
        fprintf(f, "  %s : %s\n", var.c_str(), typeName.c_str());
    }
    fclose(f);
}
