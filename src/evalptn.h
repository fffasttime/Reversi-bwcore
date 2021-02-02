#pragma once
#include "board.h"

#define EVAL_FILE "../data/reversicoeff.bin"

constexpr int Eval_PrTable[61]={-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
constexpr unsigned short pow3[]={1,3,9,27,81,243,729,2187,6561,19683,59049};

struct CoeffPack{
    short e1[pow3[10]], c52[pow3[10]], c33[pow3[9]], //by pow4to3
        e2[1<<16], e3[1<<16], e4[1<<16], k8[1<<16], k7[1<<14], k6[1<<12], k5[1<<10], k4[1<<8], //directly
        wb, wodd, wmob;
};

extern CoeffPack pdata[12];

extern unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];
int pow3to4(int len, int x);

void readShort(FILE *stream, short &tar);
void initPtnConfig();
void loadPtnData();
int evalPtn(const Board &board, int col);
