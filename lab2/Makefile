# Makefile для улучшенного синтаксического анализатора

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g
TARGET = syntax_analyzer
SOURCES = lexer.cpp parser.cpp main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Цвета для вывода
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[1;33m
NC = \033[0m # No Color

.PHONY: all clean test test-positive test-negative help install

all: $(TARGET)
	@echo "$(GREEN)✓ Build completed successfully$(NC)"

$(TARGET): $(OBJECTS)
	@echo "$(YELLOW)Linking $(TARGET)...$(NC)"
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	@echo "$(YELLOW)Compiling $<...$(NC)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(YELLOW)Cleaning up...$(NC)"
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.log *.ast
	rm -f tests/*.log tests/*.ast
	@echo "$(GREEN)✓ Clean completed$(NC)"

# Создание тестовых файлов
setup-tests:
	@echo "$(YELLOW)Setting up test files...$(NC)"
	@mkdir -p tests
	@echo 'int main() { int x = 42; return x; }' > tests/positive1.cpp
	@echo 'int factorial(int n) { if (n <= 1) return 1; else return n * factorial(n - 1); }' > tests/positive2.cpp
	@echo 'void test() { int a = 5; int b = 10; int sum = a + b; }' > tests/positive3.cpp
	@echo 'int main() { int x = 10 return 0; }' > tests/negative1.cpp
	@echo 'int main() { if (1 { x = 5; } return 0; }' > tests/negative2.cpp
	@echo 'int main() { int x = ; return 0; }' > tests/negative3.cpp
	@echo "$(GREEN)✓ Test files created$(NC)"

# Позитивные тесты
test-positive: $(TARGET) setup-tests
	@echo "$(GREEN)Running positive tests...$(NC)"
	@for test in tests/positive*.cpp; do \
		echo "$(YELLOW)Testing $$test...$(NC)"; \
		if ./$(TARGET) $$test --log $$test.log --ast $$test.ast --no-output; then \
			echo "$(GREEN)✓ $$test PASSED$(NC)"; \
		else \
			echo "$(RED)✗ $$test FAILED$(NC)"; \
		fi; \
		echo; \
	done

# Негативные тесты
test-negative: $(TARGET) setup-tests
	@echo "$(RED)Running negative tests (expecting failures)...$(NC)"
	@for test in tests/negative*.cpp; do \
		echo "$(YELLOW)Testing $$test...$(NC)"; \
		if ./$(TARGET) $$test --log $$test.log --no-output 2>/dev/null; then \
			echo "$(RED)✗ $$test FAILED (should have detected error)$(NC)"; \
		else \
			echo "$(GREEN)✓ $$test PASSED (error correctly detected)$(NC)"; \
		fi; \
		echo; \
	done

# Все тесты
test: test-positive test-negative
	@echo "$(GREEN)All tests completed$(NC)"

# Демонстрация
demo: $(TARGET) setup-tests
	@echo "$(GREEN)Running demonstration...$(NC)"
	@echo "$(YELLOW)Analyzing positive1.cpp with full output:$(NC)"
	./$(TARGET) tests/positive1.cpp --ast demo.ast --log demo.log
	@echo "\n$(YELLOW)Generated files:$(NC)"
	@ls -la *.ast *.log 2>/dev/null || true

# Справка
help:
	@echo "$(GREEN)Available targets:$(NC)"
	@echo "  all           - Build the syntax analyzer"
	@echo "  clean         - Remove all generated files"
	@echo "  test          - Run all tests"
	@echo "  test-positive - Run positive tests only"
	@echo "  test-negative - Run negative tests only"
	@echo "  setup-tests   - Create test files"
	@echo "  demo          - Run demonstration"
	@echo "  help          - Show this help"
	@echo ""
	@echo "$(GREEN)Usage:$(NC)"
	@echo "  ./$(TARGET) <source.cpp> [options]"
	@echo "  Options:"
	@echo "    --ast <file>   Save AST to file"
	@echo "    --log <file>   Save parsing log to file"
	@echo "    --no-output    Don't print AST to console"

# Установка зависимостей (для систем с package manager)
install-deps:
	@echo "$(YELLOW)Installing dependencies...$(NC)"
	@which g++ > /dev/null || (echo "$(RED)Error: g++ not found. Please install gcc/g++$(NC)" && exit 1)
	@echo "$(GREEN)✓ All dependencies satisfied$(NC)"