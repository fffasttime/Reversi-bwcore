#include "search.h"
#include "evalptn.h"
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

SearchStat searchstat;
std::string SearchStat::str(){
    std::ostringstream o;
    o<<"dep:"<<depth;
    o<<", "; o<<"cnt:"<<leafcnt;
    o<<", "; o<<"tl:"<<tl;
    o<<", pv:["; bool flag=0;
    for (auto it:pv){
        if (flag) o<<","; flag=1;
        o<<'<'<<it.first/8<<','<<it.first%8<<">:";
        o<<std::fixed<<std::setprecision(2)<<it.second;
    }
    o<<"]";
    return o.str();
}

Val search_end2(const Board &cboard, int col){
#ifdef DEBUGTREE
    constexpr int depth=2, alpha=-INF, beta=INF;
    DEBUGTREE_WARPPER_BEGIN
#endif
    searchstat.leafcnt++;
    u64 emptys=cboard.emptys();
    int p1=ctz(emptys); emptys=blsr(emptys);
    int p2=ctz(emptys);
    Board board=cboard;
    int v1=-INF, v2=-INF;
    if (board.makemove(p1, col)){
        if (board.makemove(p2, !col) || board.makemove(p2, col)) v1=board.cnt(col)*2;
        else v1=board.cnt(col)*2+1;
        board=cboard; // rewind
    }
    if (board.makemove(p2, col)){
        if (board.makemove(p1, !col) || board.makemove(p1, col)) v2=board.cnt(col)*2;
        else v2=board.cnt(col)*2+1;
        return std::max(v1,v2)-64;
    }
    if (v1!=-INF) return v1-64;
    //pass
    if (board.makemove(p1, !col)){
        if (board.makemove(p2, col) || board.makemove(p2, !col)) v1=board.cnt(col)*2;
        else v1=board.cnt(col)*2+1;
        board=cboard; // rewind
    }
    if (board.makemove(p2, !col)){
        if (board.makemove(p1, col) || board.makemove(p1, !col)) v2=board.cnt(col)*2;
        else v2=board.cnt(col)*2+1;
        return std::min(v1,v2)-64;
    }
    if (v1!=-INF) return v1-64;
    return board.cnt(col)*2-62;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

template<>
Val search_end<3>(const Board &cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    constexpr int depth=3;
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove(col);
    if (!move){
        if (pass) return eval_end(cboard, col); 
        return -search_end<3>(cboard, !col, -beta, -alpha, 1);
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
    u64 move=cboard.genmove(col);
    if (!move){
        if (pass) return eval_end(cboard, col); 
        return -search_exact(depth, cboard, !col, -beta, -alpha, 1);
    }
    if (depth==5) return search_end<5>(cboard, col, alpha, beta, 0);
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
    u64 move=cboard.genmove(col);
    if (!move){
        if (pass) return eval_end(cboard, col); 
        return -search_normal(depth, cboard, !col, -beta, -alpha, 1);
    }
    int remain=popcnt(cboard.emptys());
    if (remain==5) return search_end<5>(cboard, col, alpha, beta, 0);
    if (depth==0 && remain>5){
        searchstat.leafcnt++;
        return evalMidGame(cboard, col);
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
    assertprintf(pos.size(), "nowhere to play\n");
    return pos[rand()%pos.size()];
}

std::vector<PosVal> search_root(int depth, const Board &cboard, int col, Val delta){
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,depth, cboard, col, -INF, INF);
#endif
    searchstat.depth=depth;
    u64 move=cboard.genmove(col);
    assertprintf(move, "nowhere to play\n");
    std::vector<PosVal> result;
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
    result.erase(
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-delta;}),
        result.end());
    return result;
}

int think_choice(const Board &board, int col){
    searchstat.leafcnt=0;
    searchstat.timing();
    auto moves=search_root(5, board, col, 0);
    searchstat.timing();
    searchstat.pv.clear(); 
    searchstat.pv.assign(moves.begin(), moves.end());
    return moves[rand()%moves.size()].first;
}
