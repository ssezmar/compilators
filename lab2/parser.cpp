#include "parser.h"
#include <chrono>
#include <iomanip>

// Реализация ASTNode
void ASTNode::print(int depth, std::ostream& out) const {
    std::string indent(depth * 2, ' ');
    out << indent << name;
    if (!value.empty()) {
        out << ": \"" << value << "\"";
    }
    if (line > 0) {
        out << " [" << line << ":" << col << "]";
    }
    out << "\n";

    for (const auto& child : children) {
        child->print(depth + 1, out);
    }
}

void ASTNode::printToLog(std::ofstream& log, int depth) const {
    std::string indent(depth * 2, ' ');
    log << indent << "Node: " << name;
    if (!value.empty()) {
        log << " = \"" << value << "\"";
    }
    if (line > 0) {
        log << " at [" << line << ":" << col << "]";
    }
    log << " (children: " << children.size() << ")\n";

    for (const auto& child : children) {
        child->printToLog(log, depth + 1);
    }
}

// Реализация Parser
Parser::Parser(Lexer& lex, const std::string& logFile)
    : lex_(lex), depth_(0), loggingEnabled_(!logFile.empty())
{
    if (loggingEnabled_) {
        logFile_.open(logFile);
        if (logFile_.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            logFile_ << "=== SYNTAX ANALYSIS LOG ===\n";
            logFile_ << "Start time: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
            logFile_ << "============================\n\n";
        }
    }
    advance(); // Получить первый токен
}

Parser::~Parser() {
    if (logFile_.is_open()) {
        logFile_ << "\n=== PARSING COMPLETED ===\n";
        logFile_ << "Total nodes created: " << stats.totalNodes << "\n";
        logFile_ << "Errors encountered: " << stats.errors << "\n";
        logFile_ << "Parse time: " << std::fixed << std::setprecision(3) 
                 << stats.parseTime << " ms\n";
        logFile_.close();
    }
}

void Parser::advance() {
    current_ = lex_.next();
    if (loggingEnabled_ && logFile_.is_open()) {
        logFile_ << "Token: " << current_.toString() << "\n";
    }
}

void Parser::expect(TokenType type) {
    if (current_.type != type) {
        std::string expected = Token(type, "", 0, 0).typeToString();
        std::string found = current_.typeToString() + " \"" + current_.text + "\"";
        error("Expected " + expected + ", found " + found);
    }
    advance();
}

bool Parser::accept(TokenType type) {
    if (current_.type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::error(const std::string& msg) {
    stats.errors++;
    if (loggingEnabled_ && logFile_.is_open()) {
        logFile_ << "ERROR at " << current_.line << ":" << current_.col 
                 << " - " << msg << "\n";
    }
    throw ParseError(msg, current_.line, current_.col);
}

void Parser::logEntry(const std::string& rule, const std::string& msg) {
    if (loggingEnabled_ && logFile_.is_open()) {
        std::string indent(depth_ * 2, ' ');
        logFile_ << indent << "ENTER " << rule;
        if (!msg.empty()) {
            logFile_ << " (" << msg << ")";
        }
        logFile_ << " at " << current_.toString() << "\n";
    }
    depth_++;
}

void Parser::logExit(const std::string& rule, bool success) {
    depth_--;
    if (loggingEnabled_ && logFile_.is_open()) {
        std::string indent(depth_ * 2, ' ');
        logFile_ << indent << "EXIT " << rule << " - " 
                 << (success ? "SUCCESS" : "FAILED") << "\n";
    }
}

// Проверка, является ли токен типом данных
bool Parser::isTypeKeyword(const std::string& text) {
    static const std::vector<std::string> types = {
        "int", "float", "double", "char", "bool", "void", "string"
    };
    return std::find(types.begin(), types.end(), text) != types.end();
}

// Основная функция парсинга
std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto start = std::chrono::high_resolution_clock::now();

    try {
        auto result = parseProgram_();

        auto end = std::chrono::high_resolution_clock::now();
        stats.parseTime = std::chrono::duration<double, std::milli>(end - start).count();

        return result;
    } catch (const ParseError& e) {
        auto end = std::chrono::high_resolution_clock::now();
        stats.parseTime = std::chrono::duration<double, std::milli>(end - start).count();
        throw;
    }
}

std::shared_ptr<ASTNode> Parser::parseProgram_() {
    logEntry("Program");
    auto node = std::make_shared<ASTNode>("Program", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseDeclarationList());

    if (current_.type != T_EOF) {
        error("Expected end of file");
    }

    logExit("Program", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseDeclarationList() {
    logEntry("DeclarationList");
    auto node = std::make_shared<ASTNode>("DeclarationList", "", current_.line, current_.col);
    stats.totalNodes++;

    while (current_.type != T_EOF) {
        if (current_.type == T_KEYWORD && isTypeKeyword(current_.text)) {
            node->addChild(parseDeclaration());
        } else {
            break;
        }
    }

    logExit("DeclarationList", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseDeclaration() {
    logEntry("Declaration");

    // Сохраняем информацию о типе
    if (current_.type != T_KEYWORD) {
        error("Expected type keyword");
    }

    if (!isTypeKeyword(current_.text)) {
        error("Invalid type: " + current_.text);
    }

    auto typeToken = current_;
    advance(); // Пропускаем тип

    if (current_.type != T_IDENTIFIER) {
        error("Expected identifier after type");
    }

    auto idToken = current_;
    advance(); // Пропускаем идентификатор

    std::shared_ptr<ASTNode> result;

    // Определяем, что это - функция или переменная
    if (current_.type == T_DELIMITER && current_.text == "(") {
        // Это объявление функции
        result = std::make_shared<ASTNode>("FunctionDeclaration", "", typeToken.line, typeToken.col);
        stats.totalNodes++;

        // Добавляем тип
        auto typeNode = std::make_shared<ASTNode>("Type", typeToken.text, typeToken.line, typeToken.col);
        stats.totalNodes++;
        result->addChild(typeNode);

        // Добавляем имя функции
        auto nameNode = std::make_shared<ASTNode>("FunctionName", idToken.text, idToken.line, idToken.col);
        stats.totalNodes++;
        result->addChild(nameNode);

        // Парсим остальную часть функции
        expect(T_DELIMITER); // (

        if (current_.type != T_DELIMITER || current_.text != ")") {
            result->addChild(parseParameterList());
        }

        expect(T_DELIMITER); // )

        result->addChild(parseCompoundStatement());

    } else {
        // Это объявление переменной
        result = std::make_shared<ASTNode>("VarDeclaration", "", typeToken.line, typeToken.col);
        stats.totalNodes++;

        // Добавляем тип
        auto typeNode = std::make_shared<ASTNode>("Type", typeToken.text, typeToken.line, typeToken.col);
        stats.totalNodes++;
        result->addChild(typeNode);

        // Добавляем идентификатор
        auto idNode = std::make_shared<ASTNode>("Identifier", idToken.text, idToken.line, idToken.col);
        stats.totalNodes++;
        result->addChild(idNode);

        // Опциональная инициализация
        if (current_.type == T_OPERATOR && current_.text == "=") {
            advance();
            result->addChild(parseExpression());
        }

        expect(T_DELIMITER); // ;
    }

    logExit("Declaration", true);
    return result;
}

std::shared_ptr<ASTNode> Parser::parseVarDeclaration() {
    logEntry("VarDeclaration");
    auto node = std::make_shared<ASTNode>("VarDeclaration", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseType());

    if (current_.type != T_IDENTIFIER) {
        error("Expected identifier in variable declaration");
    }

    auto idNode = std::make_shared<ASTNode>("Identifier", current_.text, current_.line, current_.col);
    stats.totalNodes++;
    node->addChild(idNode);
    advance();

    // Опциональная инициализация
    if (current_.type == T_OPERATOR && current_.text == "=") {
        advance();
        node->addChild(parseExpression());
    }

    expect(T_DELIMITER); // ;

    logExit("VarDeclaration", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseFunctionDeclaration() {
    logEntry("FunctionDeclaration");
    auto node = std::make_shared<ASTNode>("FunctionDeclaration", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseType());

    if (current_.type != T_IDENTIFIER) {
        error("Expected function name");
    }

    auto nameNode = std::make_shared<ASTNode>("FunctionName", current_.text, current_.line, current_.col);
    stats.totalNodes++;
    node->addChild(nameNode);
    advance();

    expect(T_DELIMITER); // (

    if (current_.type != T_DELIMITER || current_.text != ")") {
        node->addChild(parseParameterList());
    }

    expect(T_DELIMITER); // )

    node->addChild(parseCompoundStatement());

    logExit("FunctionDeclaration", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseParameterList() {
    logEntry("ParameterList");
    auto node = std::make_shared<ASTNode>("ParameterList", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseParameter());

    while (current_.type == T_DELIMITER && current_.text == ",") {
        advance();
        node->addChild(parseParameter());
    }

    logExit("ParameterList", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseParameter() {
    logEntry("Parameter");
    auto node = std::make_shared<ASTNode>("Parameter", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseType());

    if (current_.type != T_IDENTIFIER) {
        error("Expected parameter name");
    }

    auto nameNode = std::make_shared<ASTNode>("ParameterName", current_.text, current_.line, current_.col);
    stats.totalNodes++;
    node->addChild(nameNode);
    advance();

    logExit("Parameter", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseType() {
    logEntry("Type");

    if (current_.type != T_KEYWORD) {
        error("Expected type keyword");
    }

    if (!isTypeKeyword(current_.text)) {
        error("Invalid type: " + current_.text);
    }

    auto node = std::make_shared<ASTNode>("Type", current_.text, current_.line, current_.col);
    stats.totalNodes++;
    advance();

    logExit("Type", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseCompoundStatement() {
    logEntry("CompoundStatement");
    auto node = std::make_shared<ASTNode>("CompoundStatement", "", current_.line, current_.col);
    stats.totalNodes++;

    expect(T_DELIMITER); // {

    node->addChild(parseStatementList());

    expect(T_DELIMITER); // }

    logExit("CompoundStatement", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseStatementList() {
    logEntry("StatementList");
    auto node = std::make_shared<ASTNode>("StatementList", "", current_.line, current_.col);
    stats.totalNodes++;

    while (current_.type != T_DELIMITER || current_.text != "}") {
        if (current_.type == T_EOF) {
            error("Expected '}' but reached end of file");
        }
        node->addChild(parseStatement());
    }

    logExit("StatementList", true);
    return node;
}

// ИСПРАВЛЕННАЯ ВЕРСИЯ parseStatement - теперь поддерживает объявления переменных
std::shared_ptr<ASTNode> Parser::parseStatement() {
    logEntry("Statement");
    std::shared_ptr<ASTNode> result;

    if (current_.type == T_KEYWORD && current_.text == "if") {
        result = parseIfStatement();
    } else if (current_.type == T_KEYWORD && current_.text == "while") {
        result = parseWhileStatement();
    } else if (current_.type == T_KEYWORD && current_.text == "return") {
        result = parseReturnStatement();
    } else if (current_.type == T_DELIMITER && current_.text == "{") {
        result = parseCompoundStatement();
    } else if (current_.type == T_KEYWORD && isTypeKeyword(current_.text)) {
        // ДОБАВЛЕНО: Поддержка объявлений переменных внутри блоков
        result = parseVarDeclaration();
    } else {
        result = parseExpressionStatement();
    }

    logExit("Statement", true);
    return result;
}

std::shared_ptr<ASTNode> Parser::parseExpressionStatement() {
    logEntry("ExpressionStatement");
    auto node = std::make_shared<ASTNode>("ExpressionStatement", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseExpression());
    expect(T_DELIMITER); // ;

    logExit("ExpressionStatement", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    logEntry("IfStatement");
    auto node = std::make_shared<ASTNode>("IfStatement", "", current_.line, current_.col);
    stats.totalNodes++;

    expect(T_KEYWORD); // if
    expect(T_DELIMITER); // (

    node->addChild(parseExpression());

    expect(T_DELIMITER); // )

    node->addChild(parseStatement());

    if (current_.type == T_KEYWORD && current_.text == "else") {
        advance();
        node->addChild(parseStatement());
    }

    logExit("IfStatement", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    logEntry("WhileStatement");
    auto node = std::make_shared<ASTNode>("WhileStatement", "", current_.line, current_.col);
    stats.totalNodes++;

    expect(T_KEYWORD); // while
    expect(T_DELIMITER); // (

    node->addChild(parseExpression());

    expect(T_DELIMITER); // )

    node->addChild(parseStatement());

    logExit("WhileStatement", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseReturnStatement() {
    logEntry("ReturnStatement");
    auto node = std::make_shared<ASTNode>("ReturnStatement", "", current_.line, current_.col);
    stats.totalNodes++;

    expect(T_KEYWORD); // return

    // Опциональное выражение
    if (current_.type != T_DELIMITER || current_.text != ";") {
        node->addChild(parseExpression());
    }

    expect(T_DELIMITER); // ;

    logExit("ReturnStatement", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    logEntry("Expression");
    auto result = parseLogicalOrExpression();
    logExit("Expression", true);
    return result;
}

std::shared_ptr<ASTNode> Parser::parseLogicalOrExpression() {
    logEntry("LogicalOrExpression");
    auto left = parseLogicalAndExpression();

    while (current_.type == T_OPERATOR && current_.text == "||") {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("BinaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(left);
        node->addChild(parseLogicalAndExpression());
        left = node;
    }

    logExit("LogicalOrExpression", true);
    return left;
}

std::shared_ptr<ASTNode> Parser::parseLogicalAndExpression() {
    logEntry("LogicalAndExpression");
    auto left = parseEqualityExpression();

    while (current_.type == T_OPERATOR && current_.text == "&&") {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("BinaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(left);
        node->addChild(parseEqualityExpression());
        left = node;
    }

    logExit("LogicalAndExpression", true);
    return left;
}

std::shared_ptr<ASTNode> Parser::parseEqualityExpression() {
    logEntry("EqualityExpression");
    auto left = parseRelationalExpression();

    while (current_.type == T_OPERATOR && 
           (current_.text == "==" || current_.text == "!=")) {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("BinaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(left);
        node->addChild(parseRelationalExpression());
        left = node;
    }

    logExit("EqualityExpression", true);
    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelationalExpression() {
    logEntry("RelationalExpression");
    auto left = parseAdditiveExpression();

    while (current_.type == T_OPERATOR && 
           (current_.text == "<" || current_.text == ">" || 
            current_.text == "<=" || current_.text == ">=")) {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("BinaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(left);
        node->addChild(parseAdditiveExpression());
        left = node;
    }

    logExit("RelationalExpression", true);
    return left;
}

std::shared_ptr<ASTNode> Parser::parseAdditiveExpression() {
    logEntry("AdditiveExpression");
    auto left = parseMultiplicativeExpression();

    while (current_.type == T_OPERATOR && 
           (current_.text == "+" || current_.text == "-")) {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("BinaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(left);
        node->addChild(parseMultiplicativeExpression());
        left = node;
    }

    logExit("AdditiveExpression", true);
    return left;
}

std::shared_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    logEntry("MultiplicativeExpression");
    auto left = parseUnaryExpression();

    while (current_.type == T_OPERATOR && 
           (current_.text == "*" || current_.text == "/" || current_.text == "%")) {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("BinaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(left);
        node->addChild(parseUnaryExpression());
        left = node;
    }

    logExit("MultiplicativeExpression", true);
    return left;
}

std::shared_ptr<ASTNode> Parser::parseUnaryExpression() {
    logEntry("UnaryExpression");

    if (current_.type == T_OPERATOR && 
        (current_.text == "!" || current_.text == "-" || current_.text == "+")) {
        auto op = current_.text;
        auto opLine = current_.line;
        auto opCol = current_.col;
        advance();

        auto node = std::make_shared<ASTNode>("UnaryOp", op, opLine, opCol);
        stats.totalNodes++;
        node->addChild(parseUnaryExpression());

        logExit("UnaryExpression", true);
        return node;
    }

    auto result = parsePrimaryExpression();
    logExit("UnaryExpression", true);
    return result;
}

std::shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    logEntry("PrimaryExpression");
    std::shared_ptr<ASTNode> result;

    if (current_.type == T_IDENTIFIER) {
        // Проверяем, не вызов ли это функции
        Token next = lex_.peek();
        if (next.type == T_DELIMITER && next.text == "(") {
            result = parseFunctionCall();
        } else {
            result = std::make_shared<ASTNode>("Identifier", current_.text, current_.line, current_.col);
            stats.totalNodes++;
            advance();
        }
    } else if (current_.type == T_INTEGER) {
        result = std::make_shared<ASTNode>("IntegerLiteral", current_.text, current_.line, current_.col);
        stats.totalNodes++;
        advance();
    } else if (current_.type == T_FLOAT) {
        result = std::make_shared<ASTNode>("FloatLiteral", current_.text, current_.line, current_.col);
        stats.totalNodes++;
        advance();
    } else if (current_.type == T_STRING) {
        result = std::make_shared<ASTNode>("StringLiteral", current_.text, current_.line, current_.col);
        stats.totalNodes++;
        advance();
    } else if (current_.type == T_DELIMITER && current_.text == "(") {
        advance(); // (
        result = parseExpression();
        expect(T_DELIMITER); // )
    } else {
        error("Expected identifier, literal, or '('");
    }

    logExit("PrimaryExpression", true);
    return result;
}

std::shared_ptr<ASTNode> Parser::parseFunctionCall() {
    logEntry("FunctionCall");
    auto node = std::make_shared<ASTNode>("FunctionCall", current_.text, current_.line, current_.col);
    stats.totalNodes++;

    advance(); // function name
    expect(T_DELIMITER); // (

    if (current_.type != T_DELIMITER || current_.text != ")") {
        node->addChild(parseArgumentList());
    }

    expect(T_DELIMITER); // )

    logExit("FunctionCall", true);
    return node;
}

std::shared_ptr<ASTNode> Parser::parseArgumentList() {
    logEntry("ArgumentList");
    auto node = std::make_shared<ASTNode>("ArgumentList", "", current_.line, current_.col);
    stats.totalNodes++;

    node->addChild(parseExpression());

    while (current_.type == T_DELIMITER && current_.text == ",") {
        advance();
        node->addChild(parseExpression());
    }

    logExit("ArgumentList", true);
    return node;
}