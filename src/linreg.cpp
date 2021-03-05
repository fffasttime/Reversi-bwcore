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
#include <cstdio>
#include <ctime>
#include <cmath>
using std::string, std::cin, std::cout;

//#define LOGISTIC

typedef std::pair<Board,double> DataType;
std::vector<DataType> data, data_test;
string folder;
int phase;

// type of data is double when training
struct CoeffPack{
    double e1[pow3[10]], c52[pow3[10]], c33[pow3[9]], //by pow4to3
        e2[1<<16], e3[1<<16], e4[1<<16], k8[1<<16], k7[1<<14], k6[1<<12], k5[1<<10], k4[1<<8], //directly
        wb, wodd, wmob, wcinner[65], wcedge, wedgeodd, ccor[1<<8], cx22[1<<8];
}ptn, ptngrad;
const int p_begins[] = {2,5, 8,11,14,18,22,27,32,38,44};
const int p_ends[]   = {4,7,10,13,17,21,26,31,37,43,50};

void writePtnData(){
    FILE *out=fopen((string("data/")+folder+"/coeff"+std::to_string(p_begins[phase])
    +"_"+std::to_string(p_ends[phase])+".bin").c_str(), "wb");
    auto &p=ptn;
    double largest=0;
    auto ws=[&](double s){
        if (s>largest) largest=s;
        auto x=short(s*256+0.5);
        fwrite(&x, 2, 1, out);
    };
    ws(p.wb);
    ws(p.wodd);
    ws(p.wmob);
    ws(p.wcedge);
    ws(p.wedgeodd);
    inc(j,65) ws(p.wcinner[j]);
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
    inc(j,pow3[4])  ws(p.ccor[pow3to4(4,j)]);
    inc(j,pow3[4])  ws(p.cx22[pow3to4(4,j)]);
    printf("largest coeff=%f\n", largest);
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
double sigmoid(double x){
    return 1/(1+exp(-x));
}
double evalptn(const Board &board){
    using namespace bitptn;
    int empty_cnt = popcnt(board.emptys());
    auto &p=ptn;

    Board b_id=board;

    double score=p.wb; //constant
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
#ifdef LOGISTIC
    return sigmoid(score);
#endif
    return score;
}
void adjustptn(const Board &board, double grad){
    using namespace bitptn;
    int empty_cnt = popcnt(board.emptys());
    auto &p=ptngrad;
    Board b_id=board;

    p.wb+=grad/2; //constant
    int cmob = popcnt(b_id.genmove());
    int codd = empty_cnt%2;
    p.wodd+=grad*codd/2;
    const int mobdiv=std::min(p_ends[phase],15)*2;
    p.wmob+=grad*cmob/mobdiv;
    p.wcinner[popcnt(b_id.b[0]&pinner)-popcnt(b_id.b[1]&pinner)+32]+=grad;
    int cedge=popcnt(b_id.b[0]&pedge)-popcnt(b_id.b[1]&pedge);
    p.wcedge+=grad*cedge/40;
    p.wedgeodd+=grad*(cedge%2)/40;

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

    //OP_EXT(b_id, ccor, 4) S(ccor[x])/2;
    //OP_EXT(b_id, cx22, 4) S(cx22[x])/2;

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
double CrossEntropy(DataType &d){
    double y=d.second>0.5, fx=evalptn(d.first);
    return y*log(fx)+(1-y)*log(1-fx);
}
double PredictAcc(DataType &d){
    double y=d.second>0.5, fx=evalptn(d.first);
    return ((y>0.5)^(fx<0.5))*100;
}
std::mt19937 rng((std::random_device())());
auto testError(){
#ifndef LOGISTIC
    double sse=0,sae=0, mae=0;
    for (auto &d:data_test){
        sse+=SE(d)/data_test.size();
        sae+=AE(d)/data_test.size();
        mae=std::max(mae,AE(d));
    }
    return std::make_tuple(mae, sse, sae);
#else
    double sce=0, acc=0;
    for (auto &d:data_test){
        sce+=CrossEntropy(d)/data_test.size();
        acc+=PredictAcc(d)/data_test.size();
    }
    return std::make_tuple(sce, acc);
#endif
}
void updateArgs(){
    for (size_t i=0;i<sizeof(CoeffPack)/sizeof(double);i++)
        ((double *)(&ptn))[i]+=((double *)(&ptngrad))[i];
    memset(&ptngrad,0,sizeof(CoeffPack));
}
int sgn(double x){
    if (x<-0.5) return -1;
    return x>0.5;
}
void accuGrad(DataType &d, int batch_size, double lr){
    auto [board, val]=d;
    auto fx=evalptn(board);
    double delta=val-fx;

    #ifdef LOGISTIC // logistic regression
    delta=(val>0.5)-evalptn(board);
    #endif 
    delta/=8 * batch_size;
    delta*=lr; // learning rate

    Board b_cur=board; adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
    b_cur=board; b_cur.flip_v(); adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
    b_cur=board; b_cur.rotate_l(); adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
    b_cur=board; b_cur.rotate_r(); adjustptn(b_cur, delta);
    b_cur.flip_h(); adjustptn(b_cur, delta);
}

FILE *flog;

void train(){
    int batch_size=512;
    int train_id=0;
    double lr;
    const int train_steps=50000;
    const int lr_decay_st=20000;
    double lr_max=0.2, lr_min=0.03;
    cout<<"lr_max(default 0.2): "; cin>>lr_max;
    fprintf(flog, "batch=%d lr_decay_st=%d lr_max=%f lr_min=%f\n", train_steps, lr_decay_st, lr_max, lr_min);
    inc(i, train_steps){
        if (i<1000) lr=lr_min; //warming up
        else if (i<lr_decay_st) lr=lr_max;
        else lr-=(lr_max-lr_min)/(train_steps-lr_decay_st);
        double sse=0, sae=0, acc=0;
        inc(j, batch_size){
            #ifndef LOGISTIC
            sse+=SE(data[train_id])/batch_size;
            #else
            sse+=CrossEntropy(data[train_id])/batch_size;
            #endif
            sae+=AE(data[train_id])/batch_size;
            acc+=PredictAcc(data[train_id])/batch_size;
            accuGrad(data[train_id], batch_size, lr);
            train_id++;
            if (train_id==(int)data.size()){
                train_id=0; 
                std::shuffle(data.begin(),data.end(),rng);
            }
        }
        if ((i+1)%200==0)
        #ifndef LOGISTIC
            printf("train step %d  acc=%.2f sse=%.3f sae=%.3f\n", i+1, acc, sse, sae);
        #else
            printf("train step %d  sce=%f acc=%f\n", i+1, sse, acc);
        #endif
        updateArgs();

        if ((i+1)%1000==0) {
        #ifndef LOGISTIC
            auto [mae,sse,sae] = testError();
            printf("TEST mae=%.2f sse=%.3f sae=%.3f\n", mae, sse, sae);
        #else
            auto [sce,acc] = testError();
            printf("TEST sce=%f acc=%f\n", sce, acc);
        #endif
        }
    }
}

void analyze(){
    printf("wb=%f wmob=%f wodd=%f\n",ptn.wb,ptn.wmob,ptn.wodd);
    fprintf(flog, "wb=%f wmob=%f wodd=%f\n",ptn.wb,ptn.wmob,ptn.wodd);

#ifndef LOGISTIC
    double tsse=0, tsae=0, tacc=0;
    for (auto &d: data){
        tsse+=SE(d)/data.size();
        tsae+=AE(d)/data.size();
        tacc+=PredictAcc(d)/data.size();
    }
    printf("train acc=%f sse=%f sae=%f\n", tacc, tsse, tsae);
    fprintf(flog, "train acc=%f sse=%f sae=%f\n", tacc, tsse, tsae);
    auto [mae,sse,sae] = testError();
    fprintf(flog, "TEST mae=%f sse=%f sae=%f\n", mae, sse, sae);
#else
    double tsce=0, tacc=0;
    for (auto &d: data){
        tsce+=CrossEntropy(d)/data.size();
        tacc+=PredictAcc(d)/data.size();
    }
    printf("train acc=%f sce=%f\n", tacc, tsce);
    fprintf(flog, "train acc=%f sce=%f\n", tacc, tsce);
    auto [sce,acc] = testError();
    fprintf(flog, "TEST sce=%f acc=%f\n", sce, acc);
#endif

#ifndef LOGISTIC
    // errcnt stat
    int _errcnt[160];
    int *errcnt=_errcnt+80;
    memset(_errcnt, 0, sizeof _errcnt);
    for (auto &d: data_test)
        errcnt[(int)(evalptn(d.first)-d.second+0.5)]++;
    fputs("error count:\n|", flog);
    for (int i=-35;i<=35;i++) fprintf(flog, "%*d|", std::max(3, int(log10(errcnt[i])+1)), i);
    fputs("\n|", flog);
    for (int i=-35;i<=35;i++) fprintf(flog, "%3d|", errcnt[i]);
    fputs("\n", flog);

    // predict acc
    double acc=0;
    for (auto &d:data_test){
        double v=evalptn(d.first);
        if ((v>0.5) ^ (d.second<0.5)) acc+=100;
    }
    printf("predict acc: %.2f%%\n", acc/data_test.size());
    fprintf(flog, "predict acc: %.2f%%\n", acc/data_test.size());
#endif

#ifndef LOGISTIC
    // outer point
    std::vector<std::pair<double, int>> temp;
    int maxf_id=0; double maxf_v=0;
    int _cnt=0;
    for (auto &d:data_test){
        temp.emplace_back(AE(d), _cnt);
        double val=evalptn(d.first);
        if (fabs(val)>maxf_v) maxf_v=fabs(val), maxf_id=_cnt;
        _cnt++;
    }
    printf("maxv r=%.2f t=%.2f board %s\n",data_test[maxf_id].second, 
        evalptn(data_test[maxf_id].first), data_test[maxf_id].first.repr().c_str());
    std::sort(temp.begin(), temp.end(), std::greater<std::pair<double,int>>());
    printf("outer point:\n");
    for (int i=0;i<5;i++){
        int id=temp[i].second;
        auto &[board, val]=data_test[id];
        printf("r=%.2f t=%.2f  board %s\n", val, evalptn(board), board.repr().c_str());
    }
#endif
}

void train_op(){
    cout<< "phase: "; cin>>phase;
    loadData();
    flog=fopen((string("data/")+folder+"/trainlog.log").c_str(),"a");
    fputs("+ pcinner pcedge ccor cx22\n", flog);
    fprintf(flog, "phase: %d (%d_%d)  ", phase, p_begins[phase], p_ends[phase]);
    time_t t=time(0);
    fprintf(flog, " time: %s", ctime(&t));
    cout<<data.size()<<" boards loaded\n";
    std::shuffle(data.begin(),data.end(),rng);
    int test_size=data.size()/20;
    fprintf(flog, "%d board (%d train, %d test)\n", (int)data.size(), (int)data.size()-test_size, test_size);
    data_test.insert(data_test.end(), data.end()-test_size, data.end());
    data.erase(data.end()-test_size, data.end());

    train();
    analyze();
    writePtnData();
    fputs("\n", flog);
    fclose(flog);
}

void mergedata(){
    const short format_version=1;
    const char *data_desc="bwcore1.5 linreg, + wcedge wcinner ccor cx22\n\
wb|wodd|wmob|wcdege|wedgeodd|wcinner(65)|e1|c52|c33|e2|e3|e4|k8|k7|k6|k5|k4|ccor|cx22\n";
    const short plen[]=
    {0,0,0,0,0,65,10,10,9,8,8,8,8,7,6,5,4,4,4};
    cout<<"folder: "; cin>>folder;

    int datapack_len=0;
    for (auto x:plen) if (x<=10) datapack_len+=pow3[x]; else datapack_len+=x;
    cout<<"datapack len: "<<datapack_len<<'\n';

    int merge_begin, merge_end;
    cout<<"merge_begin(default 1): "; cin>>merge_begin;
    cout<<"merge_end(default 10): "; cin>>merge_end;

    FILE *in[11], *out;
    for (int i=merge_begin; i<=merge_end;i++){
        string filename=string("data/")+folder+"/coeff"+
            std::to_string(p_begins[i])+"_"+std::to_string(p_ends[i])+".bin";
        in[i]=fopen(filename.c_str(),"rb");
        cout<<"open phase "<<i<<": "<<filename<<" ";
        if(!in[i]){
            cout<<"FAIL\n";
            exit(1);
        }
        else cout<<"ok\n";
    }
    string filename=string("data/")+folder+"/reversicoeff.bin";
    out=fopen(filename.c_str(), "wb");
    cout<<"save to "<<filename<<"\n";

    short checksum;
    auto ws=[&](short x){fwrite(&x, 2, 1, out); checksum^=x;};
    
    ws(format_version);
	ws(merge_end-merge_begin+1); // part count
	ws(sizeof(plen)/sizeof(short)); 
	for (short x:plen) ws(x);

    checksum=0;
    for (int i=merge_begin;i<=merge_end;i++){
        short x; readShort(in[i], x);
        for (int j=0;j<datapack_len;j++){
            ws(x);
            if (feof(in[i])) {cout<<"Fail: detected early EOF when read phase "<<i
                <<", pointer at "<<j<<'\n'; exit(1);}
            readShort(in[i], x);
        }
        if (!feof(in[i])) {cout<<"Fail: expected EOF when read phase "<<i<<'\n'; exit(1);}
        fclose(in[i]);
    }
    ws(checksum);
    ws(strlen(data_desc)+1);
    
	fwrite(data_desc, strlen(data_desc)+1, 1, out);
    
	long long t=time(NULL);
	fwrite(&t, sizeof t, 1, out);
	printf("timestamp: %lld\n",t);
	
	fclose(out);
}

int main(int argc, char **argv){
    initPtnConfig();
    if (argc==1){
        folder="rawdata1";
        train_op();
    }
    else if (argc==2){
        if (string(argv[1])=="merge")
            mergedata();
        else{
            folder=argv[1];
            train_op();
        }
    }
    return 0;
}
