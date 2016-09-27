#include "convert.h"
#include "ops.h"
#include <cctype>
#include <algorithm>

static const std::string pre_in = "    \"    ";
static const std::string post_in = "\\n\"\n";


std::list<std::pair<int, std::string>> strings;
static int stringc = 0;
static std::string newString(std::string str){
    std::stringstream tempss;
    tempss << "_s" << stringc;
    strings.push_back(std::pair<int, std::string>(stringc, str));
    stringc++;
    return tempss.str();
}

std::list<std::string> ints;
static int intc = 0;
static std::string newInt(std::string str){
    std::stringstream tempss;
    tempss << "_i" << intc;
    ints.push_back(tempss.str());
    intc++;
    return tempss.str();
}

std::set<std::string> vars;
static int varc = 0;
static void regVar(std::string str){
    vars.insert(str);
}

static int labelcount = 0;
std::string newLabel(){
	std::stringstream ss;
	ss << "label" << to_string(labelcount);
	labelcount++;
	return ss.str();
}



void outMainBlock(std::stringstream& ss, BBlock& startblock){
    std::stringstream headss;
    std::stringstream mainss;
    std::stringstream bodyss;
    
    // Standard functions
    headss << "// Standard functions" << std::endl;
	headss << "#include <iostream>" << std::endl;
    headss << "void print(const char* msg){puts(msg);}" << std::endl;
	
    mainss << "int main(){" << std::endl;

	std::set<std::string> inputSymbols = std::set<std::string>();
    inputSymbols.insert("print");
	
    std::set<std::string> outputSymbols = std::set<std::string>();
    // Regs
    vars.insert("_a0");
    //vars.insert("_a1");
    //vars.insert("_a2");
    //vars.insert("_a3");
    outputSymbols.insert("_a0");
    //outputSymbols.insert("_a1");
    //outputSymbols.insert("_a2");
    //outputSymbols.insert("_a3");

	bodyss << "    asm(" << std::endl;
    bodyss << "    // main" << std::endl;
	outBlock(startblock, bodyss, outputSymbols, inputSymbols);
    //outputSymbols.insert("intformat");
    /* TODO: To be properly implemented
    body << "// funcdefs" << std::endl;
    for (auto funcdef: funcdefs){
        outBlock(*funcdef, body, outputSymbols, inputSymbols);
		body << pre_in << "ret" << post_in;
    }
    */
    bodyss << std::endl;
    bodyss << pre_in << "label_end:" << post_in;
    
    // Work ints
    mainss << "    // Working vars decl" << std::endl;
	for (std::string symbol : vars){
		mainss << "    long " << symbol << ";" << std::endl;
	}
    
    // Strings
    mainss << "    // Strings decl" << std::endl;
    for (std::pair<int, std::string> strp : strings){
        mainss << "    const char* " << "_s" << strp.first << " = " << strp.second << ";"<< std::endl;
    }

	mainss << bodyss.str();
	mainss << "    : // Output symbols" << std::endl;
	for (auto iter = outputSymbols.begin(); iter!=outputSymbols.end(); iter++){
		mainss << "      [" << *iter << "] \"+g\" (" << *iter << ")";
		if (std::next(iter) != outputSymbols.end())
			mainss << ",";
		mainss << std::endl;
	}
	mainss << "    : // Input symbols" << std::endl;
	for (auto iter = inputSymbols.begin(); iter!=inputSymbols.end(); iter++){
		mainss << "      [" << *iter << "] \"r\" (" << *iter << ")";
		if (std::next(iter) != inputSymbols.end())
			ss << ",";
		mainss << std::endl;
	}

    mainss << "    : // Clobbers" << std::endl;
    mainss << "      \"cc\", \"rax\", \"rbx\", \"rcx\", \"rdx\", \"rsi\", \"rdi\", \"%rsp\", \"r8\", \"r9\", \"r10\", \"r11\", \"r12\", \"r13\", \"r14\"" << std::endl;
	mainss << "    );" << std::endl;
	mainss << "}" << std::endl;

    ss << headss.str();
    ss << mainss.str();
}

std::string outBlock(BBlock& block, std::stringstream& ss, std::set<std::string>& outputSymbols, std::set<std::string>& inputSymbols){
    if (!block.label.empty()){
        return block.label;
    }
    else {
        block.label = newLabel();
        ss << std::endl;
        ss << pre_in << block.label << ":" << post_in;
        for (ThreeAd& op : block.instructions){
            convertThreeAd(op, ss, outputSymbols, inputSymbols);
        }

        std::stringstream falseblock;
        std::stringstream trueblock;
        std::string bname;
        if (block.falseExit != nullptr){
            bname = outBlock(*block.falseExit, falseblock, outputSymbols, inputSymbols);
            ss << pre_in << "jne " << bname << post_in;
        }
        if (block.trueExit != nullptr){
            bname = outBlock(*block.trueExit, trueblock, outputSymbols, inputSymbols);
            ss << pre_in << "jmp " << bname << post_in;
        }
        else {
            ss << pre_in << "jmp label_end" << post_in;
        }
        if (block.falseExit != nullptr)
            ss << falseblock.str();
        if (block.trueExit != nullptr)
            ss << trueblock.str();

    }
    return block.label;
}

bool is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}


void formatInput(std::string& val, std::set<std::string>& inputSymbols){
    std::stringstream tempss;
	inputSymbols.insert(val);
	tempss << "\%[" << val << "]";
	val = tempss.str();
}

void formatOutput(std::string& val, std::set<std::string>& outputSymbols){
    if (is_digits(val)){
        std::stringstream tempss;
        tempss << "$" << val;
        val = tempss.str();
	}
	else {
        if (val[0] == '"'){
            outputSymbols.erase(val);
            val = newString(val);
        }
        else {
            regVar(val);
        }
	    outputSymbols.insert(val);
        std::stringstream tempss;
		tempss << "\%[" << val << "]";
		val = tempss.str();
	}
}

void convertThreeAd(ThreeAd& op, std::stringstream& ss, std::set<std::string>& outputSymbols, std::set<std::string>& inputSymbols){
	regVar(op.result);
	
	std::string lhs = op.lhs;
	std::string rhs = op.rhs;

	switch(op.op){
		case '+':
			{
            formatOutput(rhs, outputSymbols);
            formatOutput(lhs, outputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "addq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax , %[" << op.result << "]" << post_in;
			}
			break;
		case '-':
			{
            formatOutput(rhs, outputSymbols);
            formatOutput(lhs, outputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "subq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax , [" << op.result << "]" << post_in;
			}
			break;
		case '*':
            {
            formatOutput(rhs, outputSymbols);
            formatOutput(lhs, outputSymbols);

            
            }
			break;
        case '/':
            {
            formatOutput(rhs, outputSymbols);
            formatOutput(lhs, outputSymbols);

            }
            break;
		case '=':
			{
            formatOutput(rhs, outputSymbols);
            formatOutput(lhs, outputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "cmp "  << lhs << ", %%rax" << post_in;
			}
			break;
		// Copy
		case 'c':
            {
            formatOutput(rhs, outputSymbols);
            formatOutput(lhs, outputSymbols);

			ss << pre_in << "movq " << rhs << ", " << "%%rax" << post_in;
			ss << pre_in << "movq " << "%%rax" << ", " << lhs << post_in;
            }
			break;
		// Label
		case 'l':
			break;
		// Goto
		case 'g':
			break;
		// Call function
		case 'f':
            {
            formatInput(rhs, inputSymbols);

            // Set arguments
            ss << pre_in << "movq %[_a0], %%rdi" << post_in;
            //ss << pre_in << "movq %[_a1], %%rsi" << post_in;
            //ss << pre_in << "movq %[_a2], %%rdx" << post_in;
            //ss << pre_in << "movq %[_a3], %%rcx" << post_in;

            
            ss << pre_in << "call *" << rhs << post_in;
            }
			break;
        default:
            std::cout << "Unknown 3ac" << std::endl;
            exit(-1);
	}
}


