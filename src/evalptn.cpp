#include "util.h"
#include "evalptn.h"
#include "bitptn.h"
#include <iostream>

unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];

CoeffPack pdata[12];

void readShort(FILE *stream, short &tar){fread(&tar, 2, 1, stream);}

void initPtnConfig(){
    inc(i,1<<20){
        int w=i&((1<<10)-1), b=(i>>10)&((1<<10)-1);
        if ((b&w)==0){
            int s=0;
            inc(j,10) s=s*3+(b>>j&1)*2+(w>>j&1);
            pow4to3_10[i]=s;
        }
    }
    inc(i,1<<18){
        int w=i&((1<<9)-1), b=(i>>9)&((1<<9)-1);
        if ((b&w)==0){
            int s=0;
            inc(j,9) s=s*3+(b>>j&1)*2+(w>>j&1);
            pow4to3_9[i]=s;
        }
    }
}
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

void loadPtnData(){
    FILE *in=fopen(EVAL_FILE, "rb");
    short part_cnt; readShort(in, part_cnt);
    inc(i,part_cnt){
        CoeffPack &p=pdata[i];
        readShort(in,p.wb);
        readShort(in,p.wodd);
        readShort(in,p.wmob);
        inc(j,pow3[10]) readShort(in,p.e1[j]);
        inc(j,pow3[10]) readShort(in,p.c52[j]);
        inc(j,pow3[9])  readShort(in,p.c33[j]);
        inc(j,pow3[8])  readShort(in,p.e2[pow3to4(8,j)]);
        inc(j,pow3[8])  readShort(in,p.e3[pow3to4(8,j)]);
        inc(j,pow3[8])  readShort(in,p.e4[pow3to4(8,j)]);
        inc(j,pow3[8])  readShort(in,p.k8[pow3to4(8,j)]);
        inc(j,pow3[7])  readShort(in,p.k7[pow3to4(7,j)]);
        inc(j,pow3[6])  readShort(in,p.k6[pow3to4(6,j)]);
        inc(j,pow3[5])  readShort(in,p.k5[pow3to4(5,j)]);
        inc(j,pow3[4])  readShort(in,p.k4[pow3to4(4,j)]);
        //cout<<p.wb<<'\n';
        //cout<<p.k4[80]<<'\n';
    }
    fclose(in);
}

int evalPtn(const Board &board, int col, int step){
    //if (eval_pr==10) eval_pr=9;
    const CoeffPack &p=pdata[Eval_PrTable[60-step]];

    Board b_id=board;
    if (col) b_id.cswap();

    int score=p.wb; //constant
    int cmob = popcnt(b_id.genmove(0));
    int codd = (60-step)%2;
    score += p.wodd*codd;
    score += p.wmob*cmob;

    using namespace bitptn;
    u64 x;
 
    #define OP_EXT(brd, mask, len)\
    x=(pext(brd.b[0], mask)<<len) + pext(brd.b[1], mask);
    #define S(expr) score+=expr
    OP_EXT(b_id, h[1], 8) S(p.e2[x]);
    OP_EXT(b_id, h[6], 8) S(p.e2[x]);
    OP_EXT(b_id, v[1], 8) S(p.e2[x]);
    OP_EXT(b_id, v[6], 8) S(p.e2[x]);
    OP_EXT(b_id, h[2], 8) S(p.e3[x]);
    OP_EXT(b_id, h[5], 8) S(p.e3[x]);
    OP_EXT(b_id, v[2], 8) S(p.e3[x]);
    OP_EXT(b_id, v[5], 8) S(p.e3[x]);
    OP_EXT(b_id, h[3], 8) S(p.e4[x]);
    OP_EXT(b_id, h[4], 8) S(p.e4[x]);
    OP_EXT(b_id, v[3], 8) S(p.e4[x]);
    OP_EXT(b_id, v[4], 8) S(p.e4[x]);
    OP_EXT(b_id, d1[7], 8) S(p.k8[x]);
    OP_EXT(b_id, d2[7], 8) S(p.k8[x]);
    OP_EXT(b_id, d1[6], 7) S(p.k7[x]);
    OP_EXT(b_id, d2[6], 7) S(p.k7[x]);
    OP_EXT(b_id, d1[8], 7) S(p.k7[x]);
    OP_EXT(b_id, d2[8], 7) S(p.k7[x]);
    OP_EXT(b_id, d1[5], 6) S(p.k6[x]);
    OP_EXT(b_id, d2[5], 6) S(p.k6[x]);
    OP_EXT(b_id, d1[9], 6) S(p.k6[x]);
    OP_EXT(b_id, d2[9], 6) S(p.k6[x]);
    OP_EXT(b_id, d1[4], 5) S(p.k5[x]);
    OP_EXT(b_id, d2[4], 5) S(p.k5[x]);
    OP_EXT(b_id, d1[10], 5) S(p.k5[x]);
    OP_EXT(b_id, d2[10], 5) S(p.k5[x]);
    OP_EXT(b_id, d1[3], 4) S(p.k4[x]);
    OP_EXT(b_id, d2[3], 4) S(p.k4[x]);
    OP_EXT(b_id, d1[11], 4) S(p.k4[x]);
    OP_EXT(b_id, d2[11], 4) S(p.k4[x]);
    
    OP_EXT(b_id, edge2x, 10) S(p.e1[pow4to3_10[x]]);
    OP_EXT(b_id, c33, 9) S(p.c33[pow4to3_9[x]]);
    OP_EXT(b_id, c52, 10) S(p.c52[pow4to3_10[x]]);

    Board b_v=b_id; b_v.flip_v();
    OP_EXT(b_v, c33, 9) S(p.c33[pow4to3_9[x]]);
    OP_EXT(b_v, c52, 10) S(p.c52[pow4to3_10[x]]);

    Board b_l=b_id; b_l.rotate_l();
    OP_EXT(b_l, edge2x, 10) S(p.e1[pow4to3_10[x]]);
    OP_EXT(b_l, c52, 10) S(p.c52[pow4to3_10[x]]);

    Board b_r=b_id; b_r.rotate_r();
    OP_EXT(b_r, edge2x, 10) S(p.e1[pow4to3_10[x]]);
    OP_EXT(b_r, c52, 10) S(p.c52[pow4to3_10[x]]);

    // +bswap
    b_id.flip_h(); b_v.flip_h(); b_l.flip_h(); b_r.flip_h();
    OP_EXT(b_id, c33, 9) S(p.c33[pow4to3_9[x]]);
    OP_EXT(b_id, c52, 10) S(p.c52[pow4to3_10[x]]);
    
    OP_EXT(b_v, c33, 9) S(p.c33[pow4to3_9[x]]);
    OP_EXT(b_v, c52, 10) S(p.c52[pow4to3_10[x]]);
    
    OP_EXT(b_l, c52, 10) S(p.c52[pow4to3_10[x]]);
    
    OP_EXT(b_r, c52, 10) S(p.c52[pow4to3_10[x]]);
    #undef OP_EXT
    #undef S
    return score;
}
