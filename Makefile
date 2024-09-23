CC = g++
FLAGS = -O3 -std=c++23
FLEX = flex --header-file=lexer.hpp
BISON = bison -d #-v

TARGET = c-

$(TARGET): lexer.cpp parser.cpp token.cpp
	$(CC) $(FLAGS) $^ -o $@

lexer.hpp lexer.cpp: lexer.l
	$(FLEX) -o lexer.cpp lexer.l 

parser.hpp parser.cpp: parser.y
	$(BISON) -o parser.cpp parser.y

clean:
	rm -f $(TARGET) lexer.cpp lexer.hpp parser.cpp parser.hpp parser.output results.txt