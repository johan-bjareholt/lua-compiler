/* DV1465 / DV1505 / DV1511 Lab-task example code.
   (C) Dr Andrew Moss 2016
   This code is released into the public domain.

   You are free to use this code as a base for your second assignment after
   the lab sessions (it is not required that you do so). Please be aware of
   where the notes indicate that I have made the code shorter and easier to
   read. These are points that are harder to write if you scale this approach
   up to your assignment instead of defining a more appropriate class hierarchy
   for your parse trees.
*/
#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <vector>
#include <set>
using namespace std;

class ThreeAd;
class BBlock;
class Statement;
class Expression;

#include "globals.h"

class ThreeAd
{
public:
    char op;
    string result, lhs, rhs;

    ThreeAd(string out, char o, string l, string r);

    void dump(std::stringstream& ss);
};

class BBlock
{
    public:
    list<ThreeAd> instructions;
    BBlock *trueExit = nullptr;
    BBlock *falseExit = nullptr;
    std::string label;
  
    BBlock();

    void dump(std::stringstream& ss); 
    void dumpDot(std::stringstream& ss, bool start=true);
};


/* One style for expressing a simple parse-tree.

   There are many better ways to do this that use inheritence and encode 
   node-type in a static class hierarchy. Instead I've smashed all the 
   node types together into a single class: the code is shorter and I 
   need everyone to read and understand it quickly for the lab.
*/

///////////////////////// Expressions /////////////////////////////////////

class Expression      // Implicit union of binary operators, constants and variables.
{
    public:
    class Expression *left, *right;
    char kind, op;
    int value;
    string name;

    Expression(char k, Expression *l, Expression *r);

    void dump(std::stringstream& ss, int depth=0);
  
};

// "Helper" functions to build nodes in the tree...


Expression *BinOp(char op, Expression *l, Expression *r);

Expression *UnOp(char op, Expression *l);

Expression *Variable(string name);

Expression *TableItem(string name, Expression* index);

Expression *Constant(int value);

Expression *String(string name);

Expression *Label(std::string value);

Expression *Goto(std::string value);

/* Note: You almost certainly do not want to smash together Comparitor
         and Expression classes in anything more complex than the lab */
Expression *Equality(Expression *l, Expression *r);

///////////////////////// Statements /////////////////////////////////////

class Statement
{
public:
vector<Expression*> expressions;
vector<Statement*> children;
char kind;
    Statement(char k);

    void dump(std::stringstream& ss, int indent=0);
};

// Again, "helper" functions rather than separate classes to keep it short.

Statement *Assign(string target, Expression *val);

Statement *Assign(string target, Statement* statement);

Statement *If(Expression *condition, Statement *trueSt, Statement *falseSt);

Statement *Seq(initializer_list<Statement*> ss);

Statement *For(std::string varname, Expression* varval, Expression* boundry, Expression* step, Statement* body);

Statement *While(Expression* expression, Statement* body);

Statement *Repeat(Expression* expression, Statement* body);

Statement *FunctionDef(std::string& name, std::list<Expression*> args, Statement* body);

Statement *FunctionCall(Expression* funcname, Statement* args);


string newName();

void namePass(Expression *tree, map<Expression*,string> &nameMap);

void emitPass(Expression *tree, map<Expression*,string> &nameMap, BBlock *out);

// Returns the last evaluated name
string convert(Expression *in, BBlock *out);

// TASK: Fill this in to demonstrate understanding of gluing the expression-bit into
//       the statement-bit
void convertAssign(Statement *in, BBlock *out);

// TASK: Basically - reuse the expression translation (two bits of psuedo code get
//       called from convert() ).
void convertComparitor(Expression *in, BBlock *out);

void convertStatement(Statement *in, BBlock **current);


// TODO: Double indirection / current pointer / overwriting callers memory
// Tutorially stuff. Fake 3 line example....
// BIG TASK: invent this from slide 26 (lec8) slide 22 (lec8)
void convertIf(Statement *in, BBlock **current);

void convertSeq(Statement *in, BBlock **current);

// Despatch point
void convertStatement(Statement *in, BBlock **current);


// Iterate through the BBlock nodes in the CFG and dump each one
// exactly once. This is provided as an example of marking nodes
// in a graph and implementing traversals.
void dumpCFG(std::stringstream& ss, BBlock *start);

