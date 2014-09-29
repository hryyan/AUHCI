#!/usr/bin/python
import sys

file1 = open(sys.argv[1], "r")
file2 = open(sys.argv[2], "r")

l1 = file1.readlines()
l2 = file2.readlines()

for i in range(min(len(l1), len(l2))):
	if l1[i] != l2[i]:
		sys.stdout.write(l1[i])