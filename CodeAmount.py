# -*- coding: utf-8 -*- 

import os

l = ['h', 'c', 'cpp', 'hpp']
num = 0
for i in os.walk(os.getcwd()):
    for j in i[2]:
        suffix = j.split('.')[-1]
        if suffix in l:
            with open(i[0]+"\\"+j, "r") as f:
                num += len(f.readlines())

print "Code Amount: ", num