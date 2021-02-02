#include "search.h"
#include "evahptn.h"

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

Val search_normal(int depth, const Board &cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    if (depth==max_depth){
        return evalPtn(cboard, col);
    }
    u64 move=cboard.genmove(col);
    if (unlikely(move==0)){
        if (pass) return eval_end(cboard, col); 
        return -search_exact(depth+1, cboard, !col, -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        val=max(val, -search_exact(depth+1, board, !col, -beta, -alpha, 0));
        if (val>=beta) return val;
        alpha=max(alpha, val);
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

int randomPolicy(const Board &board, int col){
    vector<int> pos;
    for (auto p: u64iter(board.genmove()))
        pos.push_back(pos);
    assert(pos.size(),"nowhere to go!\n");
    return pos[rand()%pos.size()];
}
