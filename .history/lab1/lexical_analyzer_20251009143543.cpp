#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include <sstream>

// Типы токенов
enum TokenType {
    KEYWORD,        // Ключевые слова
    IDENTIFIER,     // Идентификаторы
    INTEGER,        // Целые числа
    FLOAT,          // Вещественные числа
    STRING_LITERAL, // Строковые литералы
    OPERATOR,       // Операторы
    DELIMITER,      // Разделители
    COMMENT,        // Комментарии
    ERROR,          // Ошибки
    END_OF_FILE     // Конец файла
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    int line;
    int position;

    Token(TokenType t, const std::string& v, int l, int p) 
        : type(t), value(v), line(l), position(p) {}
};

// Конечный автомат для распознавания лексем
class FiniteAutomaton {
public:
    enum State {
        START,          // Начальное состояние
        IDENTIFIER_ST,  // Состояние идентификатора
        INTEGER_ST,     // Состояние целого числа  
        FLOAT_ST,       // Состояние вещественного числа
        STRING_ST,      // Состояние строки
        OPERATOR_ST,    // Состояние оператора
        COMMENT_LINE,   // Однострочный комментарий
        COMMENT_BLOCK,  // Многострочный комментарий
        COMMENT_BLOCK_END, // Окончание многострочного комментария
        ACCEPT,         // Принимающее состояние
        ERROR_ST        // Состояние ошибки
    };

    State currentState;

    FiniteAutomaton() : currentState(START) {}

    void reset() {
        currentState = START;
    }

    State transition(char c) {
        switch (currentState) {
            case START:
                if (std::isalpha(c) || c == '_') return IDENTIFIER_ST;
                if (std::isdigit(c)) return INTEGER_ST;
                if (c == '"') return STRING_ST;
                if (c == '/' && (c == '/' || c == '*')) return OPERATOR_ST;
                if (isOperatorChar(c)) return OPERATOR_ST;
                if (isDelimiterChar(c)) return ACCEPT;
                return ERROR_ST;

            case IDENTIFIER_ST:
                if (std::isalnum(c) || c == '_') return IDENTIFIER_ST;
                return ACCEPT;

            case INTEGER_ST:
                if (std::isdigit(c)) return INTEGER_ST;
                if (c == '.') return FLOAT_ST;
                return ACCEPT;

            case FLOAT_ST:
                if (std::isdigit(c)) return FLOAT_ST;
                return ACCEPT;

            case STRING_ST:
                if (c == '"') return ACCEPT;
                if (c == '\n') return ERROR_ST;
                return STRING_ST;

            default:
                return ERROR_ST;
        }
    }

private:
    bool isOperatorChar(char c) {
        return std::string("+-*/%=<>!&|").find(c) != std::string::npos;
    }

    bool isDelimiterChar(char c) {
        return std::string("(){}[];,.").find(c) != std::string::npos;
    }
};

// Лексический анализатор с комбинированными методами
class LexicalAnalyzer {
private:
    std::string input;
    size_t position;
    int currentLine;
    int linePosition;
    std::vector<Token> tokens;

    // Хеш-таблица для ключевых слов (метод хеширования - 5 баллов)
    std::unordered_set<std::string> keywords;

    // Конечный автомат (расширяемый автомат - 9 баллов)
    FiniteAutomaton automaton;

    // Таблица операторов для бинарного поиска (5 баллов)
    std::vector<std::string> operators;

public:
    LexicalAnalyzer() : position(0), currentLine(1), linePosition(1) {
        initializeKeywords();
        initializeOperators();
    }

    void initializeKeywords() {
        // Инициализация хеш-таблицы ключевых слов
        keywords = {
            "int", "float", "double", "char", "bool", "void",
            "if", "else", "while", "for", "do", "switch", "case", "default",
            "break", "continue", "return", "const", "static", "extern",
            "struct", "class", "public", "private", "protected",
            "true", "false", "null", "this", "new", "delete"
        };
    }

    void initializeOperators() {
        // Упорядоченный список операторов для бинарного поиска
        operators = {
            "!", "!=", "%", "%=", "&", "&&", "&=", "*", "*=", "+", "++", "+=",
            "-", "--", "-=", "/", "/=", "<", "<<", "<<=", "<=", "=", "==",
            ">", ">=", ">>", ">>=", "^", "^=", "|", "|=", "||", "~"
        };
        std::sort(operators.begin(), operators.end());
    }

    // Метод хеширования для поиска ключевых слов
    bool isKeyword(const std::string& word) {
        return keywords.find(word) != keywords.end();
    }

    // Бинарный поиск операторов
    bool isOperator(const std::string& op) {
        return std::binary_search(operators.begin(), operators.end(), op);
    }

    void skipWhitespace() {
        while (position < input.length() && std::isspace(input[position])) {
            if (input[position] == '\n') {
                currentLine++;
                linePosition = 1;
            } else {
                linePosition++;
            }
            position++;
        }
    }

    // Обработка комментариев (+2 балла)
    bool processComment() {
        if (position + 1 < input.length()) {
            if (input[position] == '/' && input[position + 1] == '/') {
                // Однострочный комментарий
                size_t start = position;
                while (position < input.length() && input[position] != '\n') {
                    position++;
                }
                tokens.emplace_back(COMMENT, input.substr(start, position - start), 
                                  currentLine, linePosition);
                return true;
            }
            else if (input[position] == '/' && input[position + 1] == '*') {
                // Многострочный комментарий
                size_t start = position;
                position += 2;
                while (position + 1 < input.length()) {
                    if (input[position] == '*' && input[position + 1] == '/') {
                        position += 2;
                        tokens.emplace_back(COMMENT, input.substr(start, position - start),
                                          currentLine, linePosition);
                        return true;
                    }
                    if (input[position] == '\n') {
                        currentLine++;
                        linePosition = 1;
                    }
                    position++;
                }
                // Незавершенный комментарий
                tokens.emplace_back(ERROR, "Unclosed comment", currentLine, linePosition);
                return true;
            }
        }
        return false;
    }

    std::string readIdentifier() {
        size_t start = position;
        automaton.reset();

        while (position < input.length()) {
            char c = input[position];
            FiniteAutomaton::State nextState = automaton.transition(c);

            if (nextState == FiniteAutomaton::ACCEPT) {
                break;
            } else if (nextState == FiniteAutomaton::ERROR_ST) {
                break;
            }

            automaton.currentState = nextState;
            position++;
            linePosition++;
        }

        return input.substr(start, position - start);
    }

    std::string readNumber() {
        size_t start = position;
        bool isFloat = false;

        // Читаем цифры
        while (position < input.length() && std::isdigit(input[position])) {
            position++;
            linePosition++;
        }

        // Проверяем на вещественное число
        if (position < input.length() && input[position] == '.') {
            isFloat = true;
            position++;
            linePosition++;

            // Читаем дробную часть
            while (position < input.length() && std::isdigit(input[position])) {
                position++;
                linePosition++;
            }
        }

        // Проверяем экспоненциальную форму
        if (position < input.length() && (input[position] == 'e' || input[position] == 'E')) {
            isFloat = true;
            position++;
            linePosition++;

            if (position < input.length() && (input[position] == '+' || input[position] == '-')) {
                position++;
                linePosition++;
            }

            while (position < input.length() && std::isdigit(input[position])) {
                position++;
                linePosition++;
            }
        }

        return input.substr(start, position - start);
    }

    std::string readString() {
        size_t start = position;
        position++; // Пропускаем открывающую кавычку
        linePosition++;

        while (position < input.length() && input[position] != '"') {
            if (input[position] == '\\') {
                position += 2; // Пропускаем экранированный символ
                linePosition += 2;
            } else {
                if (input[position] == '\n') {
                    // Ошибка: незавершенная строка
                    tokens.emplace_back(ERROR, "Unterminated string", currentLine, linePosition);
                    return "";
                }
                position++;
                linePosition++;
            }
        }

        if (position < input.length()) {
            position++; // Пропускаем закрывающую кавычку
            linePosition++;
        }

        return input.substr(start, position - start);
    }

    std::string readOperator() {
        size_t start = position;
        std::string op;

        // Пытаемся прочитать максимально длинный оператор
        op += input[position++];
        linePosition++;

        // Проверяем двухсимвольные операторы
        if (position < input.length()) {
            std::string twoChar = op + input[position];
            if (isOperator(twoChar)) {
                op = twoChar;
                position++;
                linePosition++;
            }
        }

        return op;
    }

    std::vector<Token> analyze(const std::string& sourceCode) {
        input = sourceCode;
        position = 0;
        currentLine = 1;
        linePosition = 1;
        tokens.clear();

        while (position < input.length()) {
            skipWhitespace();

            if (position >= input.length()) break;

            // Обработка комментариев
            if (processComment()) continue;

            char currentChar = input[position];
            int tokenLine = currentLine;
            int tokenPos = linePosition;

            if (std::isalpha(currentChar) || currentChar == '_') {
                // Идентификатор или ключевое слово
                std::string identifier = readIdentifier();
                TokenType type = isKeyword(identifier) ? KEYWORD : IDENTIFIER;
                tokens.emplace_back(type, identifier, tokenLine, tokenPos);
            }
            else if (std::isdigit(currentChar)) {
                // Число
                std::string number = readNumber();
                TokenType type = (number.find('.') != std::string::npos || 
                                number.find('e') != std::string::npos ||
                                number.find('E') != std::string::npos) ? FLOAT : INTEGER;
                tokens.emplace_back(type, number, tokenLine, tokenPos);
            }
            else if (currentChar == '"') {
                // Строковый литерал
                std::string str = readString();
                if (!str.empty()) {
                    tokens.emplace_back(STRING_LITERAL, str, tokenLine, tokenPos);
                }
            }
            else if (std::string("+-*/%=<>!&|^~").find(currentChar) != std::string::npos) {
                // Оператор
                std::string op = readOperator();
                tokens.emplace_back(OPERATOR, op, tokenLine, tokenPos);
            }
            else if (std::string("(){}[];,.").find(currentChar) != std::string::npos) {
                // Разделитель
                tokens.emplace_back(DELIMITER, std::string(1, currentChar), tokenLine, tokenPos);
                position++;
                linePosition++;
            }
            else {
                // Неизвестный символ
                tokens.emplace_back(ERROR, std::string(1, currentChar), tokenLine, tokenPos);
                position++;
                linePosition++;
            }
        }

        tokens.emplace_back(END_OF_FILE, "", currentLine, linePosition);
        return tokens;
    }

    // Функция для экранирования символов Markdown
    std::string escapeMarkdown(const std::string& text) {
        std::string escaped = text;
        // Экранирование специальных символов Markdown
        size_t pos = 0;
        while ((pos = escaped.find("|", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\|");
            pos += 2;
        }
        return escaped;
    }

    void printTokens() {
        std::cout << std::left << std::setw(10) << "Строка" 
                  << std::setw(20) << "Имя лексемы"
                  << std::setw(15) << "Класс"
                  << std::setw(30) << "Значение/Атрибут" << std::endl;
        std::cout << std::string(75, '-') << std::endl;

        for (const auto& token : tokens) {
            if (token.type == END_OF_FILE) break;

            std::cout << std::left << std::setw(10) << token.line
                      << std::setw(20) << token.value
                      << std::setw(15) << getTokenTypeName(token.type)
                      << std::setw(30) << getTokenAttribute(token) << std::endl;
        }
    }

    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка открытия файла " << filename << std::endl;
            return;
        }

        // Определение типа файла по расширению
        bool isMarkdown = filename.substr(filename.find_last_of(".") + 1) == "md";

        if (isMarkdown) {
            // Вывод в формате Markdown
            file << "# Результаты лексического анализа\n\n";
            file << "| Строка | Имя лексемы | Класс | Значение/Атрибут |\n";
            file << "|--------|-------------|-------|------------------|\n";

            for (const auto& token : tokens) {
                if (token.type == END_OF_FILE) break;

                file << "| " << token.line 
                     << " | `" << escapeMarkdown(token.value) << "`"
                     << " | " << getTokenTypeName(token.type)
                     << " | " << escapeMarkdown(getTokenAttribute(token)) << " |\n";
            }

            file << "\n## Статистика\n\n";

            std::unordered_map<TokenType, int> stats;
            for (const auto& token : tokens) {
                if (token.type != END_OF_FILE) {
                    stats[token.type]++;
                }
            }

            file << "| Тип токена | Количество |\n";
            file << "|------------|------------|\n";
            for (const auto& stat : stats) {
                file << "| " << getTokenTypeName(stat.first) 
                     << " | " << stat.second << " |\n";
            }
        } else {
            // Обычный текстовый формат
            file << std::left << std::setw(10) << "Строка" 
                 << std::setw(20) << "Имя лексемы"
                 << std::setw(15) << "Класс"
                 << std::setw(30) << "Значение/Атрибут" << std::endl;
            file << std::string(75, '-') << std::endl;

            for (const auto& token : tokens) {
                if (token.type == END_OF_FILE) break;

                file << std::left << std::setw(10) << token.line
                     << std::setw(20) << token.value
                     << std::setw(15) << getTokenTypeName(token.type)
                     << std::setw(30) << getTokenAttribute(token) << std::endl;
            }
        }

        file.close();
        std::cout << "Результаты сохранены в файл: " << filename << std::endl;
    }

private:
    std::string getTokenTypeName(TokenType type) {
        switch (type) {
            case KEYWORD: return "KEYWORD";
            case IDENTIFIER: return "IDENTIFIER";
            case INTEGER: return "INTEGER";
            case FLOAT: return "FLOAT";
            case STRING_LITERAL: return "STRING";
            case OPERATOR: return "OPERATOR";
            case DELIMITER: return "DELIMITER";
            case COMMENT: return "COMMENT";
            case ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::string getTokenAttribute(const Token& token) {
        switch (token.type) {
            case IDENTIFIER:
                return "id_" + std::to_string(token.line) + "_" + std::to_string(token.position);
            case INTEGER:
            case FLOAT:
                return token.value;
            case STRING_LITERAL:
                return "str_literal";
            case KEYWORD:
            case OPERATOR:
            case DELIMITER:
                return "-";
            case COMMENT:
                return "comment";
            case ERROR:
                return "lexical_error";
            default:
                return "-";
        }
    }
};

// Основная функция
int main(int argc, char* argv[]) {
    std::string inputFile, outputFile;

    if (argc == 3) {
        inputFile = argv[1];
        outputFile = argv[2];
    } else {
        std::cout << "Введите имя входного файла: ";
        std::cin >> inputFile;
        std::cout << "Введите имя выходного файла (с расширением .md для Markdown): ";
        std::cin >> outputFile;
    }

    // Чтение исходного кода из файла
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удается открыть файл " << inputFile << std::endl;
        return 1;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    file.close();

    // Создание и запуск лексического анализатора
    LexicalAnalyzer analyzer;
    analyzer.analyze(sourceCode);

    // Вывод результатов
    std::cout << "\n=== РЕЗУЛЬТАТЫ ЛЕКСИЧЕСКОГО АНАЛИЗА ===\n" << std::endl;
    analyzer.printTokens();

    // Сохранение в файл
    analyzer.saveToFile(outputFile);

    return 0;
}