#!/usr/bin/python3
# test_compare.py
import os
myOutput = os.popen("make dotest").read()
stdOutput = os.popen("../../bin/lexer test.cl").read()

beginIndex = myOutput.index("#name")
myOutput = myOutput[beginIndex:]
#print("{}\n\n\n{}".format(myOutput,stdOutput))


while myOutput and stdOutput:
    myEnd = myOutput.find("\n")
    stdEnd = stdOutput.find("\n")
    if myOutput[0 : myEnd] != stdOutput[0 : stdEnd]:
        print("my flex ", myOutput[0 : myEnd])
        print("std flex", stdOutput[0 : stdEnd])
        print("")

    myOutput = myOutput[myEnd + 1 :]
    stdOutput = stdOutput[stdEnd + 1 :]
