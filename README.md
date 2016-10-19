Lua Compiler
============

A Bison+Flex Lua 5.1 compiler.

Has complete grammar support, x86 target however is limited but works for a few basic testcases.

In test.sh you can see which test cases that should work so you can get a hint on what is implemented.

This is the second assignment in the course DV1465, Compiler Techniques at Blekinge Institute of Technology.

#### Supported compilers (versions tested)
- GCC (Compiler+target) (6.2.1, 5.4.0, 4.9.2)
- Clang (Compiler+target) (3.5.2)

#### Flaws

- Incomplete type system
- Function declarations can only have max 4 parameters
- Function calls currently only supports one parameter
- Stack logic needs improvements to save stack space
- Lots of memory leaks
- Max 30 operands bug for inline assembly
    - https://gcc.gnu.org/ml/gcc-help/2008-03/msg00109.html
    - http://stackoverflow.com/questions/39753703/how-to-prevent-error-more-than-30-operands-in-asm-with-inline-assesmbly-with
