#include "search.h"
#include "evalptn.h"
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <setjmp.h>
#include <chrono>
#include <thread>
#include <mutex>

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
    u8 pv, depth;
    //0 nothing, 1:upper bound, 2:lower bound, 3:pv
    u8 type;
    Val val;
}translation_table[1<<20];

Val search_end2(CBoard cboard){
#ifdef DEBUGTREE
    constexpr int depth=2, alpha=-INF, beta=INF;
    DEBUGTREE_WARPPER_BEGIN
#endif
    searchstat.leafcnt++;
    u64 emptys=cboard.emptys();
    int p1=ctz(emptys); emptys=blsr(emptys);
    int p2=ctz(emptys);
    Board board=cboard;
    int v1=-INF, v2;
    if (board.makemove(p1)){
        if (board.makemove<1>(p2) || board.makemove(p2)) v1=board.cnt0()*2;
        else v1=board.cnt0()*2+1;
        board=cboard; // rewind
    }
    if (board.makemove(p2)){
        if (board.makemove<1>(p1) || board.makemove(p1)) v2=board.cnt0()*2;
        else v2=board.cnt0()*2+1;
        return std::max(v1,v2)-64;
    }
    if (likely(v1!=-INF)) return v1-64;
    //pass
    v1=INF;
    if (board.makemove<1>(p1)){
        if (board.makemove(p2) || board.makemove<1>(p2)) v1=board.cnt0()*2;
        else v1=board.cnt0()*2+1;
        board=cboard; // rewind
    }
    if (board.makemove<1>(p2)){
        if (board.makemove(p1) || board.makemove<1>(p1)) v2=board.cnt0()*2;
        else v2=board.cnt0()*2+1;
        return std::min(v1,v2)-64;
    }
    if (v1!=INF) return v1-64;
    return board.cnt0()*2-62;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

Val search_exact(int depth, CBoard cboard, Val alpha, Val beta, bool pass){
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove();
    if (!move){
        if (pass) {searchstat.leafcnt++; return eval_end(cboard);} 
        return -search_exact(depth, cboard.cswap_r(), -beta, -alpha, 1);
    }
    if (depth==5) return search_end<5>(cboard, alpha, beta, 0);
    Val val=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.cmakemove_r(p);
        val=max(val, -search_exact(depth-1, board, -beta, -alpha, 0));
        if (val>=beta) return val;
        alpha=max(alpha, val);
    }
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

Val evalMidGame(CBoard cboard){return evalPtn(cboard)/256.0;}

// special for depth == 0 or depth == 1
Val search_normal1(int depth, CBoard cboard, Val alpha, Val beta, bool pass=0){
    u64 move=cboard.genmove();
    if (depth==0){
        searchstat.leafcnt++;
        if (unlikely(!move)){
            if (unlikely(!cboard.genmove<1>())) return eval_end(cboard);
            return -evalMidGame(cboard.cswap_r());
        }
        return evalMidGame(cboard);
    }
    if (unlikely(!move)){
        if (unlikely(pass)) {searchstat.leafcnt++; return eval_end(cboard);}
        return -search_normal1(depth, cboard.cswap_r(), -beta, -alpha, 1);
    }
    Val val=-INF;
    for (auto p:u64iter(move)){
        searchstat.leafcnt++;
        Board board=cboard.cmakemove_r(p);
        Val ret;
        if (unlikely(!board.genmove())){
            if (unlikely(!board.genmove<1>())) ret=-eval_end(board);
            else ret=evalMidGame(board.cswap_r());
        }
        else ret=-evalMidGame(board);
        val=max(val,ret);
        if (val>=beta) return val;
    }
    return val;
}
int hash_hitc;
bool btimeout, btimeless;
jmp_buf jtimeout_exit;
Val search_normal(int depth, CBoard cboard, Val alpha, Val beta, bool pass){
    if (unlikely(btimeout)) longjmp(jtimeout_exit, 1);
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove();
    if (!move){
        if (pass) return eval_end(cboard); 
        return -search_normal(depth, cboard.cswap_r(), -beta, -alpha, 1);
    }
    int remain=popcnt(cboard.emptys());
    if (remain==6) return search_end<6>(cboard, alpha, beta, 0);

    if (depth==1)
        return search_normal1(1, cboard, alpha, beta);
    int firstp=ctz(move); //by default
#if 1
    bool hash_hit = false;
    auto ttnode=translation_table + cboard.hash()%(1<<20);
    if (ttnode->hash==cboard.hash()){
        firstp = ttnode->pv;
        hash_hit = true;
        hash_hitc++;
        if (ttnode->depth==depth){
            Val val=ttnode->val;
            if (ttnode->type>=2) alpha=max(alpha, val);
            if (alpha>=beta) return alpha;
        }
    }
#endif
    // presearch
    if (depth>3){
        auto t_move=move;
        btr(t_move, firstp);
        Val t_alpha;
        if (depth>5) t_alpha=-search_normal(depth-4, cboard.cmakemove_r(firstp), -INF, INF);
        else t_alpha=-search_normal1(depth-4, cboard.cmakemove_r(firstp), -INF, INF);
        for (auto p:u64iter(move)){
            Val ret;
            if (depth>5) ret=-search_normal(depth-4, cboard.cmakemove_r(p), -INF, -t_alpha);
            else ret=-search_normal1(depth-4, cboard.cmakemove_r(p), -INF, -t_alpha);
            if (ret>t_alpha){
                t_alpha=ret;
                firstp=p;
            }
        }
    }
    int pv = firstp;
    btr(move, firstp);
    Val val=-search_normal(depth-1, cboard.cmakemove_r(firstp), -beta, -alpha);
    if (val>alpha){
        if (val>=beta) goto BETA_CUT;
        alpha=val;
    }
    for (auto p:u64iter(move)){
        Board board=cboard.cmakemove_r(p);
        Val ret=-search_normal(depth-1, board, -alpha-0.01, -alpha); // zwsearch
        if (ret>alpha+0.005 && ret<beta){
            ret=-search_normal(depth-1, board, -beta, -alpha);
            if (ret>alpha) alpha=ret;
        }
        if (ret>val){
            val=ret; pv=p;
            if (val>=beta) goto BETA_CUT;
        }
    }
BETA_CUT:
#if 1
    if (!hash_hit){ //hash update
        ttnode->hash=cboard.hash();
        ttnode->depth=depth;
    }
    ttnode->pv=pv;
    ttnode->val=val;
    if (val<=alpha) ttnode->type=1;
    else if (val>=beta) ttnode->type=2;
    else ttnode->type=3;
#endif
    return val;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_END
#endif
}

int random_choice(CBoard board){
    std::vector<int> pos;
    for (auto p: u64iter(board.genmove()))
        pos.push_back(p);
    assertprintf(pos.size(), "nowhere to play\n");
    return pos[rand()%pos.size()];
}

std::ostringstream debugout;

void search_exact_root(CBoard cboard, Val delta){
    searchstat.leafcnt=0;
    searchstat.timing();
    searchstat.depth=popcnt(cboard.emptys());
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,searchstat.depth, cboard, -INF, INF);
#endif
    u64 move=cboard.genmove();
    assertprintf(move, "nowhere to play\n");
    std::vector<PosVal> result;
    Val alpha=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.cmakemove_r(p);
        Val ret;
        ret=-search_exact(searchstat.depth-1, board, -INF, -alpha+delta+0.01, 0);
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

float search_delta=0;

int search_root(int depth, CBoard cboard, int suggestp){
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,depth, cboard, -INF, INF);
#endif
    hash_hitc=0;
    searchstat.leafcnt=0;
    searchstat.timing();
    searchstat.depth=depth;
    u64 move=cboard.genmove();
    assertprintf(move, "nowhere to play\n");
    std::vector<PosVal> result;
    Val alpha=-INF;
    if (suggestp!=-1){
        btr(move, suggestp);
        alpha = -search_normal(depth-1, cboard.cmakemove_r(suggestp), 
            -INF, -alpha+search_delta+0.01, 0);
        result.emplace_back(suggestp, alpha);
    }
    for (auto p:u64iter(move)){
        Val ret=-search_normal(depth-1, cboard.cmakemove_r(p), -INF, -alpha+search_delta+0.01, 0);
        if (ret>alpha) alpha=ret, suggestp=p;
        if (ret>=alpha-search_delta) result.emplace_back(p, ret);
    }
    debugout<<hash_hitc<<' ';
#ifdef DEBUGTREE
    if (debug_tree) debug_tree->step_out(alpha);
#endif
    result.erase(
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-search_delta;}),
        result.end());
    searchstat.timing();
    searchstat.pv.clear(); 
    searchstat.pv.assign(result.begin(), result.end());
    return suggestp;
}

void search_id(CBoard board, int maxd){
    if (setjmp(jtimeout_exit)) return;
    int p=-1;
    for (int depth=4;depth<=maxd;depth++){
        p = search_root(depth, board, p);
        debugout<<searchstat.str()<<'\n';
        if (btimeless) return;
    }
}

int think_choice(CBoard board){
    debugout.str("");
    if (popcnt(board.emptys())<=12)
        search_exact_root(board, 0);
    else
        search_id(board, 8);
    return searchstat.pv[rand()%searchstat.pv.size()].first;
}

int think_choice_td(CBoard board){
    search_delta=1.0;
    debugout.str("");
    if (popcnt(board.emptys())<=12)
        search_exact_root(board, 0);
    else{
        btimeless=btimeout=false;
        std::timed_mutex tmux;
        clock_t t0=clock();
        std::thread thd([&]{
            tmux.lock();
            search_id(board, popcnt(board.emptys()));
            tmux.unlock();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(330));
        btimeless=true;
        if (tmux.try_lock_for(std::chrono::milliseconds(580))) tmux.unlock();
        btimeout=true;
        thd.join();
        debugout<<"final tl:"<<clock()-t0<<'\n';
    }
    return searchstat.pv[rand()%searchstat.pv.size()].first;
}
