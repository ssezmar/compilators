# Makefile для лексического анализатора

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = lexer
SOURCE = lexical_analyzer.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) output.txt

test: $(TARGET)
	echo "int main() { float x = 3.14; /* комментарий */ return 0; }" > test.cpp
	./$(TARGET) test.cpp output.txt
	cat output.txt

.PHONY: all clean test
