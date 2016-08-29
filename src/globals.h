#pragma once
#include "ir.h"

extern void set_input_file(char*);

extern int output_mode;
extern std::string outfile;

extern Statement* root;

extern unsigned int total;
extern int linenr;

extern bool debug_lex;
extern bool debug_grammar;
