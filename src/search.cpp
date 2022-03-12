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
#include <fstream>
#include <cstring>

u64 debug_flag;
SearchStat searchstat, searchstat_sum;
std::string SearchStat::str(){
    std::ostringstream o;
    o<<"dep:"<<depth;
    o<<", "; o<<"cnt:"<<leafcnt;
    o<<", "; o<<"hc:"<<hashhit;
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
    Board board;
    u8 pv, depth;
    //0 nothing, 1:upper bound, 2:lower bound, 3:pv
    u8 type;
    Val alpha, beta;
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

Val PC_CONSTANT = 1.4f;
struct PC_Param{
    Val w, b, sigma;
}pc_param[64][MPC_MAXD+1];
constexpr int pc_depth[MPC_MAXD+1]={0,0,0,1,2,1,2,3,4,5,6,5,6,5,6};
void loadPCData(){
    std::ifstream fin("data/pc_coeff.txt");
    inc(i,64) inc(j,MPC_MAXD+1){
        PC_Param &pa=pc_param[i][j];
        fin>>pa.w>>pa.b>>pa.sigma;
    }
}
#ifdef GENDATA_PC
std::ofstream pc_data("data/pc_data.txt", std::ios::app);
int pc_statecnt[64][15];
#endif //GENDATA_PC
// #define USE_PC
Val probcut(int depth, CBoard cboard, Val alpha, Val beta){
    Val bound, ret;
    int cnt=popcnt(cboard.occupys());
#ifdef GENDATA_PC
    pc_statecnt[cnt][depth]++;
    if (pc_statecnt[cnt][depth]>3 && rand()%(pc_statecnt[cnt][depth]-3)) return (alpha+beta)/2;
    ret=search_normal(pc_depth[depth], cboard, -INF, INF);
    pc_data<<cboard.repr()<<' '<<ret<<' '<<pc_depth[depth]<<' '<<depth<<' '<<cnt<<'\n';
    return (alpha+beta)/2;
#endif //GENDATA_PC
    PC_Param &pa=pc_param[cnt][depth];
    if (pa.sigma>50) return (alpha+beta)/2; // no data
    bound=(PC_CONSTANT*pa.sigma+beta-pa.b)/pa.w;
    ret=search_normal(pc_depth[depth],cboard, bound-0.01, bound);
    if (ret>=bound) return beta;
    bound=(-PC_CONSTANT*pa.sigma+alpha-pa.b)/pa.w;
    ret=search_normal(pc_depth[depth],cboard, bound, bound+0.01);
    if (ret<bound) return alpha;
    return (alpha+beta)/2;
}

#define ENDSEARCH_BEGIN 6
#define USE_PC
bool btimeout, btimeless;
Val search_normal(int depth, CBoard cboard, Val alpha, Val beta, bool pass){
    if (unlikely(btimeout)) return 0;
#ifdef DEBUGTREE
    DEBUGTREE_WARPPER_BEGIN
#endif
    u64 move=cboard.genmove();
    if (!move){
        if (pass) return eval_end(cboard); 
        return -search_normal(depth, cboard.cswap_r(), -beta, -alpha, 1);
    }
    int remain=popcnt(cboard.emptys());
    if (remain==ENDSEARCH_BEGIN) return search_end<ENDSEARCH_BEGIN>(cboard, alpha, beta, 0);

    // fast leaf search
    if (depth==1) return search_normal1(1, cboard, alpha, beta);

    int firstp=ctz(move); // firstp by default
#if 1
    auto ttnode=translation_table + (cboard.hash()+depth)%(1<<20);
    Val alpha0=alpha, beta0=beta;
    if (ttnode->board==cboard){
        firstp = ttnode->pv;
        assertprintf(move>>firstp&1, "invalid hash pv %d", firstp);
        searchstat.hashhit++;
        if (ttnode->depth==depth){
            alpha=std::max(alpha, ttnode->alpha);
            beta=std::min(beta, ttnode->beta);
            if (alpha>=beta) return beta;
        }
    }
#endif //1
#ifdef USE_PC
    if (depth>=3 && depth<=MPC_MAXD && !(debug_flag&64)){
        Val val=probcut(depth, cboard, alpha, beta);
        if (val>=beta || val<=alpha) return val;
    }
#endif //USE_PC
    // presearch
    if (depth>3){
        auto t_move=move;
        btr(t_move, firstp);
        Val t_alpha;
        if (depth>5) t_alpha=-search_normal(depth-4, cboard.cmakemove_r(firstp), -INF, INF);
        else t_alpha=-search_normal1(depth-4, cboard.cmakemove_r(firstp), -INF, INF);
        for (auto p:u64iter(t_move)){
            Val ret;
            if (depth>5) ret=-search_normal(depth-4, cboard.cmakemove_r(p), -INF, -t_alpha);
            else ret=-search_normal1(depth-4, cboard.cmakemove_r(p), -INF, -t_alpha);
            if (ret>t_alpha) t_alpha=ret, firstp=p;
        }
    }
    int pv = firstp;
    assertprintf(move>>firstp&1, "invalid pv %d", firstp);
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
            ret=-search_normal(depth-1, board, -beta, -ret);
            if (ret>alpha) alpha=ret;
        }
        if (ret>val){
            val=ret; pv=p;
            if (val>=beta) goto BETA_CUT;
        }
    }
BETA_CUT:
    if (unlikely(btimeout)) return 0;
#if 1
    if (!(ttnode->board==cboard) || ttnode->depth<depth){ //hash update
        ttnode->board=cboard;
        ttnode->depth=depth;
        ttnode->alpha=-INF;
        ttnode->beta=INF;
        ttnode->pv=pv;
    }
    if (val>=alpha0+0.001) ttnode->alpha=std::max(ttnode->alpha, val);
    if (val<=beta0-0.001) ttnode->beta=std::min(ttnode->beta, val);
#endif //1
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
float search_delta=1.0;

void search_exact_root(CBoard cboard){
    searchstat.reset(popcnt(cboard.emptys()));
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
        Val ret=-search_exact(searchstat.depth-1, board, -INF, -alpha+search_delta+0.01, 0);
        alpha=max(alpha, ret);
        if (ret>=alpha-search_delta) result.emplace_back(p, ret);
    }
#ifdef DEBUGTREE
    if (debug_tree) debug_tree->step_out(alpha);
#endif
    result.erase(std::remove_if(result.begin(),result.end(),
        [&](const auto &x){return x.second<alpha-search_delta;}),
        result.end());
    searchstat.timing();
    searchstat.maxv=alpha;
    searchstat.pv.assign(result.begin(), result.end());
    debugout<<searchstat.str()<<'\n';
}

int search_root(int depth, CBoard cboard, int suggestp){
#ifdef DEBUGTREE
    if (debug_tree)
        debug_tree->step_in(__func__,depth, cboard, -INF, INF);
#endif
    searchstat.reset(depth);
    u64 move=cboard.genmove();
    assertprintf(move, "nowhere to play\n");
    std::vector<PosVal> result;
    Val alpha=-INF;
    if (suggestp!=-1){
        btr(move, suggestp);
        alpha = -search_normal(depth-1, cboard.cmakemove_r(suggestp), -INF, -alpha+search_delta+0.01, 0);
        result.emplace_back(suggestp, alpha);
    }
    for (auto p:u64iter(move)){
        Val ret=-search_normal(depth-1, cboard.cmakemove_r(p), -INF, -alpha+search_delta+0.01, 0);
        if (ret>alpha) alpha=ret, suggestp=p;
        if (ret>=alpha-search_delta) result.emplace_back(p, ret);
    }
    if (btimeout) return 0;
#ifdef DEBUGTREE
    if (debug_tree) debug_tree->step_out(alpha);
#endif
    result.erase(
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-search_delta;}),
        result.end());
    searchstat.maxv=alpha;
    searchstat.timing();
    searchstat.pv.assign(result.begin(), result.end());
    debugout<<searchstat.str()<<'\n';
    return suggestp;
}

void search_id(CBoard board, int maxd){
    searchstat_sum.timing(); searchstat_sum.leafcnt=0;
    int p=-1;
    maxd=std::min(maxd, popcnt(board.emptys())-ENDSEARCH_BEGIN+1);
    for (int depth=4;depth<=maxd;depth++){
        p = search_root(depth, board, p);
        if (btimeless) break;
        searchstat_sum.leafcnt+=searchstat.leafcnt;
    }
    searchstat_sum.timing();
    searchstat_sum.leafcnt+=searchstat.leafcnt;
    debugout<<"st:"<<searchstat_sum.tl<<"  sc:"<<searchstat_sum.leafcnt<<'\n';
}

#ifndef ONLINE
int think_maxd=11;

void reset_hash(){
    if (debug_flag&32)
        for (int i=0;i<1<<20;i++)
            translation_table[i].board = Board(0,0);
}

int think_choice(CBoard board){
    #ifdef GENDATA_PC
    memset(pc_statecnt,0, sizeof(pc_statecnt));
    #endif
    reset_hash();
    debugout.str("");
    if (popcnt(board.emptys())<=12)
        search_exact_root(board);
    else
        search_id(board, think_maxd); // id+hash is faster most of the time
        //search_root(think_maxd, board, -1);
    return searchstat.pv[rand()%searchstat.pv.size()].first;
}
#endif //ONLINE

//int think_checktime=330, think_maxtime=910;
int think_checktime=100, think_maxtime=300;

int think_choice_td(CBoard board){
    debugout.str("");
    if (popcnt(board.emptys())<=12)
        search_exact_root(board);
    else{
        btimeless=btimeout=false;
        static std::timed_mutex tmux;
        std::thread thd([&]{
            tmux.lock();
            search_id(board, popcnt(board.emptys()));
            tmux.unlock();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(think_checktime));
        btimeless=true;
        if (tmux.try_lock_for(std::chrono::milliseconds(think_maxtime-think_checktime))) tmux.unlock();
        btimeout=true;
        thd.join();
    }
    return searchstat.pv[rand()%searchstat.pv.size()].first;
}
