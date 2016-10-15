Lua Compiler
============

A Bison+Flex Lua 5.1 compiler.

Has complete grammar support, x86 target however is unfinished.

In test.sh you can see which test cases that should work so you can get a hint on what is implemented.

This is the second assignment in the course DV1465, Compiler Techniques at Blekinge Institute of Technology.

#### Flaws

- Incomplete type system
- Function declarations can only have max 4 parameters
- Function calls currently only supports one parameter
- User defined recursive functions are broken (arguments doesn't use the stack)
- Lots of memory leaks
- Max 30 operands bug for inline assembly
    - https://gcc.gnu.org/ml/gcc-help/2008-03/msg00109.html
    - http://stackoverflow.com/questions/39753703/how-to-prevent-error-more-than-30-operands-in-asm-with-inline-assesmbly-with
