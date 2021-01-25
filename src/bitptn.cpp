#include "bitptn.h"

namespace bitptn{

u8 ptn_flip[8][256][256][2];
bool ptn_canflip[8][256][256];

//assume player b takes move now
void lineFilpPtn(int p, u8 b, u8 w){
    if (bget(b,p)||bget(w,p)){
        ptn_flip[p][b][w][0]=b;
        ptn_flip[p][b][w][1]=w;
        return;
    }
    u8 new_b=b, new_w=w;
    int np;
    if (p<6 && bget(w, p+1)){
        for (np=p+2;np<8 && bget(w,np);np++);
        if (np<8 && bget(b,np)){
            ptn_canflip[p][b][w]=1;
            for (--np;np-p;np--)
                new_b^=1<<np, new_w^=1<<np;
            new_b|=1<<p;
        }
    }
    if (p>1 && bget(w, p-1)){
        for (np=p-2;np>=0 && bget(w,np);np--);
        if (np>=0 && bget(b,np)){
            ptn_canflip[p][b][w]=1;
            for (++np;np-p;np++)
                new_b^=1<<np, new_w^=1<<np;
            new_b|=1<<p;
        }
    }
    ptn_flip[p][b][w][0]=new_b;
    ptn_flip[p][b][w][1]=new_w;
}

void initPtnFlip(){
    for (int p=0;p<8;p++){
        for (int b=0;b<256;b++)
            for (int w=0;w<256;w++)
                if((b&w)==0) lineFilpPtn(p,b,w);
    }
}

}