#include "convert.h"
#include "ops.h"
#include <cctype>
#include <algorithm>

static const std::string pre_in = "    \"    ";
static const std::string post_in = "\\n\"\n";

enum VAR_TYPE {
    VT_STRING,
    VT_INT,
    VT_POINTER,
    VT_FUNCTION,
    VT_LABEL,
    VT_UNKNOWN
};


static std::map<std::string, std::pair<int, std::string>> vars2;

static std::pair<int, std::string>* getVar(std::string name){
    auto result = vars2.find(name);
    if (result == vars2.end())
        return nullptr;
    else
        return &(*result).second;
}

static void addVar(std::string name, int type, std::string value){
    if (getVar(name) != nullptr){
        if (getVar(name)->first != VT_UNKNOWN)
            std::cout << "overriding value of " << name << "!" << std::endl;
        vars2.erase(name);
    }
    std::pair<int, std::string> var(type, value);
    std::pair<std::string, std::pair<int, std::string>> entry(name, var);
    vars2.insert(entry);
}

static int stringc = 0;
static std::string newString(std::string str){
    std::stringstream tempss;
    tempss << "_s" << stringc;
    std::string varname = tempss.str();
    addVar(varname, VT_STRING, str);
    stringc++;
    return varname;
}

static int varc = 0;
static void regVar(std::string str){
    if (getVar(str) == nullptr)
        addVar(str, VT_UNKNOWN, "");
}

static int labelcount = 0;
std::string newLabel(){
	std::stringstream ss;
	ss << "label" << to_string(labelcount);
    std::string varname = ss.str();
    addVar(varname, VT_LABEL, "");
	labelcount++;
	return varname;
}




void outMainBlock(std::stringstream& ss, BBlock& startblock){
    std::stringstream headss;
    std::stringstream mainss;
    std::stringstream bodyss;
    
    // Standard functions
    headss << "// Standard functions" << std::endl;
	headss << "#include <stdlib.h>" << std::endl;
	headss << "#include <stdio.h>" << std::endl;
    headss << "long io_write(const char* msg){printf(msg); return 0;}" << std::endl;
    headss << "long io_write_i(long num){ printf(\"%ld\", num); return 0;}" << std::endl;
    headss << "long print(const char* msg){ printf(\"%s\\n\", msg); return 0;}" << std::endl;
    headss << "long print_i(long num){ printf(\"%ld\\n\", num);; return 0;}" << std::endl;
    headss << "long io_read(const char* msg){ long num; scanf(\"%ld\", &num); return num; }" << std::endl;
	
    mainss << "int main(){" << std::endl;

	std::set<std::string> inputSymbols = std::set<std::string>();
	
    std::set<std::string> outputSymbols = std::set<std::string>();

	bodyss << "    asm (" << std::endl;
    bodyss << "    // main" << std::endl;
    // Output global
	outBlock(startblock, bodyss, outputSymbols, inputSymbols, true);
    // Output functions
    for (auto funcentry : funcdefs){
        std::string funcname = funcentry.first;
        BBlock* funcblock = funcentry.second;
        outFunctionBlock(funcname, *funcblock, bodyss, outputSymbols, inputSymbols);
    }
    bodyss << std::endl;
    bodyss << pre_in << "label_end:" << post_in;
    
    // Define storage variables
    for (auto var : vars2){
        std::string name = var.first;
        std::string val = var.second.second;
        int type = var.second.first;
        switch (type){
            case VT_STRING:
                mainss << "    const char* " << name << " = " << val << ";"<< std::endl;
                break;
            case VT_INT:
            case VT_UNKNOWN:
	            mainss << "    long " << name << ";" << std::endl;
                break;
            case VT_POINTER:
                mainss << "    void* " << name << ";" << std::endl;
                break;
            case VT_LABEL:
                break;
            default:
                std::cout << "Unknown vartype" << std::endl;
                exit(1);
                break;
        }
    }

    // Strings
    //mainss << "    // Strings decl" << std::endl;
    //for (std::pair<int, std::string> strp : strings){
    //    mainss << "    const char* " << "_s" << strp.first << " = " << strp.second << ";"<< std::endl;
    //}

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
    mainss << "      \"cc\", \"rax\", \"rbx\", \"rcx\", \"rdx\", \"rsi\", \"rdi\", \"rsp\"" << std::endl;
    //mainss << "      ,\"r8\", \"r9\", \"r10\", \"r11\", \"r12\", \"r13\", \"r14\", \"r15\"" << std::endl;
	mainss << "    );" << std::endl;
	mainss << "}" << std::endl;

    ss << headss.str();
    ss << mainss.str();
}

std::string outBlock(BBlock& block, std::stringstream& ss, std::set<std::string>& outputSymbols, std::set<std::string>& inputSymbols, bool exit){
    if (!block.label.empty()){
        return block.label;
    }
    else {
        block.label = newLabel();
        ss << std::endl;
        ss << pre_in << block.label << ":" << post_in;
        ThreeAd* last_op = nullptr;
        for (ThreeAd& op : block.instructions){
            convertThreeAd(op, ss, outputSymbols, inputSymbols);
            last_op = &op;
        }

        std::stringstream falseblock;
        std::stringstream trueblock;
        std::string bname;
        if (block.falseExit != nullptr && last_op){
            bname = outBlock(*block.falseExit, falseblock, outputSymbols, inputSymbols, exit);
            switch (last_op->op){
                case '=':
                    ss << pre_in << "jne " << bname << post_in;
                    break;
                case '<':
                    ss << pre_in << "jle " << bname << post_in;
                    break;
                case '>':
                    ss << pre_in << "jge " << bname << post_in;
                    break;
                default:
                    std::cout << "Unreachable block: " << bname << std::endl;
                    break;
            }
        }
        if (block.trueExit != nullptr){
            bname = outBlock(*block.trueExit, trueblock, outputSymbols, inputSymbols, exit);
            ss << pre_in << "jmp " << bname << post_in;
        }
        else if (exit){
            ss << pre_in << "jmp label_end" << post_in;
        }
        else {
            ss << pre_in << "addq " << "$16, %%rsp" << post_in;
            ss << pre_in << "ret" << post_in;
        }
        if (block.falseExit != nullptr)
            ss << falseblock.str();
        if (block.trueExit != nullptr)
            ss << trueblock.str();

    }
    return block.label;
}

static std::map<std::string, int> args;

void outFunctionBlock(std::string funcname, BBlock& block, std::stringstream& ss, std::set<std::string>& outputSymbols, std::set<std::string>& inputSymbols){
    //std::cout << "Translating function " << funcname << std::endl;
    ss << std::endl;
    ss << pre_in << funcname << ":" << post_in;
    ss << pre_in << "subq " << "$16, %%rsp" << post_in;
    /*for (ThreeAd& op : block.instructions){
        convertThreeAd(op, ss, outputSymbols, inputSymbols);
    }*/
    args.clear();
    int i=0;
    for(auto iter = block.instructions.begin(); iter->op=='a' && iter!=block.instructions.end(); iter++){
        ThreeAd& arg = *iter;
        args.insert(std::make_pair(arg.lhs, i));
        i++;
    }
	outBlock(block, ss, outputSymbols, inputSymbols, false);
    for (auto arg: args){
        
    }
    args.clear();
}

bool is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

bool is_string(const std::string& str){
    return (str.front() == '"' && str.back() == '"');
}

void add_brackets(std::string& str){
    std::stringstream tmpss;
    tmpss << "%[" << str << "]";
    str = tmpss.str();
}

void formatSymbol(std::string& val, std::set<std::string>& symbolTable){
    auto arg = args.find(val);
    if (arg != args.end()){
        val = "-8(%%rsp)";
    }
    else if (is_digits(val)){
        std::stringstream tempss;
        tempss << "$" << val;
        val = tempss.str();
	}
	else {
        if (is_string(val)){
            symbolTable.erase(val);
            val = newString(val);
        }
        else {
            regVar(val);
        }
	    symbolTable.insert(val);
        add_brackets(val);
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
        case '<':
        case '>':
			{
            formatSymbol(rhs, inputSymbols);
            formatSymbol(lhs, inputSymbols);

			ss << pre_in << "movq " << rhs << ", %%rax" << post_in;
			ss << pre_in << "movq " << lhs << ", %%rbx" << post_in;
			ss << pre_in << "cmp "  << "%%rbx, %%rax" << post_in;
			}
			break;
		// Copy
		case 'c':
            {
            if (is_string(rhs)){
                //std::cout << "test1: " << lhs << "," << rhs << std::endl;
                formatSymbol(rhs, inputSymbols);
                addVar(lhs, VT_POINTER, "");
                add_brackets(lhs);
            }
            else if (is_digits(rhs)){
                formatSymbol(lhs, inputSymbols);
                std::stringstream tempss;
                tempss << "$" << rhs;
                rhs = tempss.str();
            }
            else {
                formatSymbol(lhs, inputSymbols);
                formatSymbol(rhs, inputSymbols);
            }
            ss << pre_in << "movq " << rhs << ", " << "%%rax" << post_in;
            ss << pre_in << "movq " << "%%rax" << ", " << lhs << post_in;
            }
			break;
        // Function argument
        case 'a':
            // TODO: Support for multiple arguments
            formatSymbol(lhs, inputSymbols);
            // Push to stack
            ss << pre_in << "movq %%rdi, " << lhs << post_in;
            break;
        // Function Return
		case 'r':
            {
            formatSymbol(lhs, inputSymbols);
			ss << pre_in << "movq " << lhs << ", " << "%%rax" << post_in;
            
            ss << pre_in << "addq " << "$16, %%rsp" << post_in;
            ss << pre_in << "ret" << post_in;
            }
			break;
		// Call function
		case 'f':
            {
            // FIXME: Ugly type check and convertion
            std::string funcname = lhs;
            auto arginfo = vars2.find(rhs);
            if (arginfo != vars2.end())
                //std::cout << "Type: " << (*arginfo).second.first << "," << (*arginfo).first << "," << (*arginfo).second.second << std::endl;
            if ((funcname == "print" || funcname == "io_write") && (is_digits(rhs) || (arginfo != vars2.end() && ((*arginfo).second.first == VT_INT) || (*arginfo).second.first == VT_UNKNOWN))){
                if (lhs == "print")
                    funcname = "print_i";
                else
                    funcname = "io_write_i";
            }
            
            formatSymbol(op.result, outputSymbols);
            formatSymbol(rhs, inputSymbols);

            // Set arguments
            // FIXME: Only supports one argument :(
            ss << pre_in << "movq " << rhs << ", %%rdi" << post_in;
           
            // Call
            ss << pre_in << "call " << funcname << post_in;
            // Move return 
            ss << pre_in << "movq " << "%%rax, " << op.result << post_in;
            }
			break;
        default:
            std::cout << "Unknown 3ac: " << op.op << std::endl;
            exit(-1);
	 }
}


