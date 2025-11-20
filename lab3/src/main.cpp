/**
 * @file main.cpp
 * @brief Main Compiler Program - Integrates Lexer, Parser, Semantic Analysis, 
 *        Code Generation, Optimization and Interpretation
 * 
 * @date 2025
 * @version 1.0
 * @author Lab3
 */

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"
#include "optimizer.h"
#include "interpreter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

void printUsage(const char* programName) {
    fprintf(stderr, "Usage: %s <source_file> [options]\n", programName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -ast              Print AST\n");
    fprintf(stderr, "  -tokens           Print tokens\n");
    fprintf(stderr, "  -noopt            Disable optimization\n");
    fprintf(stderr, "  -o <file>         Output TAC to file\n");
}

std::string readFile(const char* filename) {
    std::ifstream file(filename);
    if(!file.is_open()) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printBanner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║       CODE GENERATOR - Three-Address Code Compiler       ║\n");
    printf("║              Programming Language Compiler               ║\n");
    printf("║                   Version 1.0 (Lab3)                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void printPhase(const char* phase) {
    printf("► %s\n", phase);
}

void printSuccess(const char* message) {
    printf("  ✓ %s\n", message);
}

void printError(const char* message) {
    printf("  ✗ %s\n", message);
}

void printWarning(const char* message) {
    printf("  ⚠ %s\n", message);
}

int main(int argc, char* argv[]) {
    printBanner();

    if(argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse arguments
    const char* sourceFile = argv[1];
    const char* outputFile = nullptr;
    bool printTokens = false;
    bool printAST = false;
    bool optimize = true;

    for(int i = 2; i < argc; i++) {
        if(strcmp(argv[i], "-tokens") == 0) {
            printTokens = true;
        } else if(strcmp(argv[i], "-ast") == 0) {
            printAST = true;
        } else if(strcmp(argv[i], "-noopt") == 0) {
            optimize = false;
        } else if(strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }

    printf("Input file: %s\n", sourceFile);
    if(optimize) printf("Optimization: Enabled\n");
    printf("\n");

    // ========== PHASE 1: LEXICAL ANALYSIS ==========
    printPhase("Phase 1: Lexical Analysis");
    std::string sourceCode = readFile(sourceFile);

    Lexer lexer(sourceCode);
    auto tokens = lexer.tokenize();

    printSuccess("Tokenization complete");
    printf("  Tokens generated: %zu\n", tokens.size() - 1);  // -1 for EOF

    if(printTokens) {
        printf("\n=== TOKEN LIST ===\n");
        for(const auto& token : tokens) {
            if(token.type == TokenType::END_OF_FILE) break;
            printf("  [%s] '%s' (line %d, col %d)\n",
                   token.typeString().c_str(), token.lexeme.c_str(),
                   token.line, token.column);
        }
        printf("\n");
    }

    // ========== PHASE 2: SYNTAX ANALYSIS ==========
    printPhase("Phase 2: Syntax Analysis");
    Parser parser(tokens);
    auto ast = parser.parse();

    if(!ast) {
        printError("Failed to parse program");
        return 1;
    }

    printSuccess("Parsing complete");
    printf("  Statements: %zu\n", ast->statements.size());

    // ========== PHASE 3: SEMANTIC ANALYSIS ==========
    printPhase("Phase 3: Semantic Analysis");
    SemanticAnalyzer semanticAnalyzer;
    bool semanticOK = semanticAnalyzer.analyze(ast);

    if(!semanticOK) {
        const auto& errors = semanticAnalyzer.getErrors();
        for(const auto& error : errors) {
            printError(error.c_str());
        }
        return 1;
    }

    printSuccess("Semantic analysis complete");

    const auto& warnings = semanticAnalyzer.getWarnings();
    for(const auto& warning : warnings) {
        printWarning(warning.c_str());
    }

    // ========== PHASE 4: CODE GENERATION ==========
    printPhase("Phase 4: Code Generation");
    CodeGenerator codegen(semanticAnalyzer);
    auto ir = codegen.generate(ast);

    printSuccess("Code generation complete");
    printf("  Instructions: %zu\n", ir.instructions.size());
    printf("  Variables: %zu\n", ir.variableTypes.size());

    // ========== PHASE 5: OPTIMIZATION ==========
    if(optimize) {
        printPhase("Phase 5: Optimization");
        Optimizer optimizer;
        auto optimizedIR = optimizer.optimize(ir);

        int removed = ir.instructions.size() - optimizedIR.instructions.size();
        if(removed > 0) {
            printf("  Dead code elimination: %d instructions removed\n", removed);
        }
        printSuccess("Optimization complete");
        ir = optimizedIR;
    }

    // ========== OUTPUT: THREE-ADDRESS CODE ==========
    printf("\n");
    ir.print();

    if(outputFile) {
        ir.saveToFile(outputFile);
        printf("TAC saved to: %s\n\n", outputFile);
    }

    // ========== PHASE 6: INTERPRETATION ==========
    printPhase("Phase 6: Interpretation");
    Interpreter interpreter;
    bool execOK = interpreter.execute(ir);

    if(!execOK) {
        printError("Execution failed");
        return 1;
    }

    printSuccess("Execution complete");
    const auto& output = interpreter.getOutput();
    if(!output.empty()) {
        printf("  Output lines: %zu\n", output.size());
    }

    // ========== SUMMARY ==========
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                 COMPILATION SUCCESSFUL                    ║\n");
    printf("║                                                            ║\n");
    printf("║  ✓ Lexical Analysis      ✓ Semantic Analysis              ║\n");
    printf("║  ✓ Syntax Analysis       ✓ Code Generation                ║\n");
    printf("║  ✓ Optimization          ✓ Interpretation                 ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return 0;
}
