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

bool debug_lex = false;
bool debug_grammar = false;
bool debug_cfg = false;
bool output_dotfile = false;

void yy::parser::error(const std::string& err){
    std::cout << "It's one of the bad ones... " << err << std::endl;
    exit(-1);
}

enum INPUT_MODES {
	INPUT_STDIN,
	INPUT_FILE,
};

enum OUTPUT_MODES {
    OUTPUT_STDOUT,
    OUTPUT_FILE,
};

int input_mode = INPUT_STDIN;
std::string input_filename = "";

int output_mode = OUTPUT_STDOUT;
std::string output_filename = "";

void parse_flags(int argc, char** argv);

int main(int argc, char** argv){
    std::stringstream ss;
	parse_flags(argc, argv);
	
    yy::parser parser;
	parser.parse();
	
    root->dump(ss);
    if (debug_grammar){
        std::cout << ss.str();
    }
    ss.str("");

	BBlock *start = new BBlock();
	BBlock *current = start;
	convertStatement(root, &current);
    if (debug_cfg){
	    dumpCFG(ss, start);
        std::cout << ss.str();
        ss.str("");
    }
	
    outMainBlock(ss, *start);
    if (output_mode == OUTPUT_FILE){
        std::ofstream outFile(output_filename);
        outFile << ss.str();
        outFile.close();
    }
    //else if (output_mode == OUTPUT_STDOUT){
    //    std::cout << ss.str();
    //}
    ss.str("");
	
	return 0;
}

void parse_flags(int argc, char** argv){
	for (int i=1; i<argc; i++){
        if (argv[i][0] == '-'){
            switch (argv[i][1]){
                case 'o':
                    i++;
                    output_mode = OUTPUT_FILE;
                    output_filename = argv[i];
                    break;
                case 'd':
                    debug_lex = true;
                    debug_grammar = true;
                    debug_cfg = true;
                    break;
                default:
                    std::cout << "Unknown option -"<< argv[i][1] << std::endl;
                    exit(1);
                    break;
            } 
        }
        else
		    set_input_file(argv[i]);
	}
}

