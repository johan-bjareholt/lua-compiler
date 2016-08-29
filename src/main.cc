#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "grammar.tab.hh"

#include "ir.h"
#include "convert.h"
#include "globals.h"

#include "tests.cc"

bool debug_lex = true;
bool debug_grammar = false;
bool debug_interpretation = false;
bool output_dotfile = false;
bool interpret = true;

void yy::parser::error(const std::string& err){
    std::cout << "It's one of the bad ones... " << err << std::endl;
    exit(-1);
}

enum INPUT_MODES {
	INPUT_STDIN,
	INPUT_FILE,
};

int mode = INPUT_STDIN;
char* filename = NULL;

void parse_flags(int argc, char** argv);

int main(int argc, char** argv){
	parse_flags(argc, argv);
	yy::parser parser;
	parser.parse();
	root->dump();

	BBlock *start = new BBlock();
	BBlock *current = start;
	convertStatement(root, &current);
	dumpCFG(start);
	/*
	outMainBlock(*start);
	*/
	
	return 0;
}

void parse_flags(int argc, char** argv){
	for (int i=1; i<argc; i++){
		set_input_file(argv[i]);
	}
}

