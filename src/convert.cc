#include "convert.h"
#include "ops.h"
#include <cctype>
#include <algorithm>

static const std::string stdlib = \
    "\"print:\"\n" \
    "\"io.write:\"\n" \
    "\"    call puti\"\n" \
    "\"    ret\"\n";

void outMainBlock(std::stringstream& ss, BBlock& startblock){
	ss << "#include <iostream>" << std::endl;
	ss << "int main(){" << std::endl;

	std::list<std::string> usedSymbols = std::list<std::string>();

	std::stringstream body;
    body << stdlib;
	outBlock(startblock, body, usedSymbols);
	body << "\"exit:\\n\\t\"" << std::endl;

	usedSymbols.sort();
	usedSymbols.unique();
	for (std::string symbolname : usedSymbols){
		ss << "long " << symbolname << ";" << std::endl;
	}

	ss << body.str();
	ss << ":";
	for (auto iter = usedSymbols.begin(); iter!=usedSymbols.end(); iter++){
		ss << "  [" << *iter << "] \"+g\" (" << *iter << ")";
		if (std::next(iter,1) != usedSymbols.end())
			ss << ",";
		ss << std::endl;
	}
	ss << ":" << std::endl;
	ss << ");" << std::endl;
	ss << "}" << std::endl;
}

static int labelcount = 0;
std::string newLabel(){
	std::stringstream ss;
	ss << "label" << to_string(labelcount);
	labelcount++;
	return ss.str();
}

std::string outBlock(BBlock& block, std::stringstream& ss, std::list<std::string>& usedSymbols){
	std::string blockname = newLabel();
	ss << "\""<< blockname << ":\\n\\t\"" << std::endl;
	for (ThreeAd& op : block.instructions){
		convertThreeAd(op, ss, usedSymbols);
	}

	
	ss << std::endl;

	std::stringstream subblock;
	std::string bname;
	if (block.falseExit != nullptr){
		bname = outBlock(*block.falseExit, subblock, usedSymbols);
		ss << "\"jne " << bname << "\\n\\t\"" << std::endl;
		ss << subblock.str();
		subblock.str("");
	}
	if (block.trueExit != nullptr){
		bname = outBlock(*block.trueExit, subblock, usedSymbols);
		ss << "\"jmp " << bname << "\\n\\t\"" << std::endl;
		ss << subblock.str();
	}
	else
		ss << "\"jmp exit\\n\\t\"" << std::endl;

	return blockname;
}

bool is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

static const std::string pre_in = "\"";
static const std::string post_in = "\\n\\t\"\n";

void convertThreeAd(ThreeAd& op, std::stringstream& ss, std::list<std::string>& usedSymbols){
	ss << "asm(" << std::endl;
	usedSymbols.push_back(op.result);
	
	std::string lhs = op.lhs;
	std::string rhs = op.rhs;

	std::stringstream tempss1;
	if (is_digits(rhs)){
		tempss1 << "$" << rhs;
		rhs = tempss1.str();
	}
	else {
		usedSymbols.push_back(op.rhs);
		tempss1 << "\%[" << rhs << "]";
		rhs = tempss1.str();
	}

	std::stringstream tempss2;
	if (is_digits(lhs)){
		tempss2 << "$" << lhs;
		lhs = tempss2.str();
	}
	else {
		usedSymbols.push_back(op.lhs);
		tempss2 << "\%[" << lhs << "]";
		lhs = tempss2.str();
	}
	switch(op.op){
		case '+':
			{
			//ss << "\"movq " << rhs << ", %%rax\\n\\t\"" << std::endl;
            //
			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "addq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax , %[" << op.result << "]" << post_in;
			}
			break;
		case '-':
			{
			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "subq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax , [" << op.result << "]" << post_in;
			}
			break;
		case '*':
			break;
        case '/':
            break;
		case '=':
			{
			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "cmp "  << lhs << ", %%rax" << post_in;
			}
			break;
		// Copy
		case 'c':
			ss << pre_in << "movq " << rhs << "," << lhs << post_in;
			break;
		// Label
		case 'l':
			break;
		// Goto
		case 'g':
			break;
		// Call function
		case 'f':
            ss << "\"call " << rhs << "\\n\\n\"" << std::endl;
			break;
	}
}


