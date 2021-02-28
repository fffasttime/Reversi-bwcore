#pragma once

#include "board.h"
#include "time.h"
#include <algorithm>
#include <vector>
#include <sstream>
using std::max;

typedef float Val;
constexpr int INF=256;
typedef std::pair<int, Val> PosVal;

#ifdef DEBUGTREE
    #include "debugtree.h"
#endif

Val search_end2(CBoard board);
inline Val eval_end(CBoard board){return board.cnt0()-popcnt(board.b[1]);}

template<int depth>
Val search_end(CBoard cboard, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove();
    if (!move){
        if (pass) return eval_end(cboard);
        return -search_end<depth>(cboard.cswap_r(), -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        if constexpr (depth==3) val=max(val, -search_end2(cboard.cmakemove_r(p)));
        else val=max(val, -search_end<depth-1>(cboard.cmakemove_r(p), -beta, -alpha, 0));
        if (val>=beta) return val;
        if (val>alpha) alpha=val;
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

struct SearchStat{
    int depth, leafcnt;
    int t_start, tl; 
    std::vector<PosVal> pv;
    Val maxv;
    void timing(){
        tl=clock()-t_start;
        t_start=clock();
    }
    void reset(int _depth){leafcnt=0; depth=_depth; t_start=clock(); maxv=-INF;}
    std::string str();
};

int search_root(int depth, CBoard cboard, int suggestp=-1);
Val search_normal(int depth, CBoard cboard, Val alpha, Val beta, bool pass=0);

constexpr int MPC_MAXD=14;

#ifndef ONLINE
Val search_exact(int depth, CBoard cboard, Val alpha, Val beta, bool pass=0);
int random_choice(CBoard board);
int think_choice(CBoard board);
int think_choice_td(CBoard board);
void loadPCData();

extern SearchStat searchstat;
extern SearchStat searchstat_sum;
extern std::ostringstream debugout;

extern Val search_delta;
extern int think_maxd; //think_choice() maxd midgame
extern int think_checktime, think_maxtime;

#endif //ONLINE
