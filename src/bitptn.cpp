#include "bitptn.h"

namespace bitptn{



byte ptn_flip[8][256][256][2];
bool ptn_canflip[8][256][256];

//assume player b takes move now
void lineFilpPtn(int p, byte b, byte w){
    auto gp=[](byte x, int p){return x>>p&1;};
    if (gp(b,p)||gp(w,p)){
        ptn_flip[p][b][w][0]=b;
        ptn_flip[p][b][w][1]=w;
        return;
    }
    byte new_b=b, new_w=w;
    bool ok=gp(w, p+1);
    for (int np=p+1;np<8;np++){
        if(gp(b,np)) break;
        else if (!gp(w,np)) ok=0;
    }
    ptn_canflip[p][b][w]|=ok;
    if (ok)
        for (int np=p+1;np<8;np++){
            if (gp(b,np)) break;
            else{
                new_b^=1<<np;
                new_w^=1<<np;
            }
        }
    ok=gp(w, p-1);
    for (int np=p-1;np>=0;np--){
        if(gp(b,np)) break;
        else if (!gp(w,np)) ok=0;
    }
    ptn_canflip[p][b][w]|=ok;
    if (ok)
        for (int np=p-1;np>=0;np--){
            if (gp(b,np)) break;
            else{
                new_b^=1<<np;
                new_w^=1<<np;
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