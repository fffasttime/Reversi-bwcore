#include "util.h"
#include "board.h"
#include "evalptn.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <iomanip>
#include <vector>
#include <cstring>
using std::string, std::cin, std::cout;

typedef std::pair<Board,double> DataType;
std::vector<DataType> data, data_test;
string folder;
int phase;

// type of data is double when training
struct CoeffPack{
    double e1[pow3[10]], c52[pow3[10]], c33[pow3[9]], //by pow4to3
        e2[1<<16], e3[1<<16], e4[1<<16], k8[1<<16], k7[1<<14], k6[1<<12], k5[1<<10], k4[1<<8], //directly
        wb, wodd, wmob;
}ptn, ptngrad;
const int p_begins[] = {2,5, 8,11,14,18,22,27,32,38,44};
const int p_ends[]   = {4,7,10,13,17,21,26,31,37,43,60};

void writePtnData(){
    FILE *out=fopen((string("data/")+folder+"/coeff"+std::to_string(p_begins[phase])
    +"_"+std::to_string(p_ends[phase])+".bin").c_str(), "wb");
    auto &p=ptn;
    auto ws=[&](short x){fwrite(&x, 2, 1, out);};
    ws(p.wb);
    inc(j,pow3[10]) ws(p.e1[j]);
    inc(j,pow3[10]) ws(p.c52[j]);
    inc(j,pow3[9])  ws(p.c33[j]);
    inc(j,pow3[8])  ws(p.e2[pow3to4(8,j)]);
    inc(j,pow3[8])  ws(p.e3[pow3to4(8,j)]);
    inc(j,pow3[8])  ws(p.e4[pow3to4(8,j)]);
    inc(j,pow3[8])  ws(p.k8[pow3to4(8,j)]);
    inc(j,pow3[7])  ws(p.k7[pow3to4(7,j)]);
    inc(j,pow3[6])  ws(p.k6[pow3to4(6,j)]);
    inc(j,pow3[5])  ws(p.k5[pow3to4(5,j)]);
    inc(j,pow3[4])  ws(p.k4[pow3to4(4,j)]);

    fclose(out);
}

void loadData(){
    string filename="data/"+folder+"/data"+
        std::to_string(p_begins[phase])+"_"+std::to_string(p_ends[phase])+".txt";
    std::ifstream fin(filename);
    if (!fin.is_open()){
        cout<<"file "<<filename<<" doesn't exist\n";
        exit(1);
    }
    Board b; double val;
    while (fin>>std::hex>>b.b[0]>>b.b[1]){
        fin>>val;
        data.emplace_back(b, val);
    }
}

double evalptn(const Board &board){
    int empty_cnt = popcnt(board.emptys());
    auto &p=ptn;

    Board b_id=board;

    double score=p.wb; //constant
    int cmob = popcnt(b_id.genmove());
    int codd = empty_cnt%2;
    score += p.wodd*codd;
    score += p.wmob*cmob;

    using namespace bitptn;
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
void adjustptn(const Board &board, double grad){
    int empty_cnt = popcnt(board.emptys());
    auto &p=ptngrad;

    Board b_id=board;

    p.wb+=grad/2; //constant
    int cmob = popcnt(b_id.genmove());
    int codd = empty_cnt%2;
    p.wodd+=grad*codd/2;
    p.wmob+=grad*cmob/8;

    using namespace bitptn;
    u64 x;
 
    #define OP_EXT(brd, mask, len)\
    x=(pext(brd.b[0], mask)<<len) + pext(brd.b[1], mask);
    #define S(expr) p.expr+=grad
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
}

// square error
double SE(DataType &d){
    double err=evalptn(d.first)-d.second;
    return err*err;
}
// absluate error
double AE(DataType &d){
    return fabs(evalptn(d.first)-d.second);
}
std::mt19937 rng;
void testError(){
    double sse=0,sae=0, mae=0;
    for (auto &d:data_test){
        sse+=SE(d);
        sae+=AE(d);
        mae=std::max(mae,AE(d));
    }
    sse/=data_test.size();
    sae/=data_test.size();
    printf("TEST mae: %f,  sse: %f,  sae: %f\n", mae, sse, sae);
}
void updateArgs(){
    for (size_t i=0;i<sizeof(CoeffPack)/sizeof(double);i++)
        ((double *)(&ptn))[i]+=((double *)(&ptngrad))[i];
    memset(&ptngrad,0,sizeof(CoeffPack));
}
void accuGrad(DataType &d, int batch_size){
    auto [board, val]=d;
    double delta=val-evalptn(board);
    delta/=8 * batch_size;
    delta*=0.2; // learning rate

    Board b_cur=board; adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
    b_cur=board; b_cur.flip_v(); adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
    b_cur=board; b_cur.rotate_l(); adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
    b_cur=board; b_cur.rotate_r(); adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
}

void train(){
    int train_steps=50000;
    int batch_size=512;
    int train_id=0;
    testError();
    inc(i, train_steps){
        double sse=0, sae=0;
        inc(j, batch_size){
            sse+=SE(data[train_id]);
            sae+=AE(data[train_id]);
            accuGrad(data[train_id], batch_size);
            train_id++;
            if (train_id==(int)data.size())
                train_id=0; 
        }
        if ((i+1)%200==0)
            printf("train step %d   sse: %f, sae: %f\n", i+1, sse/batch_size, sae/batch_size);
        updateArgs();
        if ((i+1)%1000==0) testError();
    }
    printf("wb:%f wmob:%f wodd:%f",ptn.wb,ptn.wmob,ptn.wodd);
}

int main(){
    initPtnConfig();
    folder="rawdata1";
    phase=1;
    //cout<< "folder: "; cin>>folder;
    //cout<< "phase: "; cin>>phase;
    loadData();
    cout<<data.size()<<" boards loaded\n";
    std::shuffle(data.begin(),data.end(),rng);
    int test_size=data.size()/20;
    data_test.insert(data_test.end(), data.end()-test_size, data.end());
    data.erase(data.end()-test_size, data.end());

    train();
    return 0;
}
