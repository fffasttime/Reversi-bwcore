import re

files=['util.h','util.cpp', 
'board.h', 'board.cpp', 
'evalptn.h', 'evalptn.cpp', 
'search.h', 'search.cpp',
'online.cpp']

head = '''/*
Author: fffasttime
See https://github.com/fffasttime/Reversi-bwcore
*/
#include <bits/stdc++.h>
'''
source = ''

for file in files:
    with open('src/'+file, 'r') as f:
        source += f.read() + '\n'

source = re.sub(r"#include.*\n", "", source)
source = re.sub(r"#pragma once.*\n", "", source)

source = re.sub(r"#ifndef ONLINE(.|\n)*?#endif //ONLINE","",source)
source = re.sub(r"#if 1([\s\S]*?)#endif //1","\g<1>", source);
source = re.sub(r"#ifdef GENDATA_PC(.|\n)*?#endif //GENDATA_PC","", source);
source = re.sub(r"(.*)//.*\n",r"\g<1>\n",source)
source = re.sub(r"\/\*[^\/]*\*\/","",source)
source = re.sub(r"assertprintf\(.*?\);","",source)
source = re.sub(r"#ifdef DEBUGTREE(.|\n)*?#endif","",source)
source = re.sub(r"#ifdef DEBUG(.|\n)*?#endif","",source)

source = re.sub(r"#ifdef RUN_BY_STEP([\s\S]*)#else([\s\S]*)#endif","\g<2>", source);

source = re.sub(r"\s*\n","\n",source)
source = re.sub(r"\n+","\n",source)

with open('data/botzone.cpp', 'w') as f:
    f.write(head + source)
