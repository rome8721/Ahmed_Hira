# Makefile — Ahmed Hira
# Usage:
#   make        - compile the program
#   make run    - compile and run
#   make clean  - remove build artifacts

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g
SRC      := $(wildcard src/*.cpp)
TARGET   := program

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(TARGET).dSYM

.PHONY: all run clean
