import re

files=['util.h','util.cpp', 
'bitptn.h', 'bitptn.cpp', 
'board.h', 'board.cpp', 
'evalptn.h', 'evalptn.cpp', 
'search.h', 'search.cpp',
'cui.h', 'cui.cpp', 'main.cpp']

head = '''#include <bits/stdc++.h>
#define ONLINE
'''
source = ''

for file in files:
    with open('../src/'+file, 'r') as f:
        source += f.read() + '\n'

source = re.sub(r"#include.*\n", "", source);
source = re.sub(r"#pragma once.*\n", "", source);

with open('../data/botzone.cpp', 'w') as f:
    f.write(head + source)
