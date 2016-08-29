#pragma once
#include "ir.h"

extern void set_input_file(char*);

extern Statement* root;

extern unsigned int total;
extern int linenr;

extern bool debug_lex;
extern bool debug_grammar;
extern bool debug_interpretation;
