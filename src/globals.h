#pragma once
#include "ir.h"
#include <vector>

extern void set_input_file(char*);

extern int output_mode;
extern std::string outfile;

extern Statement* root;
extern std::map<std::string, BBlock*> funcdefs;

extern unsigned int total;
extern int linenr;

extern bool debug_lex;
extern bool debug_grammar;
