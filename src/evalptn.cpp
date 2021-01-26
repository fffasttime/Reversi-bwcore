#include "util.h"
#include "evalptn.h"

short pow4to3_10[1<<20], pow4to3_9[1<<18];

CoeffPack pdata[12];

void readShort(FILE *stream, short &tar){fread(&tar, 2, 1, stream);}

void initPtnConfig(){
    inc(i,1<<20){
        int b=i&((1<<10)-1), w=(i>>10)&((1<<10)-1);
        if (b&w==0){
            int s=0;
            inc(j,10) s=s*3+(b>>i&1)*2+(w>>i&1);
            pow4to3_10[i]=s;
        }
    }
    inc(i,1<<18){
        int b=i&((1<<9)-1), w=(i>>9)&((1<<9)-1);
        if (b&w==0){
            int s=0;
            inc(j,9) s=s*3+(b>>i&1)*2+(w>>i&1);
            pow4to3_9[i]=s;
        }
    }
}
int pow3to4(int len, int x){
    int b=0, w=0;
    inc(i,len){
        int p=x/pow3[i]%pow3[i+1];
        if (p==2) b|=1<<len;
        if (p==1) w|=1<<len;
    }
    return b<<len|w;
}

void initPtnData(){
    FILE *in=fopen(EVAL_FILE, "rb");
    short part_cnt; readShort(in, part_cnt);
    inc(i,part_cnt){
        CoeffPack &p=data[i];
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
    Bit cnt[3];
    map.countPiece(cnt);
    //if (eval_pr==10) eval_pr=9;
    const CoeffPack &p=pdata[Eval_PrTable[cnt[0]]];

    int score=p.wb; //constant
    int cmob = popcnt(board.genmove(col));
    int codd = cnt[0]%2;
    score += p.wodd*codd;
    score += p.wmob*cmob;
    
    
}
