CXX      = g++
CXXFLAGS = -std=c++17 -Wall -g -Isrc

SRC = src/tac.cpp src/cfg.cpp src/liveness.cpp \
	src/dce.cpp src/unreachable.cpp \
	src/constant_propagation.cpp src/main.cpp

all: parser.tab.c parser.tab.h lex.yy.c
	$(CXX) $(CXXFLAGS) parser.tab.c lex.yy.c $(SRC) -o dce_compiler

parser.tab.c parser.tab.h: src/parser.y
	bison -d src/parser.y -o parser.tab.c

lex.yy.c: src/lexer.l parser.tab.h
	flex  src/lexer.l

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c dce_compiler
