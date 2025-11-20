CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
DEBUGFLAGS = -std=c++17 -Wall -Wextra -g -O0
SRCDIR = src
BINDIR = bin
TESTDIR = test
OUTDIR = output

TARGET = $(BINDIR)/compiler
SOURCES = $(SRCDIR)/main.cpp
HEADERS = $(SRCDIR)/*.h

# Default target
.PHONY: all
all: dirs $(TARGET)

# Create directories
.PHONY: dirs
dirs:
	@mkdir -p $(BINDIR) $(OUTDIR)

# Build main executable
$(TARGET): $(SOURCES) $(HEADERS) | dirs
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Debug build
.PHONY: debug
debug: dirs
	$(CXX) $(DEBUGFLAGS) $(SOURCES) -o $(TARGET)_debug
	@echo "Debug build complete: $(TARGET)_debug"

# Run tests
.PHONY: test
test: $(TARGET)
	@echo "=== Running Tests ==="
	@echo "\n--- Positive Test 1: Simple Calculations ---"
	@$(TARGET) $(TESTDIR)/example1.txt 2>&1 | head -50
	@echo "\n--- Positive Test 2: Loops and Conditionals ---"
	@$(TARGET) $(TESTDIR)/example2.txt 2>&1 | head -50
	@echo "\n--- Positive Test 3: Functions ---"
	@$(TARGET) $(TESTDIR)/example3.txt 2>&1 | head -50
	@echo "\n--- Negative Test 1: Type Error ---"
	@$(TARGET) $(TESTDIR)/error1.txt 2>&1 | head -30
	@echo "\n--- Negative Test 2: Undefined Variable ---"
	@$(TARGET) $(TESTDIR)/error2.txt 2>&1 | head -30

# Run with valgrind (memory check)
.PHONY: valgrind
valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all \
		$(TARGET)_debug $(TESTDIR)/example1.txt

# Clean
.PHONY: clean
clean:
	@rm -f $(BINDIR)/* $(OUTDIR)/*
	@rmdir $(BINDIR) $(OUTDIR) 2>/dev/null || true
	@echo "Clean complete"

# Help
.PHONY: help
help:
	@echo "Code Generator - Makefile Commands"
	@echo "===================================="
	@echo "make              - Build project"
	@echo "make debug        - Build with debug symbols"
	@echo "make test         - Run all tests"
	@echo "make valgrind     - Run memory check"
	@echo "make clean        - Remove build artifacts"
	@echo "make help         - Show this help"
	@echo "\nUsage: ./bin/compiler <input_file>"
