#include <iostream>
#include <fstream>
#include <memory>
#include "lexer.h"
#include "parser.h"

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source.cpp>\n";
        return 1;
    }
    auto source = readFile(argv[1]);
    try {
        Lexer lexer(source);
        Parser parser(lexer);
        auto ast = parser.parseProgram();
        std::cout << "Parsing succeeded\n";
        ast->print(0);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}