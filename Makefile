NAME=lua
SRC=main.cc ir.cc
CPPFLAGS= --std=c++11 -g

all:
	$(CXX) $(CPPFLAGS) $(SRC) -o $(NAME)
