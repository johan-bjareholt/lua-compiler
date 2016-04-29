NAME=lua
SRC=main.cc ir.cc convert.cc
CPPFLAGS= --std=c++11 -g

all:
	$(CXX) $(CPPFLAGS) $(SRC) -o $(NAME)
