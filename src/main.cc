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
	parse_flags(argc, argv);
	
    // Grammar
    yy::parser parser;
	parser.parse();
    if (debug_grammar){
        std::stringstream ss;
        root->dump(ss);
        std::cout << ss.str();
    }

    // IR
	BBlock* start = new BBlock();
	BBlock* current = start;
	convertStatement(root, &current);
    if (debug_cfg){
        std::stringstream ss;
	    dumpCFG(ss, start);
        for (auto block : funcdefs){
            ss << "Function " << block.first << std::endl;
            dumpCFG(ss, block.second);
        }
        std::cout << ss.str();
    }
   
    // CFG dot graph
    std::ofstream outDotFile("graph.dot");
    std::list<BBlock*> outdotblocks;
    outdotblocks.push_back(start);
    for (auto block : funcdefs){
        outdotblocks.push_back(block.second);
    }
    dumpDot(outDotFile, outdotblocks);
    outDotFile.close();
	
    // Target output
    if (output_mode == OUTPUT_FILE){
        std::ofstream outFile(output_filename);
        outMainBlock(outFile, *start);
        outFile.close();
    }
    else if (output_mode == OUTPUT_STDOUT){
        std::stringstream ss;
        outMainBlock(ss, *start);
        std::cout << ss.str();
    }
	
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

