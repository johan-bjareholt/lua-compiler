#!/bin/bash

#
# This is essentially just a crash test, not a proper unit test
#

testpass=0
testcount=0

function test() {
	testcount=$(($testcount+1))
	echo "Grammar testing $1"
	./lua $1
	if [ $? -eq 0 ]; then
		testpass=$(($testpass+1))
	else
		echo "FAILED: $1"
		failedtests=$failedtests$1
	fi
}

test "tests/test1.lua"
test "tests/test2.lua"
test "tests/test3.lua"
test "tests/test4.lua"
test "tests/misc.lua"

printf "%d/%d tests passed\n" $testpass $testcount
