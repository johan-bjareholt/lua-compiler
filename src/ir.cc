#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>
#include <vector>
#include <set>

#include "ir.h"

std::map<std::string, BBlock*> funcdefs;

using namespace std;

ThreeAd::ThreeAd(string out, char o, string l, string r)
 :  result(out), op(o), lhs(l), rhs(r)      {}

void ThreeAd::dump(std::ostream& ss)
{
    ss  << result << " := " << lhs << " " 
        << op << " " << rhs << endl;
}

BBlock::BBlock() :trueExit(NULL), falseExit(NULL){}

void BBlock::dump(std::ostream& ss)
{
    ss << "BBlock " << " @ " << this << endl;
    for(auto i : instructions)
        i.dump(ss);
    ss << "True: " << trueExit << endl;
    ss << "False: " << falseExit << endl;
}

static std::set<BBlock*> traversed_blocks;

void dumpDot(std::ostream& ss, std::list<BBlock*> startblocks){
    traversed_blocks = std::set<BBlock*>();
    ss << "digraph {" << std::endl;
    ss << "size=\"6,6\";" << std::endl;
    ss << "node [color=lightblue, style=filled];" << std::endl;

    for (BBlock* block : startblocks){
        block->dumpDot(ss);
    }
    
    ss << "}" << std::endl;
}

void BBlock::dumpDot(std::ostream& ss){
    traversed_blocks.insert(this);
    // Def self
    ss  << '"' << this << '"' << std::endl;
    // Set self string
    if (this->label.empty())
        ss  << "[" << "label=\"" << this << "" << std::endl;
    else
        ss  << "[" << "label=\"" << this->label << "" << std::endl;
    std::stringstream ss_code;
    for(auto i : instructions){
        i.dump(ss_code);
    }
    // Handle " in the code so it doesn't escape
    std::string code_s = ss_code.str();
    for (int i = 0; i < code_s.length(); i++) {
        //std::cout << i << std::endl;
        if (code_s[i] == '"'){
            code_s[i] = '\'';
        }
    }
    ss << code_s;
    // Close self def
    ss << "\"];" << std::endl;
    // Connect to subs
    if (trueExit)
        ss  << "    \"" << this << "\" -> \"" << trueExit << "\";" << std::endl;
    if (falseExit)
        ss  << "    \"" << this << "\" -> \"" << falseExit << "\";"<< std::endl;
    // Dumpdot subs
    bool trueExitFound = traversed_blocks.find(trueExit) != traversed_blocks.end();
    if (trueExit && !trueExitFound)
        trueExit->dumpDot(ss);
    bool falseExitFound = traversed_blocks.find(falseExit) != traversed_blocks.end();
    if (falseExit && !falseExitFound)
        falseExit->dumpDot(ss);
}


/* One style for expressing a simple parse-tree.

   There are many better ways to do this that use inheritence and encode 
   node-type in a static class hierarchy. Instead I've smashed all the 
   node types together into a single class: the code is shorter and I 
   need everyone to read and understand it quickly for the lab.
*/

///////////////////////// Expressions /////////////////////////////////////

Expression::Expression(char k, Expression *l, Expression *r) 
 : kind(k), left(l), right(r)                {}

void Expression::dump(std::ostream& ss, int depth)
{
    for(int i=0; i<depth; i++)  
        ss << "  ";
    switch(kind)
    {
        case 'N':  ss << op    << endl; break;
        case 'V':  ss << name  << endl; break;
        case 'C':  ss << value << endl; break;
	    case 'S':  ss << name  << endl; break;
	    case 'f':  ss << name  << endl; break;
        default: std::cout << "ERROR: bad expression kind '" << kind << '\''<< endl; ss << "bad expression" << endl; break;
    }
    if(left!=NULL)
        left->dump(ss, depth+1);
    if(right!=NULL)
        right->dump(ss, depth+1);
    for (Expression* exp : subexp)
        exp->dump(ss, depth+1);
}



// "Helper" functions to build nodes in the tree...

Expression *BinOp(char op, Expression *l, Expression *r)
{
    Expression *result = new Expression('N',l,r);
    result->op = op;
    return result;
}

Expression *UnOp(char op, Expression *l){
	Expression *result = new Expression('N',l,NULL);
	result->op = op;
	return result;
}

Expression *Variable(string name)
{
    Expression *result = new Expression('V',NULL,NULL);
    result->name = name;
    return result;
}

Expression *Constant(int value)
{
    Expression *result = new Expression('C',NULL,NULL);
    result->value = value;
    return result;
}

Expression *String(std::string value){
    Expression *result = new Expression('S',NULL,NULL);
    result->name = value;
    return result;
}

Expression *FunctionCall(std::string name, std::list<Expression*> args){
    Expression *result = new Expression('f',NULL,NULL);
    result->name = name;
    result->op = 'f';
    result->subexp = args;
    return result;
}

/*
  I probably shouldn't have a single Equality statement
  (should have one for branching and one for a bool)
*/

Expression *Equality(Expression *l, Expression *r)
{
    Expression *result = new Expression('N',l,r);
    result->kind = '=';
    return result;
}

///////////////////////// Statements /////////////////////////////////////

Statement::Statement(char k)  : kind(k)  {}

void Statement::dump(std::ostream& ss, int indent)
{
    for(int i=0; i<indent; i++)
        ss << "  ";
    ss << "Statement(" << kind << ")" << endl ;
    for( auto e: expressions ){
        e->dump(ss, indent+1);
    }
    for( auto c: children )
        c->dump(ss, indent+1);
}


// Again, "helper" functions rather than separate classes to keep it short.

Statement *Assign(string target, Expression *val)
{
    Statement *result = new Statement('A');
    result->expressions.push_back( Variable(target) );
    result->expressions.push_back(val);
    return result;
}

Statement *If(Expression *condition, Statement *trueSt, Statement *falseSt)
{
    Statement *result = new Statement('I');
    result->expressions.push_back(condition);
    result->children.push_back(trueSt);
    result->children.push_back(falseSt);
    return result;
}

Statement *Seq(initializer_list<Statement*> ss)
{
    Statement *result = new Statement('S');
    for(auto s : ss)
        result->children.push_back(s);
    return result;
}

Statement *Loop(Statement* pre, Expression* check, Statement* body, Statement* post){
    Statement *result = new Statement('L');
    result->children.push_back(pre);
    result->expressions.push_back(check);
    result->children.push_back(body);
    result->children.push_back(post);
    return result;
}

Statement *For(std::string varname, Expression* varval, Expression* boundry, Expression* step, Statement* body){
    // Get statement pointer as address
    std::ostringstream address;
    address << "_l";
    address << (void const *)body;
    std:string labelname = address.str();

    // Loop start (assign)
    Statement *pre = new Statement('S');
	pre->children.push_back(Assign(varname, varval));
	// Loop body (if,code,increment,if etc.)
    char checktoken = '=';
    if (step->value > 0)
        checktoken = '>';
    else if (step->value < 0)
        checktoken = '<';
    else {
        std::cout << "Step value in for loop cannot be 0" << std::endl;
        exit(0);
    }
    Expression* check = BinOp(checktoken,Variable(varname),boundry);
    // Increment
	Statement *post = Assign(varname, BinOp('+',Variable(varname),step));
    // Return
	return Loop(pre, check, body, post);
}

Statement *While(Expression* expression, Statement* body){
    Statement* pre = new Statement('S');
    Expression* check = expression;
    Statement* post = new Statement('S');
	return Loop(pre, check, body, post);
}

Statement *Repeat(Expression* expression, Statement* body){
    Statement* repeat = new Statement('R');
    repeat->children.push_back(repeat);
    repeat->expressions.push_back(expression);
	return repeat;
}

Statement *FunctionDef(std::string& name, std::vector<Expression*> args, Statement* body){
	Statement* result = new Statement('F');
	Expression* nameExp = String(name);
	result->expressions.push_back(nameExp);
	// TODO: Add proper args support
    Statement* argscontainer = new Statement('E');
    argscontainer->expressions = args;
	result->children.push_back(argscontainer);
	result->children.push_back(body);
	return result;
}

Statement *Return(std::list<Expression*> retvals){
    Statement *result = new Statement('r');
    for (Expression* exp : retvals){
        result->expressions.push_back(exp);
    }
    return result;
}





int nameCounter = 0;
string newName()
{
    stringstream result;
    result << "_t" << nameCounter++;
    return result.str();
}
int stringNameCounter = 0;
string newStringName()
{
    stringstream result;
    result << "_s" << stringNameCounter++;
    return result.str();
}


void namePass(Expression *tree, map<Expression*,string> &nameMap)
{
    std::string name;
    // Exp is funccall
    if (tree->kind == 'f'){
        for (Expression* exp : tree->subexp){
	        namePass(exp, nameMap);
        }
        name = newName();
    }
    // Exp is binop
    else if (tree->left != nullptr && tree->right != nullptr){
	    namePass(tree->left, nameMap);
	    namePass(tree->right, nameMap);
	    name = newName();
    }
    else {
        // Exp is string
        if (tree->name == "")
            name = to_string(tree->value);
        // Exp is var
	    else
            name = tree->name;
    }
  
    auto entry = std::pair<Expression*, string>(tree, name);
    nameMap.insert(entry);
}

void emitPass(Expression *tree, map<Expression*,string> &nameMap, BBlock *out)
{
    if (tree->kind == 'N'){
        emitPass(tree->left, nameMap, out);
        emitPass(tree->right, nameMap, out);
	    ThreeAd* ta = new ThreeAd(
            nameMap[tree],
	        tree->op, nameMap[tree->left],
	        nameMap[tree->right]
        );
        out->instructions.push_back(*ta);
    }
    else if (tree->kind = 'f' && tree->op == 'f'){
        for (Expression* exp : tree->subexp){
            emitPass(exp, nameMap, out);
        }
	    ThreeAd* ta = new ThreeAd(
            nameMap[tree],
	        tree->op, 
            tree->name, nameMap[tree->subexp.front()]
        );
        out->instructions.push_back(*ta);
    }
}

// Returns the last evaluated name
string convert(Expression *in, BBlock *out)
{
    map<Expression*,string> naming;
    namePass(in, naming);
    emitPass(in, naming, out);
    return naming[in];
}

void convertStatement(Statement *in, BBlock **current);

void convertAssign(Statement *in, BBlock *out)
{
    std::string name = in->expressions.at(0)->name;
    std::string val = convert(in->expressions.at(1), out);
    ThreeAd ta = ThreeAd(name,'c',name,val);
    out->instructions.push_back(ta);
}

void convertComparitor(Expression *in, BBlock *out)
{
    std::string l = convert(in->left, out);
    std::string r = convert(in->right, out);
    std::string name = newName();
    ThreeAd ta = ThreeAd(name,in->op,l,r);
    out->instructions.push_back(ta);
}


void convertIf(Statement *in, BBlock **current)
{
    // Translate comparitor
    Expression* comparitor = in->expressions.back();
    convertComparitor(comparitor, *current);

    // Create true and false block
    BBlock* trueBlock = new BBlock();
    BBlock* trueBlockPtr = trueBlock;
    convertStatement(in->children.at(0), &trueBlockPtr);
    (*current)->trueExit = trueBlock;

    BBlock* falseBlock = new BBlock();
    BBlock* falseBlockPtr = falseBlock;
    convertStatement(in->children.at(1), &falseBlockPtr);
    (*current)->falseExit = falseBlock;

    // Create next block
    BBlock* nextBlock = new BBlock();
    trueBlock->trueExit = nextBlock;
    falseBlock->trueExit = nextBlock;

    *current = nextBlock;
}

void convertFuncDef(Statement* in, BBlock *current){
	std::string name = in->expressions.at(0)->name;
    //std::cout << "Defining function " << name << std::endl;
	Statement* args = in->children.at(0);
	Statement* body = in->children.at(1);
	BBlock* bodyBlock = new BBlock();
    BBlock* currblock = bodyBlock;
    // TODO: Fix support for multiple arguments
    if (args->expressions.size() > 0){
        Expression* exp1 = args->expressions.at(0);
        ThreeAd ta (name, 'a', exp1->name, exp1->name);
        bodyBlock->instructions.push_back(ta);
    }
	convertStatement(body, &currblock);

    funcdefs.insert(std::make_pair(name, bodyBlock));
}

void convertSeq(Statement *in, BBlock **current)
{
    for(auto s: in->children)
        convertStatement(s,current);
}

void convertRepeat(Statement *in, BBlock **current){
    Statement*  body = in->children.at(0);
    Expression* check = in->expressions.at(0);

    BBlock* bodyBlock = new BBlock();
    BBlock* checkBlock = new BBlock();
    BBlock* nextBlock = new BBlock();

    (*current)->trueExit = bodyBlock;

    (*current) = bodyBlock;
    convertStatement(body, current);
    bodyBlock->trueExit = checkBlock;
    
    (*current) = checkBlock;
    convertComparitor(check, *current);
    checkBlock->trueExit = bodyBlock;
    checkBlock->falseExit = nextBlock;

    (*current) = nextBlock;
}

void convertLoop(Statement *in, BBlock **current){
    Statement* pre = in->children.at(0);
    convertStatement(pre, current);
    Expression* check = in->expressions.at(0);
    Statement* body = in->children.at(1);
    Statement* post = in->children.at(2);

    BBlock* loopBlock = new BBlock();
    BBlock* bodyBlock = new BBlock();
    BBlock* nextBlock = new BBlock();

    (*current)->trueExit = loopBlock;
    // Set loop block
    (*current) = loopBlock;
    Expression* comparitor = in->expressions.back();
    convertComparitor(comparitor, *current);
    (*current)->falseExit = bodyBlock;
    (*current)->trueExit = nextBlock;

    // Set body block
    (*current) = bodyBlock;
    convertStatement(body, current);
    convertStatement(post, current);
    (*current)->trueExit = loopBlock;

    // Set next block
    (*current) = nextBlock;
}

void convertExpressions(Statement *in, BBlock *current){
    for(Expression* e: in->expressions)
        convert(e,current);
}

void convertReturn(Statement *in, BBlock *current){
    // TODO: Support for multiple return values
    Expression* retexp = in->expressions.at(0);
    std::string retval = convert(retexp, current);
    ThreeAd ta = ThreeAd("_r",'r',retval, retval);
    current->instructions.push_back(ta);
    
    /*for(Expression* exp: in->expressions){
        convert(exp, current);
    }*/
}

// Despatch point
void convertStatement(Statement *in, BBlock **current)
{
    switch(in->kind)
    {
        case 'A':
            convertAssign(in,*current);   // Does not update current
            break;
        case 'F':
            convertFuncDef(in,*current);	// Does not update current
            break;
        case 'E':
            convertExpressions(in, *current); // Does not update current
            break;
        case 'r':
            convertReturn(in,*current); // Does not update current
            break;
        case 'L':
            convertLoop(in,current);
            break;
        case 'R':
            convertRepeat(in,current);
            break;
        case 'I':
            convertIf(in,current);
            break;
        case 'S':
            convertSeq(in,current);
            break;
    }
}


// Iterate through the BBlock nodes in the CFG and dump each one
// exactly once. This is provided as an example of marking nodes
// in a graph and implementing traversals.
void dumpCFG(std::ostream& ss, BBlock *start)
{
    set<BBlock*> done, todo;
    todo.insert(start);
    while(todo.size()>0)
    {
        // Pop an arbitrary element from todo set
        auto first = todo.begin();
        BBlock *next = *first;
        todo.erase(first);

        next->dump(ss);

        done.insert(next);
        if(next->trueExit!=NULL && done.find(next->trueExit)==done.end())
            todo.insert(next->trueExit);
        if(next->falseExit!=NULL && done.find(next->falseExit)==done.end())
            todo.insert(next->falseExit);
    }
}

