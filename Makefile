NAME=lua
SRC=src/main.cc src/ir.cc src/convert.cc
CPPFLAGS= --std=c++11 -g -Isrc/ -Iobj/

.PHONY: prepare
all: obj/lex.yy.c obj/grammar.tab.o $(SRC)
	$(CXX) $(CPPFLAGS) $(SRC) obj/grammar.tab.o obj/lex.yy.c -o $(NAME)

.PHONY: prepare
prepare:
	mkdir -p ./obj

obj/grammar.tab.o: obj/grammar.tab.cc prepare
	g++ $(CPPFLAGS) -c obj/grammar.tab.cc -o $@
obj/grammar.tab.cc: src/grammar.yy prepare
	bison src/grammar.yy -o $@

obj/lex.yy.c: src/lex.ll obj/grammar.tab.cc
	flex -o $@ src/lex.ll

.PHONY: clean
clean:
	rm -rf obj
	rm -f lua
