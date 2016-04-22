#include "ir.h"

// Some simple code fragements I used when testing.
Expression *e = BinOp('-', 
                      BinOp('+', Variable("y"), Constant(7)), 
                      Variable("x"));

Statement *s = Assign( "x",
                       BinOp('+',
                             Variable("x"),
                             Constant(1)
                       )
               );
Statement *ss = Seq( {s,s,s} );   // C++0x initialiser-list syntax


/* Parse tree represents:

   x := x+1
   if x==10 then
     y := x+1
   else
     y := x*2
   x := x+1
*/
Statement *testCase =
  Seq({                       // C++0x initialiser-list
    Assign( "x",
            BinOp('+',
              Variable("x"),
              Constant(1)
            )
    ),
    If( Equality( Variable("x"), Constant(10)),
        Assign( "y",
                BinOp('+',
                  Variable("x"),
                  Constant(1)
                )
        ),
        Assign( "y",
                BinOp('*',
                  Variable("x"),
                  Constant(2)
                )
        )
    ),
    Assign( "x",
            BinOp('+',
              Variable("x"),
              Constant(1)
            )
    )
  });

Statement* testCase2 =
  Seq({
    Assign( "x",
            BinOp('+',
              Variable("x"),
              Constant(1)
            )
    ),
    Assign( "y",
            BinOp('+',
              Variable("y"),
              Constant(1)
            )
    ),
    If( Equality( Variable("x"), Constant(0)),
      If( Equality( Variable("y"), Constant(0)),
        Assign("x", Constant(1)),
        Assign("y", Constant(2))
      ),
      Assign("y", Constant(3))
    )
  });

Statement *ifTest = If( Equality( Variable("x"), Constant(10)),
        Assign( "y",
                BinOp('+',
                  Variable("x"),
                  Constant(1)
                )
        ),
        Assign( "y",
                BinOp('*',
                  Variable("x"),
                  Constant(2)
                )
        )
    );

Expression *expressionTest = BinOp('+',
			BinOp('-', Variable("x"), Variable("y")),
			BinOp('+',Constant(2),
				BinOp('-',Constant(5),Constant(3)))
		);

int main()
{
  testCase->dump();
BBlock *start = new BBlock();
BBlock *current = start;
  convertStatement(testCase2, &current);
  convert(expressionTest, current);
  dumpCFG(start);
}


