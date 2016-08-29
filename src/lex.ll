%top{
    #include "grammar.tab.hh"
	#include "globals.h"
    #define YY_DECL yy::parser::symbol_type yylex()

	int linenr = 0;

	void log(std::string message, std::string text){
		if (debug_lex == true)
			std::cout << message << ": " << text << std::endl;
	}
}

%option noyywrap nounput batch noinput

%%

 /* comments */
--\[\[([^\]])*\]\]--					{ log("comment",yytext); }
--[^\n]*								{ log("comment",yytext); }

 /*
	Reserved keywords
 */
 /* Looping */
do										{ log("do", yytext); return yy::parser::make_DO(yytext); }
while									{ log("while", yytext); return yy::parser::make_WHILE(yytext); }
for										{ log("for", yytext); return yy::parser::make_FOR(yytext); }
until									{ log("until", yytext); return yy::parser::make_UNTIL(yytext); }
repeat									{ log("repeat", yytext); return yy::parser::make_REPEAT(yytext); }
end										{ log("end", yytext); return yy::parser::make_END(yytext); }
in										{ log("in", yytext); return yy::parser::make_IN(yytext); }
 /* if/else statements*/
if										{ log("if", yytext); return yy::parser::make_IF(yytext); }
then									{ log("then", yytext); return yy::parser::make_THEN(yytext); }
elseif									{ log("elseif", yytext); return yy::parser::make_ELSEIF(yytext); }
else									{ log("else", yytext); return yy::parser::make_ELSE(yytext); }
 
 /* Token categories */
([+*/^%<>]|\.\.|<=|>=|==|~=|and|or)		{ log("binop",yytext); return yy::parser::make_BINOP(yytext); }
([#]|not)								{ log("unop",yytext); return yy::parser::make_UNOP(yytext); }
-										{ log("minus(unop/binop)",yytext); return yy::parser::make_MINUS(yytext); }

 /*  */
local									{ log("local", yytext); return yy::parser::make_LOCAL(yytext); }

 /* function */
function								{ log("function",yytext); return yy::parser::make_FUNCTION(yytext); }
return									{ log("return",yytext); return yy::parser::make_RETURN(yytext); }
break									{ log("break",yytext); return yy::parser::make_BREAK(yytext); }

 /* Values */
nil										{ log("nil", yytext); return yy::parser::make_NIL(yytext);}
false									{ log("false", yytext); return yy::parser::make_FALSE(yytext); }
true									{ log("true", yytext); return yy::parser::make_TRUE(yytext);}
[0-9]+									{ log("number",yytext); return yy::parser::make_NUMBER(yytext);}
\"[^\"]*\"								{ log("string",yytext); return yy::parser::make_STRING(yytext);}
\.\.\.									{ log("tdot",yytext); return yy::parser::make_TDOT(yytext);}
[A-Za-z_][A-Za-z0-9_]*					{ log("name",yytext); return yy::parser::make_NAME(yytext); }



 /* Single tokens */
=										{ log("equals",yytext); return yy::parser::make_EQUALS(yytext); }
\.										{ log("dot",yytext); return yy::parser::make_DOT(yytext); }
:										{ log("colon",yytext); return yy::parser::make_COLON(yytext); }
,										{ log("comma",yytext); return yy::parser::make_COMMA(yytext); }
;										{ log("semicolon",yytext); return yy::parser::make_SEMICOLON(yytext); }

 /* blocks */
\(										{ log("parentheses_l",yytext); return yy::parser::make_PARANTHESES_L(yytext); }
\)										{ log("parantheses_r",yytext); return yy::parser::make_PARANTHESES_R(yytext); }
\{										{ log("braces_l", yytext); return yy::parser::make_BRACES_L(yytext); }
\}										{ log("braces_r", yytext); return yy::parser::make_BRACES_R(yytext); }
[\[]									{ log("bracket_l",yytext); return yy::parser::make_BRACKET_L(yytext); }
[\]]									{ log("bracket_r",yytext); return yy::parser::make_BRACKET_R(yytext); }


[ \t]									{ /* spacing */}
[\n]									{ linenr++; }
<<EOF>>                 				{ log("end", ""); return yy::parser::make_QUIT(); }

%%
void set_input_file(char* filename)
{
	yyin = fopen(filename, "r");
}
