#include "util.h"
#include "evalptn.h"
#include <iostream>

#define EVAL_FILE "data/rawdata1/reversicoeff.bin"

constexpr int Eval_PrTable[61]={-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};

unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];

struct CoeffPack{
    short e1[pow3[10]], c52[pow3[10]], c33[pow3[9]], //by pow4to3
        e2[1<<16], e3[1<<16], e4[1<<16], k8[1<<16], k7[1<<14], k6[1<<12], k5[1<<10], k4[1<<8], //directly
        wb, wodd, wmob, wcinner[65], wcedge, wedgeodd, ccor[1<<8], cx22[1<<8];
}pdata[12];

void readShort(FILE *stream, short &tar){fread(&tar, 2, 1, stream);}

int pow3to4(int len, int x){
    int b=0, w=0;
    inc(i,len){
        b<<=1; w<<=1;
        int p=x/pow3[i]%3;
        if (p==2) b|=1;
        if (p==1) w|=1;
    }
    return b<<len|w;
}
void initPtnConfig(){
    inc(i,pow3[10]) pow4to3_10[pow3to4(10, i)]=i;
    inc(i,pow3[9]) pow4to3_9[pow3to4(9, i)]=i;
}

void loadPtnData(){
    FILE *in=fopen(EVAL_FILE, "rb");
    short format_version; readShort(in, format_version);
    assertprintf(format_version==1, "pattern data format doesn't match\n");
    short part_cnt; readShort(in, part_cnt);
    short type_cnt; readShort(in, type_cnt);
    short _; inc(i, type_cnt)  readShort(in, _);
    short checksum=0;
    auto rdc=[&](short &x){readShort(in, x); checksum^=x;};
    for (int i=1;i<1+part_cnt;i++){
        CoeffPack &p=pdata[i];
        rdc(p.wb);
        rdc(p.wodd);
        rdc(p.wmob);
        rdc(p.wcedge);
        rdc(p.wedgeodd);
        inc(j,65) rdc(p.wcinner[j]);
        inc(j,pow3[10]) rdc(p.e1[j]);
        inc(j,pow3[10]) rdc(p.c52[j]);
        inc(j,pow3[9])  rdc(p.c33[j]);
        inc(j,pow3[8])  rdc(p.e2[pow3to4(8,j)]);
        inc(j,pow3[8])  rdc(p.e3[pow3to4(8,j)]);
        inc(j,pow3[8])  rdc(p.e4[pow3to4(8,j)]);
        inc(j,pow3[8])  rdc(p.k8[pow3to4(8,j)]);
        inc(j,pow3[7])  rdc(p.k7[pow3to4(7,j)]);
        inc(j,pow3[6])  rdc(p.k6[pow3to4(6,j)]);
        inc(j,pow3[5])  rdc(p.k5[pow3to4(5,j)]);
        inc(j,pow3[4])  rdc(p.k4[pow3to4(4,j)]);
        inc(j,pow3[4])  rdc(p.ccor[pow3to4(4,j)]);
        inc(j,pow3[4])  rdc(p.cx22[pow3to4(4,j)]);
    }
    short file_checksum; readShort(in, file_checksum);
    assertprintf(checksum==file_checksum, "pattern file checksum failed\n");
    fclose(in);
}

int evalPtn(const Board &board){
    using namespace bitptn;
    int empty_cnt = popcnt(board.emptys());
    const CoeffPack &p=pdata[Eval_PrTable[empty_cnt]];

    Board b_id=board;

    int score=p.wb; //constant
    int cmob = popcnt(b_id.genmove());
    int codd = empty_cnt%2;
    score += p.wodd*codd;
    score += p.wmob*cmob;
    score += p.wcinner[popcnt(b_id.b[0]&pinner)-popcnt(b_id.b[1]&pinner)+32];
    int cedge=popcnt(b_id.b[0]&pedge)-popcnt(b_id.b[1]&pedge);
    score += p.wcedge*cedge;
    score += p.wedgeodd*(cedge%2);

    u64 x;
    #define OP_EXT(brd, mask, len)\
    x=(pext(brd.b[0], mask)<<len) + pext(brd.b[1], mask);
    #define S(expr) score+=p.expr
    OP_EXT(b_id, h[1], 8) S(e2[x]);
    OP_EXT(b_id, h[6], 8) S(e2[x]);
    OP_EXT(b_id, v[1], 8) S(e2[x]);
    OP_EXT(b_id, v[6], 8) S(e2[x]);
    OP_EXT(b_id, h[2], 8) S(e3[x]);
    OP_EXT(b_id, h[5], 8) S(e3[x]);
    OP_EXT(b_id, v[2], 8) S(e3[x]);
    OP_EXT(b_id, v[5], 8) S(e3[x]);
    OP_EXT(b_id, h[3], 8) S(e4[x]);
    OP_EXT(b_id, h[4], 8) S(e4[x]);
    OP_EXT(b_id, v[3], 8) S(e4[x]);
    OP_EXT(b_id, v[4], 8) S(e4[x]);
    OP_EXT(b_id, d1[7], 8) S(k8[x]);
    OP_EXT(b_id, d2[7], 8) S(k8[x]);
    OP_EXT(b_id, d1[6], 7) S(k7[x]);
    OP_EXT(b_id, d2[6], 7) S(k7[x]);
    OP_EXT(b_id, d1[8], 7) S(k7[x]);
    OP_EXT(b_id, d2[8], 7) S(k7[x]);
    OP_EXT(b_id, d1[5], 6) S(k6[x]);
    OP_EXT(b_id, d2[5], 6) S(k6[x]);
    OP_EXT(b_id, d1[9], 6) S(k6[x]);
    OP_EXT(b_id, d2[9], 6) S(k6[x]);
    OP_EXT(b_id, d1[4], 5) S(k5[x]);
    OP_EXT(b_id, d2[4], 5) S(k5[x]);
    OP_EXT(b_id, d1[10], 5) S(k5[x]);
    OP_EXT(b_id, d2[10], 5) S(k5[x]);
    OP_EXT(b_id, d1[3], 4) S(k4[x]);
    OP_EXT(b_id, d2[3], 4) S(k4[x]);
    OP_EXT(b_id, d1[11], 4) S(k4[x]);
    OP_EXT(b_id, d2[11], 4) S(k4[x]);

    OP_EXT(b_id, ccor, 4) S(ccor[x]);
    OP_EXT(b_id, cx22, 4) S(cx22[x]);
    
    OP_EXT(b_id, edge2x, 10) S(e1[pow4to3_10[x]]);
    OP_EXT(b_id, c33, 9) S(c33[pow4to3_9[x]]);
    OP_EXT(b_id, c52, 10) S(c52[pow4to3_10[x]]);

    Board b_v=b_id; b_v.flip_v();
    OP_EXT(b_v, c33, 9) S(c33[pow4to3_9[x]]);
    OP_EXT(b_v, c52, 10) S(c52[pow4to3_10[x]]);

    Board b_l=b_id; b_l.rotate_l();
    OP_EXT(b_l, edge2x, 10) S(e1[pow4to3_10[x]]);
    OP_EXT(b_l, c52, 10) S(c52[pow4to3_10[x]]);

    Board b_r=b_id; b_r.rotate_r();
    OP_EXT(b_r, edge2x, 10) S(e1[pow4to3_10[x]]);
    OP_EXT(b_r, c52, 10) S(c52[pow4to3_10[x]]);

    // +bswap
    b_id.flip_h(); b_v.flip_h(); b_l.flip_h(); b_r.flip_h();
    OP_EXT(b_id, edge2x, 10) S(e1[pow4to3_10[x]]);
    OP_EXT(b_id, c33, 9) S(c33[pow4to3_9[x]]);
    OP_EXT(b_id, c52, 10) S(c52[pow4to3_10[x]]);
    
    OP_EXT(b_v, c33, 9) S(c33[pow4to3_9[x]]);
    OP_EXT(b_v, c52, 10) S(c52[pow4to3_10[x]]);
    
    OP_EXT(b_l, c52, 10) S(c52[pow4to3_10[x]]);
    
    OP_EXT(b_r, c52, 10) S(c52[pow4to3_10[x]]);
    #undef OP_EXT
    #undef S
    return score;
}
