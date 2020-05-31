CXX=g++
CXXFLAGS=-std=c++17 -Wall -I. -fPIC -O3 -g
LDFLAGS=

# All headers needed for AST usage
AST_HEADERS=frontend/ast.h frontend/token.h frontend/ast_visitor.h frontend/print_visitor.h

.PHONY: test clean all

all: build/c1 #build/lexer_test build/token_test build/parser_test

build/token.o: frontend/token.cpp frontend/token.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/token.cpp -o $@

build/lexer.o: frontend/token.h frontend/lexer.h frontend/lexer.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/lexer.cpp -o $@

build/ast.o: $(AST_HEADERS) frontend/ast.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/ast.cpp -o $@

build/parser.o: frontend/parser.cpp frontend/parser.h $(AST_HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/parser.cpp -o $@

build/ir.o: frontend/parser.cpp frontend/parser.h midend/ir.h $(AST_HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c midend/ir.cpp -o $@

build/lexer_test.o: frontend/token.h frontend/lexer.h frontend/lexer_test.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/lexer_test.cpp -o $@

build/token_test.o: frontend/token.h frontend/token_test.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/token_test.cpp -o $@

build/parser_test.o: frontend/parser_test.cpp frontend/parser.h $(AST_HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c frontend/parser_test.cpp -o $@

build/main.o: frontend/token.h frontend/lexer.h $(AST_HEADERS) frontend/parser.h midend/ir.h main.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c main.cpp -o $@

build/c1: build/main.o build/lexer.o build/token.o build/parser.o build/ast.o build/ir.o
	$(CXX) $(LDFLAGS) $^ -o $@

build/lexer_test: build/lexer.o build/token.o build/lexer_test.o
	$(CXX) $(LDFLAGS) $^ -o $@

build/token_test: build/token.o build/token_test.o
	$(CXX) $(LDFLAGS) $^ -o $@

build/parser_test: build/parser.o build/token.o build/lexer.o build/parser_test.o build/ast.o
	$(CXX) $(LDFLAGS) $^ -o $@

test: build/token_test build/lexer_test build/parser_test
	-./build/token_test
	-./build/lexer_test
	-./build/parser_test

clean:
	rm -f build/*
