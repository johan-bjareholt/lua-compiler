#!/bin/bash

#
# This is essentially just a crash test, not a proper unit test
#

testpass=0
testcount=0

function ctest() {
	testcount=$(($testcount+1))
	echo "Crash testing $1"
	./lua $1
	if [ $? -eq 0 ]; then
		testpass=$(($testpass+1))
	else
		echo "FAILED: $1"
		failedtests=$failedtests$1
	fi
}

ctest "tests/test1.lua"
ctest "tests/test2.lua"
ctest "tests/test3.lua"
ctest "tests/test4.lua"
ctest "tests/misc.lua"

printf "%d/%d crashtests passed\n" $testpass $testcount
