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
	headss << "#include <stdlib.h>" << std::endl;
	headss << "#include <stdio.h>" << std::endl;
    headss << "long io_write(const char* msg){puts(msg); return 0;}" << std::endl;
    headss << "long print(int* num){char nstr[15]; sprintf(nstr,\"%d\", num); io_write(nstr); return 0;}" << std::endl;
    headss << "long io_read(const char* msg){ long num; scanf(\"%ld\", &num); return num; }" << std::endl;
	
    mainss << "int main(){" << std::endl;

	std::set<std::string> inputSymbols = std::set<std::string>();
	
    std::set<std::string> outputSymbols = std::set<std::string>();

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
	mainss << "    long ";
	for (auto iter = vars.begin(); iter != vars.end(); iter++){
	    mainss << *iter;
        if (std::next(iter) != vars.end())
            mainss << ", ";
        else
            mainss << ";" << std::endl;
	}
    
    // Strings
    mainss << "    // Strings decl" << std::endl;
    for (std::pair<int, std::string> strp : strings){
        mainss << "    const char* " << "_s" << strp.first << " = " << strp.second << ";"<< std::endl;
    }

	mainss << bodyss.str();
	mainss << "    : // Output symbols" << std::endl;
	for (auto iter = outputSymbols.begin(); iter!=outputSymbols.end(); iter++){
        if (inputSymbols.find(*iter) != inputSymbols.end()){
            inputSymbols.erase(*iter);
		    mainss << "      [" << *iter << "] \"+m\" (" << *iter << ")";
        }
        else
		    mainss << "      [" << *iter << "] \"=m\" (" << *iter << ")";
		if (std::next(iter) != outputSymbols.end())
			mainss << ",";
		mainss << std::endl;
	}
	mainss << "    : // Input symbols" << std::endl;
	for (auto iter = inputSymbols.begin(); iter!=inputSymbols.end(); iter++){
		mainss << "      [" << *iter << "] \"m\" (" << *iter << ")";
		if (std::next(iter) != inputSymbols.end())
			mainss << ",";
		mainss << std::endl;
	}

    mainss << "    : // Clobbers" << std::endl;
    mainss << "      \"cc\", \"rax\", \"rbx\", \"rcx\", \"rdx\", \"rsi\", \"rdi\", \"%rsp\"" << std::endl;
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


void formatSymbol(std::string& val, std::set<std::string>& symbolTable){
    if (is_digits(val)){
        std::stringstream tempss;
        tempss << "$" << val;
        val = tempss.str();
	}
	else {
        if (val[0] == '"'){
            symbolTable.erase(val);
            val = newString(val);
        }
        else {
            regVar(val);
        }
	    symbolTable.insert(val);
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
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);
            formatSymbol(op.result, outputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "addq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax , " << op.result << post_in;
			}
			break;
		case '-':
			{
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);
            formatSymbol(op.result, outputSymbols);

			ss << pre_in << "movq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "subq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax , " << op.result << post_in;
			}
			break;
		case '*':
            {
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);
            formatSymbol(op.result, outputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << lhs << ", %%rbx" << post_in;
			ss << pre_in << "imulq " << "%%rbx" << ", %%rax" << post_in;
			ss << pre_in << "movq " << "%%rax" << ", " << op.result << post_in;
            }
			break;
        case '/':
            {
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);
            formatSymbol(op.result, outputSymbols);
			
            ss << pre_in << "movq " << rhs << ", %%rbx" << post_in;
            ss << pre_in << "movq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "cqto" << post_in; // Sign extend %rax to %rdx:rax
			ss << pre_in << "idivq " << "%%rbx" << post_in;
			ss << pre_in << "movq " << "%%rax" << ", " << op.result << post_in;
            }
            break;
        case '%':
            {
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);
            formatSymbol(op.result, outputSymbols);
			
            ss << pre_in << "movq " << rhs << ", %%rbx" << post_in;
            ss << pre_in << "movq " << lhs << ", %%rax" << post_in;
			ss << pre_in << "cqto" << post_in; // Sign extend %rax to %rdx:rax
			ss << pre_in << "idivq " << "%%rbx" << post_in;
			ss << pre_in << "movq " << "%%rdx" << ", " << op.result << post_in;
            }
            break;
		case '=':
			{
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "cmp "  << lhs << ", %%rax" << post_in;
			}
			break;
		// Copy
		case 'c':
            {
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, outputSymbols);

			ss << pre_in << "movq " << rhs << ", " << "%%rax" << post_in;
			ss << pre_in << "movq " << "%%rax" << ", " << lhs << post_in;
            }
			break;
		// Call function
		case 'f':
            {
            formatSymbol(rhs, inputSymbols);
            formatSymbol(op.result, outputSymbols);

            // Set arguments
            // FIXME: Only supports one argument :/
            ss << pre_in << "movq " << rhs << ", %%rdi" << post_in;
           
            // Call
            ss << pre_in << "call " << lhs << post_in;
            // Move return 
            ss << pre_in << "movq " << "%%rax, " << op.result << post_in;
            }
			break;
        default:
            std::cout << "Unknown 3ac: " << op.op << std::endl;
            exit(-1);
	 }
}


