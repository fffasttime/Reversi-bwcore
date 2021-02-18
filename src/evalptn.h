#pragma once
#include "board.h"

#ifndef ONLINE
extern unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];
int pow3to4(int len, int x);

void readShort(FILE *stream, short &tar);
void initPtnConfig();
void loadPtnData();
int evalPtn(const Board &board);
#endif //ONLINE
