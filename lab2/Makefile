CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

SRC = lexer.cpp parser.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = syntax

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

test:
	@echo "=== Positive tests ==="
	@./syntax tests/positive1.cpp && echo "positive1 OK"
	@./syntax tests/positive2.cpp && echo "positive2 OK"
	@echo "\n=== Negative tests ==="
	-@./syntax tests/negative1.cpp && echo "negative1 FAILED" || echo "negative1 error detected"
	-@./syntax tests/negative2.cpp && echo "negative2 FAILED" || echo "negative2 error detected"

.PHONY: all clean
