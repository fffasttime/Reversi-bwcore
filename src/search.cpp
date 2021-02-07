#include "search.h"
#include "evalptn.h"
#include <vector>
#include <algorithm>

Val search_end2(const Board &board_old, int col){
    u64 emptys=board_old.emptys();
    int p1=ctz(emptys); btr(emptys, p1);
    int p2=ctz(emptys);
    Board board;
    board=board_old;
    int v1=-INF, v2=-INF;
    if (board.makemove(p1, col)){
        if (board.makemove(p2, !col)) v1=board.cnt(col)*2;
        else if (board.makemove(p2, col)) v1=board.cnt(col)*2;
        else v1=board.cnt(col)*2+1;
    }
    if (board.makemove(p2, col)){
        if (board.makemove(p1, !col)) v2=board.cnt(col)*2;
        else if (board.makemove(p1, col)) v2=board.cnt(col)*2;
        else v2=board.cnt(col)*2+1;
    }
    int r=max(v1,v2);
    if (r!=-INF) return r-64;
    //pass
    if (board.makemove(p1, !col)){
        if (board.makemove(p2, col)) v1=board.cnt(col)*2;
        else if (board.makemove(p2, !col)) v1=board.cnt(col)*2;
        else v1=board.cnt(col)*2+1;
    }
    if (board.makemove(p2, !col)){
        if (board.makemove(p1, col)) v2=board.cnt(col)*2;
        else if (board.makemove(p1, !col)) v2=board.cnt(col)*2;
        else v2=board.cnt(col)*2+1;
    }
    r=max(v1,v2);
    if (r!=-INF) return r-64;
    return board.cnt(col)*2-62;
}

template<>
Val search_end<3>(const Board &cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    constexpr int depth=3;
    DEBUGTREE_WARPPER_BEGIN
#endif
    constexpr int depth=3;
    u64 move=cboard.genmove(col);
    if (unlikely(move==0)){
        if (pass) return eval_end(cboard, col); 
        return -search_end<depth>(cboard, !col, -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        val=max(val, -search_end2(board, !col));
        if (val>=beta) return val;
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

Val search_exact(int depth, const Board &cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    if (depth==5) return search_end<5>(cboard, col, alpha, beta, 1);
    u64 move=cboard.genmove(col);
    if (unlikely(move==0)){
        if (pass) return eval_end(cboard, col); 
        return -search_exact(depth-1, cboard, !col, -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        val=max(val, -search_exact(depth-1, board, !col, -beta, -alpha, 0));
        if (val>=beta) return val;
        alpha=max(alpha, val);
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

Val evalMidGame(const Board &cboard, int col){
    return evalPtn(cboard, col)/256.0;
}

Val search_normal(int depth, const Board &cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    if (depth==0){
        return evalMidGame(cboard, col);
    }
    u64 move=cboard.genmove(col);
    if (unlikely(move==0)){
        if (pass) return eval_end(cboard, col); 
        return -search_normal(depth-1, cboard, !col, -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        val=max(val, -search_normal(depth-1, board, !col, -beta, -alpha, 0));
        if (val>=beta) return val;
        alpha=max(alpha, val);
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

int random_choice(const Board &board, int col){
    std::vector<int> pos;
    for (auto p: u64iter(board.genmove(col)))
        pos.push_back(p);
    assert(pos.size(), "nowhere to play\n");
    return pos[rand()%pos.size()];
}

int search_root(int depth, const Board &cboard, int col, Val delta){
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,depth, cboard, col, -INF, INF);
#endif
    u64 move=cboard.genmove(col);
    assert(move, "nowhere to play\n");
    std::vector<std::pair<int, Val>> result;
    Val alpha=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        Val val=-search_normal(depth-1, board, !col, -INF, -alpha+delta+0.01, 0);
        alpha=max(alpha, val);
        if (val>=alpha-delta) result.emplace_back(p, val);
    }
#ifdef DEBUGTREE
    if (debug_tree) debug_tree->step_out(alpha);
#endif
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-delta;});
    return result[rand()%result.size()].first;
}

int think_choice(const Board &board, int col){
    return search_root(5, board, col, 0);
}
