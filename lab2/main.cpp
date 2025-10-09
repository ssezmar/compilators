#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void saveASTToFile(const std::shared_ptr<ASTNode>& ast, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        ast->print(0, file);
        file.close();
        std::cout << "AST saved to: " << filename << std::endl;
    }
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <source_file> [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --ast <file>     Save AST to file\n";
    std::cout << "  --log <file>     Save parsing log to file\n";
    std::cout << "  --no-output      Don't print AST to console\n";
    std::cout << "  --help           Show this help\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string sourceFile = argv[1];
    std::string astFile = "";
    std::string logFile = "parse.log";
    bool printAST = true;

    // Разбор аргументов командной строки
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--ast" && i + 1 < argc) {
            astFile = argv[++i];
        } else if (arg == "--log" && i + 1 < argc) {
            logFile = argv[++i];
        } else if (arg == "--no-output") {
            printAST = false;
        }
    }

    try {
        // Чтение исходного файла
        std::cout << "Reading source file: " << sourceFile << "\n";
        std::string source = readFile(sourceFile);

        // Создание лексера и парсера
        Lexer lexer(source);
        Parser parser(lexer, logFile);

        std::cout << "Starting syntax analysis...\n";
        auto startTime = std::chrono::high_resolution_clock::now();

        // Парсинг
        auto ast = parser.parseProgram();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(endTime - startTime);

        // Вывод результатов
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "PARSING COMPLETED SUCCESSFULLY\n";
        std::cout << std::string(50, '=') << "\n";
        std::cout << "Parse time: " << std::fixed << std::setprecision(2) << duration.count() << " ms\n";
        std::cout << "Nodes created: " << parser.stats.totalNodes << "\n";
        std::cout << "Log saved to: " << logFile << "\n";

        // Вывод AST
        if (printAST) {
            std::cout << "\nAbstract Syntax Tree:\n";
            std::cout << std::string(30, '-') << "\n";
            ast->print();
        }

        // Сохранение AST в файл
        if (!astFile.empty()) {
            saveASTToFile(ast, astFile);
        }

        return 0;

    } catch (const ParseError& e) {
        std::cerr << "\n" << std::string(50, '=') << "\n";
        std::cerr << "PARSING FAILED\n";
        std::cerr << std::string(50, '=') << "\n";
        std::cerr << "Parse Error at line " << e.line << ", column " << e.col << ":\n";
        std::cerr << "  " << e.what() << "\n";

        if (!e.expected.empty() && !e.found.empty()) {
            std::cerr << "  Expected: " << e.expected << "\n";
            std::cerr << "  Found: " << e.found << "\n";
        }

        std::cerr << "\nCheck the log file for detailed parsing trace: " << logFile << "\n";
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}