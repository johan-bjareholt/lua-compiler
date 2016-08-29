%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%define api.token.constructor
%code requires{
	#include "globals.h"
	#include "ir.h"
}
%code{
    #include <string>
    #define YY_DECL yy::parser::symbol_type yylex()

    YY_DECL;

    Statement* root = new Statement('S');
}

%type <Statement*> block
%type <Statement*> chunk
%type <Statement*> chunk2
%type <Statement*> stat
%type <Statement*> laststat

%type <Statement*> ifblock
%type <Statement*> iflist
%type <Statement*> if
%type <Statement*> elseif
%type <Statement*> else

%type <Statement*> forblock

%type <Expression*> var
%type <std::list<Expression*>> varlist

%type <Expression*> exp
%type <Expression*> prefixexp
%type <std::list<Expression*>> explist

%type <std::string> funcname
%type <std::string> funcname2
%type <std::list<std::string>> namelist

%type <Statement*> function
%type <Statement*> functioncall
%type <Statement*> funcbody
%type <std::list<Expression*>> parlist
%type <Statement*> args

%type <Statement*> tableconstructor

%type <Expression*> field
%type <Statement*> fieldlist
%type <Statement*> fieldlist2
%type <std::string> optfieldsep
%type <std::string> fieldsep

%type <Expression*> string
%type <std::string> binop
%type <std::string> unop

%token <std::string> DO
%token <std::string> WHILE
%token <std::string> FOR
%token <std::string> UNTIL
%token <std::string> REPEAT
%token <std::string> END
%token <std::string> IN

%token <std::string> IF
%token <std::string> THEN
%token <std::string> ELSEIF
%token <std::string> ELSE

%token <std::string> LOCAL

%token <std::string> FUNCTION
%token <std::string> RETURN
%token <std::string> BREAK

%token <std::string> NIL
%token <std::string> FALSE
%token <std::string> TRUE
%token <std::string> NUMBER
%token <std::string> STRING
%token <std::string> TDOT
%token <std::string> NAME

%token <std::string> BINOP
%token <std::string> UNOP
%token <std::string> MINUS

%token <std::string> EQUALS
%token <std::string> DOT
%token <std::string> COLON
%token <std::string> COMMA
%token <std::string> SEMICOLON

%token <std::string> BRACES_L
%token <std::string> BRACES_R

%token <std::string> BRACKET_L
%token <std::string> BRACKET_R

%token <std::string> PARANTHESES_L
%token <std::string> PARANTHESES_R

%token QUIT 0 "end of file"

%%

block	: chunk
		{
			$$ = new Statement('S');
			$$->children.push_back($1);
			root = $$;
		}
		;

chunk	: chunk2 laststat {
	  		$$ = $1;
			$$->children.push_back($2);
	  	}
		| chunk2 {
	  		$$ = $1;
		}
		| laststat {
			$$ = new Statement('S');
			$$->children.push_back($1);
		}
		;

chunk2	: stat optsemi
	   	{	
			$$ = new Statement('S');
			$$->children.push_back($1);
		}
	   	| chunk stat optsemi {
			$$ = $1;
			$$->children.push_back($2);
		}
		;

optsemi	: SEMICOLON {}
		| /* empty */
		;

laststat: RETURN explist optsemi {
			//$$ = Node("return","explist");
			//$$.children.push_back($2);
			// TODO: Implement
			//$$ = $2;
			$$ = new Statement('S');
			$$->expressions.push_back(Constant(0));
		}
		| RETURN optsemi {
			//$$ = Node("return","empty");
			$$ = new Statement('S');
			$$->expressions.push_back(Constant(0));
		}
		| BREAK optsemi {
			//$$ = Node("return","break");
			//$$.children.push_back(Node("break",""));
			$$ = new Statement('S');
			$$->expressions.push_back(Constant(0));
		}
		;

stat	: namelist EQUALS explist {
			$$ = new Statement('S');
			int items = $1.size();
			for (int i=0; i<items; i++){
				$$->children.push_back(Assign($1.front(), $3.front()));
				$1.pop_front();
				$3.pop_front();
			}
		}
		| LOCAL namelist EQUALS explist {
			$$ = new Statement('S');
			int items = $2.size();
			for (int i=0; i<items; i++){
				$$->children.push_back(Assign($2.front(), $4.front()));
				$2.pop_front();
				$4.pop_front();
			}
		}
		| LOCAL namelist {
			$$ = new Statement('S');
			int items = $2.size();
			for (int i=0; i<items; i++){
				$$->children.push_back(Assign($2.front(), Constant(0)));
				$2.pop_front();
			}
		}
		| FUNCTION funcname funcbody {
			$$ = FunctionDef($2, {}, $3);
		}
		| LOCAL FUNCTION NAME funcbody {
			$$ = FunctionDef($3, {}, $4);
		}
		| functioncall {
			$$ = $1;
		}
		/*| DO block END {
			$$ = Node("do", "");
			$$.children.push_back($2);
		}
		| WHILE exp DO block END {
			$$ = Node("while","");
			$$.children.push_back($2);
			 $$.children.push_back($4);
		}
		| REPEAT block UNTIL exp {
			$$ = Node("repeat","");
			$$.children.push_back($2);
			$$.children.push_back($4);
		}*/
		| ifblock {
			$$ = $1;
		}
		| forblock {
			$$ = $1;
		}
		/*
		| FOR name EQUALS exp COMMA exp DO block END {
			$$ = Node("for","2var");
			$$.children.push_back($2);
			$$.children.push_back($4);
			$$.children.push_back($6);
			$$.children.push_back($8);
		}
		| FOR name EQUALS exp COMMA exp COMMA exp DO block END {
			$$ = Node("for","3var");
			$$.children.push_back($2);
			$$.children.push_back($4);
			$$.children.push_back($6);
			$$.children.push_back($8);
			$$.children.push_back($10);
		}
		| FOR namelist IN explist DO block END {
			$$ = Node("for","in");
			$$.children.push_back($2);
			$$.children.push_back($4);
			$$.children.push_back($6);
		}*/
	 	;

forblock: FOR NAME EQUALS exp COMMA exp DO block END {
			$$ = For($2, $4, $6, Constant(1), $8);
	 	}
		| FOR NAME EQUALS exp COMMA exp COMMA exp DO block END {
			$$ = For($2, $4, $8, $6, $10);
		}
		| FOR namelist IN explist DO block END {
			// Unimplemented
			std::cout << "Namelist type for is not supported!" << std::endl;
			exit(-1);
		}
		;

ifblock	: iflist else END {
			$$ = new Statement('S');
			Statement* exitBlock = new Statement('S');
			for (int i=0; i < $1->children.size(); i++){
				Statement* ifcond = $1->children.at(i);
				Statement* d;
				int elsei = i+1;
				if (elsei >= $1->children.size())
					d = $2;
				else
					d = $1->children.at(elsei);
				ifcond->children.at(1) = d;
				
			}
			$$->children.push_back($1->children.at(0));
			$$->children.push_back(exitBlock);
		}

iflist: if {
	  		$$ = new Statement('S');
			$$->children.push_back($1);
		}
		| iflist elseif {
			$$ = $1;
			$$->children.push_back($2);
		}
		;

if		: IF exp THEN block {
			$$ = If($2,$4,NULL);
		}
		;

elseif	: ELSEIF exp THEN block {
			$$ = If($2,$4,new Statement('S'));
		}
		;

else	: ELSE block {
			$$ = $2;
	 	}
		| /* empty */ {
			$$ = new Statement('S');
		}
		;

var		: NAME {
	 		/*
			$$ = Node("var", "name");
			$$.children.push_back($1);
			*/
			$$ = Variable($1);//Variable($1);
	 	}
		| prefixexp BRACKET_L exp BRACKET_R {
			/*
			$$ = Node("var","inbrackets");
			$$.children.push_back($1);
			$$.children.push_back($3);
			*/
		}
		| prefixexp DOT NAME {
			// TODO: Implement
			//$$ = $1;
			//$$.name
			$$ = $1;
		}
	 	;

varlist	: var {
			$$ = std::list<Expression*>();
			$$.push_back($1);
			/*
			$$ = Node("varlist","");
			$$.children.push_back($1);
			*/
		}
		| varlist COMMA var {
			$$ = $1;
			$$.push_back($3);
		}
		;

funcname: funcname2 {
			$$ = $1;
		}
		| funcname2 COLON NAME {
			// TODO: Implement
			//$$ = $1;
			//$$.children.push_back($3);
			std::stringstream ss;
			ss << $1 << $3;
			$$ = ss.str();
		}
		;

funcname2: NAME {
			//$$ = Node("funcname",$1.value);
			$$ = $1;
		}
		| funcname2 DOT NAME {
			//$$ = $1;
			//$$.value = $$.value +"."+ $3.value;
			std::stringstream ss;
			ss << $1 << $3;
			$$ = ss.str();
		}
		;

namelist: NAME {
			$$ = std::list<std::string>();
			$$.push_back($1);
		}
		| namelist COMMA NAME {
			$$ = $1;
			$$.push_back($3);
		}
		;

exp		: NIL {
			$$ = Constant(-1);
	 	}
	 	| FALSE {
			$$ = Constant(0);
		}
		| TRUE {
			$$ = Constant(1);
		}
		| NUMBER {
			$$ = Constant(std::stoi($1));
		}
		| string {
			$$ = $1; 
		}
		| TDOT {
			//$$ = Node("exp", $1);
		}
		| function {
			//$$ = Node("exp","function");
			//$$.children.push_back($1);
		}
		| prefixexp {
			$$ = $1;
		}
		| tableconstructor {
			//$$ = Node("exp","tableconstructor");
			//$$.children.push_back($1);
		}
		| exp binop exp {
			$$ = BinOp($2[0], $1, $3);
		}
		| unop exp {
			$$ = UnOp($1[0], $2);
		}
		;

explist	: exp {
			//$$ = Node("explist", "");
			//$$.children.push_back($1);
			$$ = std::list<Expression*>();
			$$.push_back($1);
		}
		| explist COMMA exp {
			$$ = $1;
			$$.push_back($3);
		}
		;

prefixexp: var {
			$$ = $1;
		}
		| functioncall {
			//$$ = $1;
		}
		| PARANTHESES_L exp PARANTHESES_R {
			$$ = $2;
		}
		;

function: FUNCTION funcbody {
			//$$ = Node("function","in-line");
			//$$.children.push_back($2);
		}
		;

functioncall: prefixexp args {
			//$$ = Node("funccall","");
			//$$.children.push_back($1);
			//$$.children.push_back($2);
			$$ = FunctionCall($1, $2);
		}
		| prefixexp COLON NAME args {
			//$$ = Node("funccall","2");
			//$$.children.push_back($1);
			//$$.children.push_back($3);
			//$$.children.push_back($4);
			$$ = FunctionCall($1, $4);
		}
		;

funcbody: PARANTHESES_L parlist PARANTHESES_R block END {
			//$$ = Node("funcbody","");
			//$$.children.push_back($2);
			//$$.children.push_back($4);
			$$ = $4;
			for (Expression* exp : $2){
				$$->expressions.push_back(exp);
			}
		}
		| PARANTHESES_L PARANTHESES_R block END {
			//$$ = Node("funcbody","");
			//$$.children.push_back(Node("parlist","empty"));
			//$$.children.push_back($3);
			$$ = $3;
		}
		;

parlist	: namelist {
			$$ = std::list<Expression*>();
			for (std::string name : $1)
				$$.push_back(Variable(name));
			//$$ = Node("parlist","namelist");
			//$$.children.push_back($1);
		}
		| namelist COMMA TDOT {
			$$ = std::list<Expression*>();
			for (std::string name : $1)
				$$.push_back(Variable(name));
			//$$ = $1;
			//$$.children.push_back(Node("argover",""));
		}
		| TDOT {
			$$ = std::list<Expression*>();
			//$$ = Node("parlist","tdot");
		}
		;

args	: PARANTHESES_L PARANTHESES_R {
	 		$$ = new Statement('S');
	 		//$$ = Node("explist","empty");
	 	}
		| PARANTHESES_L explist PARANTHESES_R {
			$$ = new Statement('S');
			for (Expression* exp : $2){
				$$->expressions.push_back(exp);
			}
			//$$ = $2;
		}
		| tableconstructor {
			//$$ = $1;
		}
		| string {
			$$ = new Statement('S');
			$$->expressions.push_back($1);
		}
		;

tableconstructor: BRACES_L fieldlist BRACES_R {
			//$$ = Node("tableconstructor","");
			//$$.children.push_back($2);
		}
		| BRACES_L BRACES_R {
			//$$ = Node("tableconstructor","empty");
		}
		;

field	: BRACKET_L exp BRACKET_R EQUALS exp {
	  		//$$ = Node("field","bracketequals");
			//$$.children.push_back($2);
			//$$.children.push_back($5);
	  	}
		| NAME EQUALS exp {
			//$$ = Node("field","equals");
			//$$.children.push_back($1);
			//$$.children.push_back($3);
			//$$ = Assign($1, $3);
		}
		| exp {
			//$$ = Node("field", "exp");
			//$$.children.push_back($1);
			$$ = $1;
		}
	  	;

fieldlist: fieldlist2 optfieldsep {
		 	//$$ = $1;
			//$$.children.push_back($1);
		}
		;

fieldlist2: field {
			//$$ = Node("fieldlist","");
			//$$.children.push_back($1);
		}
		| fieldlist2 fieldsep field {
			//$$ = $1;
			//$$.children.push_back($3);
		}

optfieldsep: fieldsep { $$ = $1; }
		| /* empty */ {}
		;

fieldsep: COMMA {
			$$ = ",";
		}
		| SEMICOLON {
			$$ = ",";
		}
		;

string	: STRING {
	   		$$ = String($1);
	   	}
		;

binop	: BINOP {
	 		$$ = $1;
	  		//$$ = Node("binop", $1);
	  	}
		| MINUS {
	 		$$ = $1;
			//$$ = Node("binop", $1);
		}
		;

unop	: UNOP {
	 		$$ = $1;
	 		//$$ = Node("unop", $1);
	 	}
		| MINUS {
	 		$$ = $1;
			//$$ = Node("unop", $1);
		}
		;
