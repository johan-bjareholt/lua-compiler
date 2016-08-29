NAME=lua
SRC=main.cc ir.cc convert.cc
CPPFLAGS= --std=c++11 -g

all: lex.yy.c grammar.tab.o $(SRC)
	$(CXX) $(CPPFLAGS) $(SRC) grammar.tab.o lex.yy.c -o $(NAME)


grammar.tab.o: grammar.tab.cc
	g++ -c grammar.tab.cc
grammar.tab.cc: grammar.yy
	bison grammar.yy

lex.yy.c: lex.ll grammar.tab.cc
	flex lex.ll

clean:
	rm $(NAME) grammar.tab.* lex.yy.c* stack.hh
