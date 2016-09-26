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
%type <Statement*> whileblock

%type <Expression*> var
%type <std::list<Expression*>> varlist

%type <Expression*> exp
%type <std::list<Expression*>> explist
%type <Expression*> prefixexp

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

%type <Expression*> op
%type <Expression*> op_1
%type <Expression*> op_2
%type <Expression*> op_3
%type <Expression*> op_4
%type <Expression*> op_5
%type <Expression*> op_6
%type <Expression*> op_7
%type <Expression*> op_8
%type <Expression*> op_9

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

%token <char> PLUS
%token <char> MINUS
%token <char> TIMES
%token <char> DIVIDE
%token <char> POWER
%token <char> MODULO

%token <char> EQUALS
%token <char> LESS_THAN
%token <char> MORE_THAN
%token <char> LESS_EQUAL_THAN
%token <char> MORE_EQUAL_THAN
%token <char> TILDE_EQUAL

%token <char> AND
%token <char> OR
%token <char> SQUARE
%token <char> NOT

%token <char> APPEND

%token <std::string> ASSIGN
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
			// TODO: Implement
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

stat	: varlist ASSIGN explist {
			$$ = new Statement('S');
			int items = $1.size();
			for (int i=0; i<items; i++){
                // TODO: Once tables work, remove ->name
				$$->children.push_back(Assign($1.front()->name, $3.front()));
				$1.pop_front();
				$3.pop_front();
			}
		}
		| LOCAL namelist ASSIGN explist {
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
		| DO block END {
			$$ = $2;
		}
        | whileblock {
            $$ = $1;
        }
		| REPEAT block UNTIL exp {
			$$ = Repeat($4, $2);
		}
		| ifblock {
			$$ = $1;
		}
		| forblock {
			$$ = $1;
		}
	 	;

forblock: FOR NAME ASSIGN exp COMMA exp DO block END {
			$$ = For($2, $4, $6, Constant(1), $8);
	 	}
		| FOR NAME ASSIGN exp COMMA exp COMMA exp DO block END {
			$$ = For($2, $4, $8, $6, $10);
		}
		| FOR namelist IN explist DO block END {
			// Unimplemented
			std::cout << "Namelist type for is not supported!" << std::endl;
			exit(-1);
		}
		;
		
whileblock: WHILE exp DO block END {
		    $$ = While($2, $4);
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
			$$ = Variable($1);
	 	}
		| prefixexp BRACKET_L exp BRACKET_R {
            // TODO: Implement
            //$$ = TableItem($1->name, $3);
		}
		| prefixexp DOT NAME {
			// TODO: Implement
            std::stringstream ss;
            ss << $1->name << "." << $3;
			
            $$ = $1;
			$$->name = ss.str();
		}
	 	;

varlist : var {
			$$ = std::list<Expression*>();
			$$.push_back($1);
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
		/*| tableconstructor {
            //$$ = $1;
			//$$ = Node("exp","tableconstructor");
			//$$.children.push_back($1);
		}*/
		| op {
			$$ = $1;
		}
		;

explist : exp {
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
	 	}
		| PARANTHESES_L explist PARANTHESES_R {
			$$ = new Statement('S');
			for (Expression* exp : $2){
				$$->expressions.push_back(exp);
			}
			//$$ = $2;
		}
		| tableconstructor {
			$$ = $1;
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

field	: BRACKET_L exp BRACKET_R ASSIGN exp {
	  		//$$ = Node("field","bracketequals");
			//$$.children.push_back($2);
			//$$.children.push_back($5);
	  	}
		| NAME ASSIGN exp {
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


/*
    Operator Priority
*/

op      : op_1 {
            $$ = $1;
        }
        ;

op_1    : op_1 OR op_2 {
			$$ = BinOp($2, $1, $3);
        }
        | op_2 {
            $$ = $1;
        }
        ;

op_2    : op_2 AND op_3 {
			$$ = BinOp($2, $1, $3);
        }
        | op_3 {
            $$ = $1;
        }
        ;

op_3    : op_3 LESS_THAN op_4 {
			$$ = BinOp($2, $1, $3);
        }
        | op_3 LESS_EQUAL_THAN op_4 {
			$$ = BinOp($2, $1, $3);
        }
        | op_3 MORE_THAN op_4 {
			$$ = BinOp($2, $1, $3);
        }
        | op_3 MORE_EQUAL_THAN op_4 {
			$$ = BinOp($2, $1, $3);
        }
        | op_3 TILDE_EQUAL op_4 {
			$$ = BinOp($2, $1, $3);
        }
        | op_3 EQUALS op_4 {
			$$ = BinOp($2, $1, $3);
        }
        | op_4 {
            $$ = $1;
        }
        ;

op_4    : op_4 APPEND op_5 {
			$$ = BinOp($2, $1, $3);
        }
        | op_5 {
            $$ = $1;
        }
        ;

op_5    : op_5 PLUS op_6 {
			$$ = BinOp($2, $1, $3);
        }
        | op_5 MINUS op_6 {
			$$ = BinOp($2, $1, $3);
        }
        | op_6 {
            $$ = $1;
        }
        ;

op_6    : op_6 TIMES op_7 {
			$$ = BinOp($2, $1, $3);
        }
        | op_6 DIVIDE op_7 {
			$$ = BinOp($2, $1, $3);
        } 
        | op_6 MODULO op_7 {
			$$ = BinOp($2, $1, $3);
        } 
        | op_7 {
            $$ = $1;
        }
        ;

op_7    : NOT op_8 {
			$$ = UnOp($1, $2);
        } 
        | SQUARE op_8 {
			$$ = UnOp($1, $2);
        } 
        | MINUS op_8 {
			$$ = UnOp($1, $2);
        } 
        | op_8 {
            $$ = $1;
        }
        ;

op_8    : op_8 POWER op_9 {
			$$ = BinOp($2, $1, $3);
        }
        | op_9 {
            $$ = $1;
        }
        ;

op_9    : exp {
            $$ = $1;
        }
        ;
