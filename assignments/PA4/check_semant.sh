#!/bin/bash
test_set='./grading2/*.cl'
for i in $test_set
do
        ./mysemant $i >1.out
        ./stdsemant $i > 2.out
        if diff 1.out 2.out; then
                echo "AC!"
        else
                echo "WA!" $i
        fi
done
