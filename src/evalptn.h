#pragma once
#include "board.h"

constexpr unsigned short pow3[]={1,3,9,27,81,243,729,2187,6561,19683,59049};

#ifndef ONLINE

extern unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];
int pow3to4(int len, int x);

void readShort(FILE *stream, short &tar);
void initPtnConfig();
void loadPtnData();
int evalPtn(const Board &board);

extern unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];

#endif //ONLINE
