import re

files=['util.h','util.cpp', 
'board.h', 'board.cpp', 
'evalptn.h', 'evalptn.cpp', 
'search.h', 'search.cpp',
'online.cpp']

head = '''#include <bits/stdc++.h>
'''
source = ''

for file in files:
    with open('src/'+file, 'r') as f:
        source += f.read() + '\n'

source = re.sub(r"#include.*\n", "", source);
source = re.sub(r"#pragma once.*\n", "", source);

source = re.sub(r"(.*)//.*\n","\g<1>\n",source);
source = re.sub(r"\/\*[^\/]*\*\/","",source);
source = re.sub(r"#ifdef DEBUGTREE(.|\n)*?#endif","",source);
source = re.sub(r"#ifndef ONLINE(.|\n)*?#endif","",source);
source = re.sub(r"#ifdef DEBUG(.|\n)*?#endif","",source);
source = re.sub(r"assertprintf\(.*?\);","",source);
source = re.sub(r"\s*\n","\n",source);
source = re.sub(r"\n+","\n",source);

with open('data/botzone.cpp', 'w') as f:
    f.write(head + source)
