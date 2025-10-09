#include "lexical_analyzer.h"
#include "parser.h"
#include <fstream>
#include <iostream>

std::string readFile(const char* path) {
    std::ifstream in(path);
    return { std::istreambuf_iterator<char>(in),
             std::istreambuf_iterator<char>() };
}

void printAST(const std::shared_ptr<ASTNode>& node, int d=0) {
    std::cout<<std::string(d*2,' ')<<node->name<<"\n";
    for(auto& c:node->children) printAST(c,d+1);
}

int main(int argc,char* argv[]){
    if(argc<2) { std::cerr<<"Usage: "<<argv[0]<<" file\n"; return 1;}
    std::string src=readFile(argv[1]);
    try {
        LexicalAnalyzer lex(src);
        Parser parser(lex);
        auto ast=parser.parseProgram();
        std::cout<<"Parsing successful\nAST:\n";
        printAST(ast);
    } catch(...) {
        return 1;
    }
}
