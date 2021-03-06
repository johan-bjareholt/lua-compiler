#!/bin/bash

#
# This is essentially just a crash test, not a proper unit test
#

testpass=0
testcount=0

CC=gcc
#CC=gcc-5
#CC=gcc-4.9
#CC=clang

function ctest() {
	testcount=$(($testcount+1))
	echo "Crash testing $1"
	./lua $1 -o test.c
	if [ $? -ne 0 ]; then
		echo "Translation error: $1"
		failedtests=$failedtests$1
    else
        $CC test.c
        if [ $? -ne 0 ]; then
            echo "Compilation error: $1"
            failedtests=$failedtests$1
        else
            ./a.out
            if [ $? -ne 0 ]; then
                echo "Runtime error: $1"
                failedtests=$failedtests$1
            else
                testpass=$(($testpass+1))
            fi
        fi
	fi
}

ctest "tests/compile/arithmetic.lua"
ctest "tests/compile/basic.lua"
ctest "tests/compile/if.lua"
ctest "tests/compile/print.lua"
ctest "tests/compile/functions.lua"

ctest "tests/ass/test1.lua"
ctest "tests/ass/test2.lua"
ctest "tests/ass/test3.lua"
ctest "tests/ass/test4.lua"
ctest "tests/ass/test6.lua"

# Will not fix
#ctest "tests/ass/test5.lua"

printf "%d/%d crashtests passed\n" $testpass $testcount
