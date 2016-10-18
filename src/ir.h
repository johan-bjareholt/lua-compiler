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

    void dump(std::ostream& ss);
};

void dumpDot(std::ostream& ss, std::list<BBlock*>);

class BBlock
{
    public:
    list<ThreeAd> instructions;
    BBlock *trueExit = nullptr;
    BBlock *falseExit = nullptr;
    std::string label;
  
    BBlock();

    void dump(std::ostream& ss); 
    void dumpDot(std::ostream& ss);
};


///////////////////////// Expressions /////////////////////////////////////

class Expression
{
    // Implicit union of binary operators, constants and variables.
    public:
    class Expression *left, *right;
    std::list<Expression*> subexp;
    char kind, op;
    int value;
    string name;

    Expression(char k, Expression *l, Expression *r);

    void dump(std::ostream& ss, int depth=0);
  
};

// "Helper" functions to build nodes in the tree...


Expression *BinOp(char op, Expression *l, Expression *r);

Expression *UnOp(char op, Expression *l);

Expression *Variable(string name);

Expression *Constant(int value);

Expression *String(string name);

Expression *FunctionCall(std::string name, std::list<Expression*> args);

Expression *Equality(Expression *l, Expression *r);




// Statements

class Statement
{
public:
vector<Expression*> expressions;
vector<Statement*> children;
char kind;
    Statement(char k);

    void dump(std::ostream& ss, int indent=0);
};

// "helper" functions rather than separate classes to keep it short.

Statement *Assign(string target, Expression *val);

Statement *Assign(string target, Statement* statement);

Statement *If(Expression *condition, Statement *trueSt, Statement *falseSt);

Statement *Seq(initializer_list<Statement*> ss);

Statement *For(std::string varname, Expression* varval, Expression* boundry, Expression* step, Statement* body);

Statement *While(Expression* expression, Statement* body);

Statement *Repeat(Expression* expression, Statement* body);

Statement *FunctionDef(std::string& name, std::vector<Expression*> args, Statement* body);

Statement *Return(std::list<Expression*>);




// Control flow and translation from statements/expressions to 3ac

string newName();

void namePass(Expression *tree, map<Expression*,string> &nameMap);

void emitPass(Expression *tree, map<Expression*,string> &nameMap, BBlock *out);

// Returns the last evaluated name
string convert(Expression *in, BBlock *out);

void convertAssign(Statement *in, BBlock *out);

void convertComparitor(Expression *in, BBlock *out);

void convertStatement(Statement *in, BBlock **current);

void convertIf(Statement *in, BBlock **current);

void convertSeq(Statement *in, BBlock **current);

// Despatch point
void convertStatement(Statement *in, BBlock **current);



void dumpCFG(std::ostream& ss, BBlock *start);

