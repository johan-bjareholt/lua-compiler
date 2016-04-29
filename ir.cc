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
#include<string>
#include<iostream>
#include<sstream>
#include<list>
#include<map>
#include<vector>
#include<set>

#include "ir.h"

using namespace std;

ThreeAd::ThreeAd(string out, char o, string l, string r)
 :  result(out), op(o), lhs(l), rhs(r)      {}

void ThreeAd::dump()
{
  cout << result << " := " << lhs << " " 
       << op << " " << rhs << endl;
}

BBlock::BBlock()
 :  trueExit(NULL), falseExit(NULL)  {}

void BBlock::dump()
{
  cout << "BBlock @ " << this << endl;
  for(auto i : instructions)
    i.dump();
  cout << "True: " << trueExit << endl;
  cout << "False: " << falseExit << endl;
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

void Expression::dump(int depth)
{
  for(int i=0; i<depth; i++)  
    cout << "  ";
  switch(kind)
  {
    case 'N':  cout << op << endl;    break;
    case 'V':  cout << name << endl;  break;
    case 'C':  cout << value << endl; break;
  }
  if(left!=NULL)
    left->dump(depth+1);
  if(right!=NULL)
    right->dump(depth+1);
}



// "Helper" functions to build nodes in the tree...

Expression *BinOp(char op, Expression *l, Expression *r)
{
Expression *result = new Expression('N',l,r);
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

/* Note: You almost certainly do not want to smash together Comparitor
         and Expression classes in anything more complex than the lab */
Expression *Equality(Expression *l, Expression *r)
{
Expression *result = new Expression('N',l,r);
  result->kind = '=';
  return result;
}

///////////////////////// Statements /////////////////////////////////////

Statement::Statement(char k)  : kind(k)  {}

void Statement::dump(int indent)
{
  for(int i=0; i<indent; i++)
    cout << "  ";
  cout << "Statement(" << kind << ")" << endl ;
  for( auto e: expressions )
    e->dump(indent+1);
  for( auto c: children )
    c->dump(indent+1);
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




int nameCounter = 0;
string newName()
{
stringstream result;
  result << "_t" << nameCounter++;
  return result.str();
}

void namePass(Expression *tree, map<Expression*,string> &nameMap)
{
  std::string name;
  if (tree->left != nullptr && tree->right != nullptr){
	  namePass(tree->left, nameMap);
	  namePass(tree->right, nameMap);
	  name = newName();
  }
  else {
    if (tree->name == "")
      name = to_string(tree->value);
	else
      name = tree->name;
  }
  
  auto entry = std::pair<Expression*, string>(tree, name);
  nameMap.insert(entry);
}

void emitPass(Expression *tree, map<Expression*,string> &nameMap, BBlock *out)
{
  // ...
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
}

// Returns the last evaluated name
string convert(Expression *in, BBlock *out)
{
  map<Expression*,string> naming;
  // TODO: Implement the pseudo code from lec-8 / slides 11 and 12 
  //       in the empty procedures above and write the output into
  //       the target BBlock object.
  namePass(in, naming);
  emitPass(in, naming, out);
  return naming[in];
}

// TASK: Fill this in to demonstrate understanding of gluing the expression-bit into
//       the statement-bit
void convertAssign(Statement *in, BBlock *out)
{
  // TODO: Convert the assignment statement passed in to 3-address instructions
  //       to fill the target BBlock with.
  std::string name = in->expressions.at(0)->name;
  std::string val = convert(in->expressions.at(1), out);
  ThreeAd ta = ThreeAd(name,'c',val,val);
  out->instructions.push_back(ta);
}

// TASK: Basically - reuse the expression translation (two bits of psuedo code get
//       called from convert() ).
void convertComparitor(Expression *in, BBlock *out)
{
  // TODO: Convert the expression tree with a root comparitor operation into
  //       3-address instructions and fill the target BBlock.
  std::string l = convert(in->left, out);
  std::string r = convert(in->right, out);
  //std::cout << "left:" << l << std::endl;
  //std::cout << "right:" << r << std::endl;
  //std::string name = convert(in, out);
  std::string name = newName();
  ThreeAd ta = ThreeAd(name,'=',l,r);
  out->instructions.push_back(ta);
}

void convertStatement(Statement *in, BBlock **current);


// TODO: Double indirection / current pointer / overwriting callers memory
// Tutorially stuff. Fake 3 line example....
// BIG TASK: invent this from slide 26 (lec8) slide 22 (lec8)
void convertIf(Statement *in, BBlock **current)
{
  // TODO: Build the graph structure to implement the if statement
  //       as shown in lecture 8 / slide 22 (and 25). Update the
  //       caller's current pointer before returning.

  // Translate comparitor
  Expression* comparitor = in->expressions.back();
  convertComparitor(comparitor, *current);
  
  // Create true and false block
  BBlock* trueBlock = new BBlock();
  convertStatement(in->children.at(0), &trueBlock);
  (*current)->trueExit = trueBlock;
  
  BBlock* falseBlock = new BBlock();
  convertStatement(in->children.at(1), &falseBlock);
  (*current)->falseExit = falseBlock;
  
  BBlock* nextBlock = new BBlock();
  trueBlock->trueExit = nextBlock;
  falseBlock->trueExit = nextBlock;

  *current = nextBlock;
}

void convertSeq(Statement *in, BBlock **current)
{
  for(auto s: in->children)
  {
    convertStatement(s,current);
  }
}

// Despatch point
void convertStatement(Statement *in, BBlock **current)
{
  switch(in->kind)
  {
    case 'A':
      convertAssign(in,*current);   // Does not update current
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
void dumpCFG(BBlock *start)
{
  set<BBlock *> done, todo;
  todo.insert(start);
  while(todo.size()>0)
  {
    // Pop an arbitrary element from todo set
    auto first = todo.begin();
    BBlock *next = *first;
    todo.erase(first);

    next->dump();
    done.insert(next);
    if(next->trueExit!=NULL && done.find(next->trueExit)==done.end())
      todo.insert(next->trueExit);
    if(next->falseExit!=NULL && done.find(next->falseExit)==done.end())
      todo.insert(next->falseExit);
  }
}

