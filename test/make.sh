rm ./a
gcc test.c -I../ -std=c99 -Wall -Wextra -g
./a

./a > output.txt

if git --no-pager diff -u --word-diff --color=always \
       --no-index good_out.txt output.txt; then
	echo "Test passed: Output matches expected output."
else
	echo "Test failed: Output differs from expected output."
fi
