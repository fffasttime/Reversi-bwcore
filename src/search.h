#pragma once

#include "board.h"
#include <algorithm>
using std::max;

typedef float Val;
constexpr int INF=256;

#ifdef DEBUGTREE
    #include "debugtree.h"
#endif

Val search_end2(const Board &board, int col);
inline Val eval_end(const Board &board, int col){return board.b[col]-board.b[!col];}

template<int depth>
Val search_end(const Board &cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove(col);
    if (unlikely(move==0)){
        if (pass) return eval_end(cboard, col); 
        return -search_end<depth>(cboard, !col, -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        val=max(val, -search_end<depth-1>(board, !col, -beta, -alpha, 0));
        if (val>=beta) return val;
        if (val>alpha) alpha=val;
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

#ifndef ONLINE
Val search_exact(int depth, const Board &cboard, int col, Val alpha, Val beta, bool pass=0);
Val search_normal(int depth, const Board &cboard, int col, Val alpha, Val beta, bool pass=0);
int random_choice(const Board &board, int col);
int think_choice(const Board &board, int col);
#endif
