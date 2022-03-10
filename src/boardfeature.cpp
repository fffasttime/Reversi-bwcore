#include "evalptn.h"

int gen_nnue_input(const Board &board, long long *feature){
    using namespace bitptn;
    int empty_cnt = popcnt(board.emptys());

    Board b_id=board;
    int cfeature=0; 

    #define S(expr) feature[cfeature++]=expr;
    S(1); //constant
    u64 moves=b_id.genmove();
    S(popcnt(moves));
    inc(i, 10) S(popcnt(moves&bsympos[i]));
    int codd = empty_cnt%2; S(codd);
    S(popcnt(b_id.b[0]&pinner)-popcnt(b_id.b[1]&pinner));
    int cedge=popcnt(b_id.b[0]&pedge)-popcnt(b_id.b[1]&pedge);
    S(cedge);
    S(cedge%2);

    u64 x;
    #define OP_EXT(brd, mask, len)\
    x=(pext(brd.b[0], mask)<<len) + pext(brd.b[1], mask);
    OP_EXT(b_id, h[1], 8) S(x);
    OP_EXT(b_id, h[6], 8) S(x);
    OP_EXT(b_id, v[1], 8) S(x);
    OP_EXT(b_id, v[6], 8) S(x);
    OP_EXT(b_id, h[2], 8) S(x);
    OP_EXT(b_id, h[5], 8) S(x);
    OP_EXT(b_id, v[2], 8) S(x);
    OP_EXT(b_id, v[5], 8) S(x);
    OP_EXT(b_id, h[3], 8) S(x);
    OP_EXT(b_id, h[4], 8) S(x);
    OP_EXT(b_id, v[3], 8) S(x);
    OP_EXT(b_id, v[4], 8) S(x);
    OP_EXT(b_id, d1[7], 8) S(x);
    OP_EXT(b_id, d2[7], 8) S(x);
    OP_EXT(b_id, d1[6], 7) S(x);
    OP_EXT(b_id, d2[6], 7) S(x);
    OP_EXT(b_id, d1[8], 7) S(x);
    OP_EXT(b_id, d2[8], 7) S(x);
    OP_EXT(b_id, d1[5], 6) S(x);
    OP_EXT(b_id, d2[5], 6) S(x);
    OP_EXT(b_id, d1[9], 6) S(x);
    OP_EXT(b_id, d2[9], 6) S(x);
    OP_EXT(b_id, d1[4], 5) S(x);
    OP_EXT(b_id, d2[4], 5) S(x);
    OP_EXT(b_id, d1[10], 5) S(x);
    OP_EXT(b_id, d2[10], 5) S(x);
    OP_EXT(b_id, d1[3], 4) S(x);
    OP_EXT(b_id, d2[3], 4) S(x);
    OP_EXT(b_id, d1[11], 4) S(x);
    OP_EXT(b_id, d2[11], 4) S(x);

    OP_EXT(b_id, ccor, 4) S(x);
    OP_EXT(b_id, cx22, 4) S(x);
    
    OP_EXT(b_id, edge2x, 10) S(pow4to3_10[x]);
    OP_EXT(b_id, c33, 9) S(pow4to3_9[x]);
    //OP_EXT(b_id, c3cor, 7) S(rand()%4096);
    OP_EXT(b_id, c52, 10) S(pow4to3_10[x]);

    Board b_v=b_id; b_v.flip_v();
    OP_EXT(b_v, c33, 9) S(pow4to3_9[x]);
    //OP_EXT(b_id, c3cor, 7) S(rand()%4096);
    OP_EXT(b_v, c52, 10) S(pow4to3_10[x]);

    Board b_l=b_id; b_l.rotate_l();
    OP_EXT(b_l, edge2x, 10) S(pow4to3_10[x]);
    OP_EXT(b_l, c52, 10) S(pow4to3_10[x]);

    Board b_r=b_id; b_r.rotate_r();
    OP_EXT(b_r, edge2x, 10) S(pow4to3_10[x]);
    OP_EXT(b_r, c52, 10) S(pow4to3_10[x]);

    // +bswap
    b_id.flip_h(); b_v.flip_h(); b_l.flip_h(); b_r.flip_h();
    OP_EXT(b_id, edge2x, 10) S(pow4to3_10[x]);
    OP_EXT(b_id, c33, 9) S(pow4to3_9[x]);
    //OP_EXT(b_id, c3cor, 7) S(rand()%4096);
    OP_EXT(b_id, c52, 10) S(pow4to3_10[x]);
    
    OP_EXT(b_v, c33, 9) S(pow4to3_9[x]);
    //OP_EXT(b_id, c3cor, 7) S(rand()%4096);
    OP_EXT(b_v, c52, 10) S(pow4to3_10[x]);
    
    OP_EXT(b_l, c52, 10) S(pow4to3_10[x]);
    
    OP_EXT(b_r, c52, 10) S(pow4to3_10[x]);
    #undef OP_EXT
    #undef S
    return cfeature;
}

unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];

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

extern "C"{

void init(){
    initPtnConfig();
}

int get_feature(u64 b0, u64 b1, int do_tf, long long *features){
    Board board(b0,b1);
    int tf = do_tf ? rand()%8 : 0;
    if (tf>4) tf-=4, board.flip_v();
    while (tf--) board.rotate_r();
    return gen_nnue_input(board, features);
}

int get_emptys(u64 b0, u64 b1){
    return popcnt(Board(b0, b1).emptys());
}

}