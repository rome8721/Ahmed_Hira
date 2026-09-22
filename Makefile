# Makefile — Ahmed Hira
# Usage:
#   make        - compile the program
#   make run    - compile and run
#   make clean  - remove build artifacts

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g
SRC      := $(wildcard src/*.cpp)
HDR      := $(wildcard src/*.h)

ifeq ($(OS),Windows_NT)
# Windows (MinGW g++): the compiler adds .exe to the program name.
TARGET   := program.exe
else
TARGET   := program
# macOS only: point the compiler at the Command Line Tools SDK.
UNAME_S  ?= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
CXXFLAGS += -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
endif
endif

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f program program.exe
	rm -rf program.dSYM

.PHONY: all run clean
