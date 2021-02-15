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
        if (flag) o<<","; else flag=1;
        o<<'<'<<it.first/8<<','<<it.first%8<<">:";
        o<<std::fixed<<std::setprecision(2)<<it.second;
    }
    o<<"]";
    return o.str();
}

struct TranslationTableNode{
    u64 hash;
    u8 col, pv;
    //0 nothing, 1:upper bound, 2:lower bound, 3:pv
    u8 type, depth;
    Val val;
}translation_table[1<<20];

Val search_end2(CBoard cboard, int col){
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

Val search_exact(int depth, CBoard cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove(col);
    if (!move){
        if (pass) {searchstat.leafcnt++; return eval_end(cboard, col);} 
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

Val evalMidGame(CBoard cboard, int col){
    return evalPtn(cboard, col)/256.0;
}

Val search_normal1(int depth, CBoard cboard, int col, Val alpha, Val beta, bool pass=0){
    u64 move=cboard.genmove(col);
    if (!move){
        if (pass) {searchstat.leafcnt++; return eval_end(cboard, col);}
        return -search_normal1(depth, cboard, !col, -beta, -alpha, 1);
    }
    if (depth==0){
        searchstat.leafcnt++;
        return evalMidGame(cboard, col);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        val=max(val, -search_normal1(depth-1, cboard.makemove_r(p, col), !col, -beta, -alpha));
        if (val>=beta) return val;
    }
    return val;
}
int hash_hitc;
Val search_normal(int depth, CBoard cboard, int col, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove(col);
    if (!move){
        if (pass) return eval_end(cboard, col); 
        return -search_normal(depth, cboard, !col, -beta, -alpha, 1);
    }
    int remain=popcnt(cboard.emptys());
    if (remain==6) return search_end<6>(cboard, col, alpha, beta, 0);
    if (depth==1)
        return search_normal1(1, cboard, col, alpha, beta);
    int firstp=ctz(move); //by default
#if 1
    bool hash_hit = false;
    auto ttnode=translation_table + cboard.hash()%(1<<20);
    if (ttnode->hash==cboard.hash() && ttnode->col==col){
        firstp = ttnode->pv;
        hash_hit = true;
        hash_hitc++;
    }
#endif
    int pv = firstp;
    btr(move, firstp);
    Val val=-search_normal(depth-1, cboard.makemove_r(firstp, col), !col, -beta, -alpha);
    if (val>alpha){
        if (val>=beta) goto BETA_CUT;
        alpha=val;
    }
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        Val ret=-search_normal(depth-1, board, !col, -alpha-0.01, -alpha); // zwsearch
        if (ret>alpha+0.005 && ret<beta){
            ret=-search_normal(depth-1, board, !col, -beta, -alpha);
            if (ret>alpha) alpha=ret;
        }
        if (ret>val){
            val=ret; pv=p;
            if (val>=beta) goto BETA_CUT;
        }
    }
BETA_CUT:
#if 1
    if (searchstat.depth-depth<4){
    if (!hash_hit){ //hash update
        ttnode->hash=cboard.hash();
        ttnode->col=col;
        ttnode->depth=depth;
    }
    ttnode->pv=pv;
    }
#endif
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

int random_choice(CBoard board, int col){
    std::vector<int> pos;
    for (auto p: u64iter(board.genmove(col)))
        pos.push_back(p);
    assertprintf(pos.size(), "nowhere to play\n");
    return pos[rand()%pos.size()];
}

void search_exact_root(CBoard cboard, int col, Val delta){
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,depth, cboard, col, -INF, INF);
#endif
    searchstat.leafcnt=0;
    searchstat.timing();
    searchstat.depth=popcnt(cboard.emptys());
    u64 move=cboard.genmove(col);
    assertprintf(move, "nowhere to play\n");
    std::vector<PosVal> result;
    Val alpha=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        Val ret;
        ret=-search_exact(searchstat.depth-1, board, !col, -INF, -alpha+delta+0.01, 0);
        alpha=max(alpha, ret);
        if (ret>=alpha-delta) result.emplace_back(p, ret);
    }
#ifdef DEBUGTREE
    if (debug_tree) debug_tree->step_out(alpha);
#endif
    result.erase(
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-delta;}),
        result.end());
    searchstat.timing();
    searchstat.pv.clear(); 
    searchstat.pv.assign(result.begin(), result.end());
    debugout<<searchstat.str()<<'\n';
}
void search_root(int depth, CBoard cboard, int col, Val delta){
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,depth, cboard, col, -INF, INF);
#endif
    hash_hitc=0;
    searchstat.leafcnt=0;
    searchstat.timing();
    searchstat.depth=depth;
    u64 move=cboard.genmove(col);
    assertprintf(move, "nowhere to play\n");
    std::vector<PosVal> result;
    Val alpha=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.makemove_r(p, col);
        Val ret;
        ret=-search_normal(depth-1, board, !col, -INF, -alpha+delta+0.01, 0);
        alpha=max(alpha, ret);
        if (ret>=alpha-delta) result.emplace_back(p, ret);
    }
    debugout<<hash_hitc<<' ';
#ifdef DEBUGTREE
    if (debug_tree) debug_tree->step_out(alpha);
#endif
    result.erase(
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-delta;}),
        result.end());
    searchstat.timing();
    searchstat.pv.clear(); 
    searchstat.pv.assign(result.begin(), result.end());
}

std::ostringstream debugout;

int think_choice(CBoard board, int col){
    debugout.str("");
    if (popcnt(board.emptys())<=10){
        search_exact_root(board, col, 0);
        debugout<<searchstat.str()<<'\n';
    }
    else{
        for (int depth=4;depth<=8;depth++){
            search_root(depth, board, col, 0);
            debugout<<searchstat.str()<<'\n';
        }
    }
    return searchstat.pv[rand()%searchstat.pv.size()].first;
}
