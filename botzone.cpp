/*
Author: fffasttime
See https://github.com/fffasttime/Reversi-bwcore
*/
#include <bits/stdc++.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define PBLACK 0
#define PWHITE 1
#define inc(i,n) for(int i=0;i<n;i++)
typedef unsigned char u8;
typedef unsigned long long u64;
typedef const u64& cu64;
inline int pos(int x, int y){ return x*8+y;}
inline int popcnt(cu64 x){
	u64 ret;
	asm("popcnt %1,%0":"=r"(ret):"r"(x));
	return ret;
}
inline u64 pext(cu64 x, cu64 mask){
	u64 ret;
	asm("pext %1,%2,%0":"=r"(ret):"r"(mask), "r"(x));
	return ret;
}
inline u64 pdep(cu64 x, cu64 mask){
	u64 ret;
	asm("pdep %1,%2,%0":"=r"(ret):"r"(mask), "r"(x));
	return ret;
}
inline u64 blsr(cu64 x){
	u64 ret;
	asm("blsr %1,%0":"=r"(ret):"r"(x));
	return ret;
}
inline bool bget(cu64 x, u64 p){
	bool ret;
	asm(
		"bt %2,%1\n"
		"setc %0\n"
		:"=&r"(ret) :"r"(x), "r"(p));
	return ret;
}
inline void bts(u64 &x, u64 p){asm("bts %1,%0\n":"+r"(x):"r"(p));}
inline void btr(u64 &x, u64 p){asm("btr %1,%0\n":"+r"(x):"r"(p));}
inline int ctz(u64 x){ return __builtin_ctzll(x);}
inline int tzcnt(u64 x){if (x==0) return 64; return __builtin_ctzll(x);}
inline void flip_h(u64 &x){asm("bswap %0\n":"+r"(x));}
inline void flip_v(u64 &x){
	x = (x & 0xaaaaaaaaaaaaaaaa) >> 1  | (x & 0x5555555555555555) << 1;
	x = (x & 0xcccccccccccccccc) >> 2  | (x & 0x3333333333333333) << 2;
	x = (x & 0xf0f0f0f0f0f0f0f0) >> 4  | (x & 0x0f0f0f0f0f0f0f0f) << 4;
}
inline void rotate_r(u64 &x){
	x = (x & 0xf0f0f0f000000000) >> 4  | (x & 0x0f0f0f0f00000000) >> 32
	  | (x & 0x00000000f0f0f0f0) << 32 | (x & 0x000000000f0f0f0f) << 4;
	x = (x & 0xcccc0000cccc0000) >> 2  | (x & 0x3333000033330000) >> 16
	  | (x & 0x0000cccc0000cccc) << 16 | (x & 0x0000333300003333) << 2;
	x = (x & 0xaa00aa00aa00aa00) >> 1  | (x & 0x5500550055005500) >> 8
	  | (x & 0x00aa00aa00aa00aa) << 8  | (x & 0x0055005500550055) << 1;
}
inline void rotate_l(u64 &x){
	x = (x & 0xf0f0f0f000000000) >> 32 | (x & 0x0f0f0f0f00000000) << 4
	  | (x & 0x00000000f0f0f0f0) >> 4  | (x & 0x000000000f0f0f0f) << 32;
	x = (x & 0xcccc0000cccc0000) >> 16 | (x & 0x3333000033330000) << 2
	  | (x & 0x0000cccc0000cccc) >> 2  | (x & 0x0000333300003333) << 16;
	x = (x & 0xaa00aa00aa00aa00) >> 8  | (x & 0x5500550055005500) << 1
	  | (x & 0x00aa00aa00aa00aa) >> 1  | (x & 0x0055005500550055) << 8;
}
struct u64iter{
	u64 x;
    u64iter(u64 x):x(x){}
    struct iter;
	iter begin();
	iter end();
};
struct u64iter::iter{
	u64iter x;
	int operator*() const{return ctz(x.x);}
	bool operator!=(const iter &v) const{return x.x!=v.x.x;}
	iter &operator++(){
		x.x=blsr(x.x);
		return *this;
	}
};
inline u64iter::iter u64iter::begin(){return {*this};}
inline u64iter::iter u64iter::end(){return {u64iter(0)};}
namespace bitptn{
template<class Function, std::size_t... Indices>
constexpr auto make_array_helper(Function f, std::index_sequence<Indices...>)
-> std::array<typename std::result_of<Function(std::size_t)>::type, sizeof...(Indices)> {
    return {{ f(Indices)... }};
}
template<int N, class Function>
constexpr auto make_array(Function f)
-> std::array<typename std::result_of<Function(std::size_t)>::type, N>{
    return make_array_helper(f, std::make_index_sequence<N>{});
}
#define CEXPR_ARRAY_DEF(name, N, expr) \
    constexpr static auto f##name(int x){return (expr);}\
    constexpr auto name = make_array<N>(f##name)
    CEXPR_ARRAY_DEF(h, 8, 0xffull<<(x*8));
    CEXPR_ARRAY_DEF(v, 8, 0x101010101010101ull<<x);
    CEXPR_ARRAY_DEF(d1, 15, x>7?0x8040201008040201u<<((x-7)*8):0x8040201008040201u>>((7-x)*8));
    CEXPR_ARRAY_DEF(d2, 15, x>7?0x0102040810204080u<<((x-7)*8):0x0102040810204080u>>((7-x)*8));
#undef CEXPR_ARRAY_DEF
constexpr u64 edge2x=0x42ff, c33=0x70707, c52=0x1f1f, ccor=0x8100000000000081,
cx22=0x0042000000004200, pedge=0x7e8181818181817e, pinner=0x003c7e7e7e7e3c00;
}
class Board{
public:
	u64 b[2];
	Board(){}
	Board(u64 _0, u64 _1):b{_0,_1}{}
	void setStart(){b[0]=0x810000000,b[1]=0x1008000000;}
	bool operator==(const Board &v) const{return b[0]==v.b[0] && b[1]==v.b[1];}
	std::string repr() const;
	template<int col=0> u64 genmove() const{
		cu64 b_cur = b[col], b_opp = b[!col];
		u64 b_opp_inner = b_opp & 0x7E7E7E7E7E7E7E7Eu;
		u64 moves = 0;
		u64 b_flip, b_opp_adj;
		#define GENMOVE(arrow, d, opp) \
		b_flip = (b_cur arrow d) & opp; \
		b_flip |= (b_flip arrow d) & opp; \
		b_opp_adj = opp & (opp arrow d); \
		b_flip |= (b_flip arrow (d+d)) & b_opp_adj; \
		b_flip |= (b_flip arrow (d+d)) & b_opp_adj; \
		moves |= b_flip arrow d
		GENMOVE(>>, 1, b_opp_inner); GENMOVE(<<, 1, b_opp_inner);
		GENMOVE(>>, 8, b_opp); GENMOVE(<<, 8, b_opp);
		GENMOVE(>>, 7, b_opp_inner); GENMOVE(<<, 7, b_opp_inner);
		GENMOVE(>>, 9, b_opp_inner); GENMOVE(<<, 9, b_opp_inner);
		#undef GENMOVE
		return moves & ~(b_cur | b_opp);
	}
	bool testmove(int p) const{return bget(genmove(),p);}
	template<int col=0> bool makemove(int p){
		using namespace bitptn;
		u64 &b_cur = b[col]; u64 &b_opp = b[!col];
		u64 b_flip, b_opp_adj, flips = 0;
		u64 b_opp_inner = b_opp & 0x7E7E7E7E7E7E7E7Eu;
		#define GENMOVE(arrow, d, opp) \
		b_flip = ((1ull<<p) arrow d) & opp; \
		b_flip |= (b_flip arrow d) & opp; \
		b_opp_adj = opp & (opp arrow d); \
		b_flip |= (b_flip arrow (d+d)) & b_opp_adj; \
		b_flip |= (b_flip arrow (d+d)) & b_opp_adj; \
		if((b_flip arrow d) & b_cur) flips |= b_flip
		GENMOVE(>>, 1, b_opp_inner); GENMOVE(<<, 1, b_opp_inner);
		GENMOVE(>>, 8, b_opp); GENMOVE(<<, 8, b_opp);
		GENMOVE(>>, 7, b_opp_inner); GENMOVE(<<, 7, b_opp_inner);
		GENMOVE(>>, 9, b_opp_inner); GENMOVE(<<, 9, b_opp_inner);
		#undef GENMOVE
		if(flips) b_cur^=flips, b_opp^=flips, bts(b_cur, p);
		return flips;
	}
	bool cmakemove(int p){cswap();return makemove<1>(p);}
	Board makemove_r(int p) const{Board r=*this;r.makemove(p);return r;}
	Board cmakemove_r(int p) const{Board r(b[1],b[0]);r.makemove<1>(p);return r;}
	int cnt0() const{return popcnt(b[0]);}
	int cnt1() const{return popcnt(b[1]);}
	u64 hash() const{return (b[0]*19260817)^b[1];}
	Board cswap_r() const{return Board(b[1],b[0]);}
	void cswap(){std::swap(b[0],b[1]);}
	void flip_h(){::flip_h(b[0]);::flip_h(b[1]);}
	void flip_v(){::flip_v(b[0]);::flip_v(b[1]);}
	void rotate_l(){::rotate_l(b[0]);::rotate_l(b[1]);}
	void rotate_r(){::rotate_r(b[0]);::rotate_r(b[1]);}
	u64 emptys() const{return ~(b[0]|b[1]);}
	u64 occupys() const{return b[0]|b[1];}
	int operator[](int p) const{return 2-2*bget(b[0], p)-bget(b[1], p);}
};
typedef const Board &CBoard;
class Game{
public:
	std::string repr() const;
	Board board;
	Board board_before[60];
	int col_before[60], move_before[60];
	int step,col;
	Game(){ step=col=0; board.setStart(); }
	bool testmove(int p){return p>=0 && p<64 && board.testmove(p);}
	void makemove(int p, bool autopass=1){
		col_before[step]=col;
		board_before[step]=board;
		move_before[step]=p;
		step++;
		board.cmakemove(p); col=!col;
		if (autopass && !hasmove()) col=!col, board.cswap();
	}
	void unmakemove(){
		--step;
		board=board_before[step];
		col=col_before[step];
	}
	void reset(){
		step=0, col=0;
		board.setStart();
	}
	bool isend(){return !hasmove();}
	int cnt(int _col) const{return popcnt(board.b[col^_col]);}
	int winner() const{
		if (cnt(PBLACK)>cnt(PWHITE)) return PBLACK;
		else if (cnt(PBLACK)<cnt(PWHITE)) return PWHITE;
		return 2;
	}
	u64 genmove() const{return board.genmove();}
	bool hasmove() const{return popcnt(board.genmove());}
	int operator[](int p) const{
		return board[p];
	}
};
std::string Board::repr() const{
    std::ostringstream o;
    o<<std::hex<<"0x"<<b[0]<<" 0x"<<b[1];
    return o.str();
}
std::string Game::repr() const{
    std::ostringstream o;
    o<<"board "<<board.repr()<<" col "<<col;
    return o.str();
}
constexpr unsigned short pow3[]={1,3,9,27,81,243,729,2187,6561,19683,59049};
#define EVAL_FILE "data/rawdata2/reversicoeff.bin"
constexpr int Eval_PrTable[61]={-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
unsigned short pow4to3_10[1<<20], pow4to3_9[1<<18];
struct CoeffPack{
    short e1[pow3[10]], c52[pow3[10]], c33[pow3[9]],
        e2[1<<16], e3[1<<16], e4[1<<16], k8[1<<16], k7[1<<14], k6[1<<12], k5[1<<10], k4[1<<8],
        wb, wodd, wmob, wcinner[65], wcedge, wedgeodd, ccor[1<<8], cx22[1<<8];
}pdata[12];
void readShort(FILE *stream, short &tar){fread(&tar, 2, 1, stream);}
int pow3to4(int len, int x){
    int b=0, w=0;
    inc(i,len){
        b<<=1; w<<=1;
        int p=x/pow3[i]%3;
        if (p==2) b|=1;
        if (p==1) w|=1;
    }
    return b<<len|w;
}
void initPtnConfig(){
    inc(i,pow3[10]) pow4to3_10[pow3to4(10, i)]=i;
    inc(i,pow3[9]) pow4to3_9[pow3to4(9, i)]=i;
}
void loadPtnData(){
    FILE *in=fopen(EVAL_FILE, "rb");
    short format_version; readShort(in, format_version);
    short part_cnt; readShort(in, part_cnt);
    short type_cnt; readShort(in, type_cnt);
    short _; inc(i, type_cnt)  readShort(in, _);
    short checksum=0;
    auto rdc=[&](short &x){readShort(in, x); checksum^=x;};
    for (int i=1;i<1+part_cnt;i++){
        CoeffPack &p=pdata[i];
        rdc(p.wb);
        rdc(p.wodd);
        rdc(p.wmob);
        rdc(p.wcedge);
        rdc(p.wedgeodd);
        inc(j,65) rdc(p.wcinner[j]);
        inc(j,pow3[10]) rdc(p.e1[j]);
        inc(j,pow3[10]) rdc(p.c52[j]);
        inc(j,pow3[9])  rdc(p.c33[j]);
        inc(j,pow3[8])  rdc(p.e2[pow3to4(8,j)]);
        inc(j,pow3[8])  rdc(p.e3[pow3to4(8,j)]);
        inc(j,pow3[8])  rdc(p.e4[pow3to4(8,j)]);
        inc(j,pow3[8])  rdc(p.k8[pow3to4(8,j)]);
        inc(j,pow3[7])  rdc(p.k7[pow3to4(7,j)]);
        inc(j,pow3[6])  rdc(p.k6[pow3to4(6,j)]);
        inc(j,pow3[5])  rdc(p.k5[pow3to4(5,j)]);
        inc(j,pow3[4])  rdc(p.k4[pow3to4(4,j)]);
        inc(j,pow3[4])  rdc(p.ccor[pow3to4(4,j)]);
        inc(j,pow3[4])  rdc(p.cx22[pow3to4(4,j)]);
    }
    short file_checksum; readShort(in, file_checksum);
    fclose(in);
}
int evalPtn(const Board &board){
    using namespace bitptn;
    int empty_cnt = popcnt(board.emptys());
    const CoeffPack &p=pdata[Eval_PrTable[empty_cnt]];
    Board b_id=board;
    int score=p.wb;
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
using std::max;
typedef float Val;
constexpr int INF=256;
typedef std::pair<int, Val> PosVal;
Val search_end2(CBoard board);
inline Val eval_end(CBoard board){return board.cnt0()-popcnt(board.b[1]);}
template<int depth>
Val search_end(CBoard cboard, Val alpha, Val beta, bool pass){
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
SearchStat searchstat, searchstat_sum;
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
    Board board;
    u8 pv, depth;
    u8 type;
    Val val;
}translation_table[1<<20];
Val search_end2(CBoard cboard){
    searchstat.leafcnt++;
    u64 emptys=cboard.emptys();
    int p1=ctz(emptys); emptys=blsr(emptys);
    int p2=ctz(emptys);
    Board board=cboard;
    int v1=-INF, v2;
    if (board.makemove(p1)){
        if (board.makemove<1>(p2) || board.makemove(p2)) v1=board.cnt0()*2;
        else v1=board.cnt0()*2+1;
        board=cboard;
    }
    if (board.makemove(p2)){
        if (board.makemove<1>(p1) || board.makemove(p1)) v2=board.cnt0()*2;
        else v2=board.cnt0()*2+1;
        return std::max(v1,v2)-64;
    }
    if (likely(v1!=-INF)) return v1-64;
    v1=INF;
    if (board.makemove<1>(p1)){
        if (board.makemove(p2) || board.makemove<1>(p2)) v1=board.cnt0()*2;
        else v1=board.cnt0()*2+1;
        board=cboard;
    }
    if (board.makemove<1>(p2)){
        if (board.makemove(p1) || board.makemove<1>(p1)) v2=board.cnt0()*2;
        else v2=board.cnt0()*2+1;
        return std::min(v1,v2)-64;
    }
    if (v1!=INF) return v1-64;
    return board.cnt0()*2-62;
}
Val search_exact(int depth, CBoard cboard, Val alpha, Val beta, bool pass){
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
}
Val evalMidGame(CBoard cboard){return evalPtn(cboard)/256.0;}
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
Val probcut(int depth, CBoard cboard, Val alpha, Val beta){
    const Val t=1.4;
    Val bound, ret;
    int cnt=popcnt(cboard.occupys());
    PC_Param &pa=pc_param[cnt][depth];
    if (pa.sigma>50) return (alpha+beta)/2;
    bound=(t*pa.sigma+beta-pa.b)/pa.w;
    ret=search_normal(pc_depth[depth],cboard, bound-0.01, bound);
    if (ret>=bound) return beta;
    bound=(-t*pa.sigma+alpha-pa.b)/pa.w;
    ret=search_normal(pc_depth[depth],cboard, bound, bound+0.01);
    if (ret<bound) return alpha;
    return (alpha+beta)/2;
}
#define ENDSEARCH_BEGIN 6
#define USE_PC
int hash_hitc;
bool btimeout, btimeless;
Val search_normal(int depth, CBoard cboard, Val alpha, Val beta, bool pass){
    if (unlikely(btimeout)) return 0;
    u64 move=cboard.genmove();
    if (!move){
        if (pass) return eval_end(cboard);
        return -search_normal(depth, cboard.cswap_r(), -beta, -alpha, 1);
    }
    int remain=popcnt(cboard.emptys());
    if (remain==ENDSEARCH_BEGIN) return search_end<ENDSEARCH_BEGIN>(cboard, alpha, beta, 0);
    if (depth==1) return search_normal1(1, cboard, alpha, beta);
    int firstp=ctz(move);
    auto ttnode=translation_table + cboard.hash()%(1<<20);
    if (ttnode->board==cboard){
        firstp = ttnode->pv;
        hash_hitc++;
        if (ttnode->depth==depth){
            Val val=ttnode->val;
            if (ttnode->type>=2) alpha=max(alpha, val);
            if (alpha>=beta) return alpha;
        }
    }
#ifdef USE_PC
    if (depth>=3 && depth<=MPC_MAXD){
        Val val=probcut(depth, cboard, alpha, beta);
        if (val>=beta || val<=alpha) return val;
    }
#endif
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
    btr(move, firstp);
    Val val=-search_normal(depth-1, cboard.cmakemove_r(firstp), -beta, -alpha);
    if (val>alpha){
        if (val>=beta) goto BETA_CUT;
        alpha=val;
    }
    for (auto p:u64iter(move)){
        Board board=cboard.cmakemove_r(p);
        Val ret=-search_normal(depth-1, board, -alpha-0.01, -alpha);
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
    if (!(ttnode->board==cboard)){
        ttnode->board=cboard;
        ttnode->depth=depth;
    }
    ttnode->pv=pv;
    ttnode->val=val;
    if (val<=alpha) ttnode->type=1;
    else if (val>=beta) ttnode->type=2;
    else ttnode->type=3;
    return val;
}
int random_choice(CBoard board){
    std::vector<int> pos;
    for (auto p: u64iter(board.genmove()))
        pos.push_back(p);
    return pos[rand()%pos.size()];
}
std::ostringstream debugout;
float search_delta=1.0;
void search_exact_root(CBoard cboard){
    searchstat.reset(popcnt(cboard.emptys()));
    u64 move=cboard.genmove();
    std::vector<PosVal> result;
    Val alpha=-INF;
    for (auto p:u64iter(move)){
        Board board=cboard.cmakemove_r(p);
        Val ret=-search_exact(searchstat.depth-1, board, -INF, -alpha+search_delta+0.01, 0);
        alpha=max(alpha, ret);
        if (ret>=alpha-search_delta) result.emplace_back(p, ret);
    }
    result.erase(std::remove_if(result.begin(),result.end(),
        [&](const auto &x){return x.second<alpha-search_delta;}),
        result.end());
    searchstat.timing();
    searchstat.maxv=alpha;
    searchstat.pv.assign(result.begin(), result.end());
    debugout<<searchstat.str()<<'\n';
}
int search_root(int depth, CBoard cboard, int suggestp){
    hash_hitc=0;
    searchstat.reset(depth);
    u64 move=cboard.genmove();
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
    result.erase(
    std::remove_if(result.begin(),result.end(),[&](const auto &x){return x.second<alpha-search_delta;}),
        result.end());
    searchstat.maxv=alpha;
    searchstat.timing();
    searchstat.pv.assign(result.begin(), result.end());
    debugout<<hash_hitc<<' '<<searchstat.str()<<'\n';
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
int think_checktime=330, think_maxtime=580;
int think_choice_td(CBoard board){
    debugout.str("");
    if (popcnt(board.emptys())<=12)
        search_exact_root(board);
    else{
        btimeless=btimeout=false;
        std::timed_mutex tmux;
        std::thread thd([&]{
            tmux.lock();
            search_id(board, popcnt(board.emptys()));
            tmux.unlock();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(think_checktime));
        btimeless=true;
        if (tmux.try_lock_for(std::chrono::milliseconds(think_maxtime))) tmux.unlock();
        btimeout=true;
        thd.join();
    }
    return searchstat.pv[rand()%searchstat.pv.size()].first;
}
int main(){
    srand(time(nullptr));
    initPtnConfig();
    loadPtnData();
    loadPCData();
    Game game;
    int n,x,y; scanf("%d", &n);
    for(n=0;;n++){
        scanf("%d%d", &x, &y);
        if (x!=-1) game.makemove(pos(x,y), 0);
        else if (n) game.col=!game.col, game.board.cswap();
        if (game.hasmove()){
            int sp=think_choice_td(game.board);
            printf("%d %d\n", sp/8, sp%8);
            printf("%s, %s, st:%d, scnt: %d\n", game.repr().c_str(), searchstat.str().c_str(),
                searchstat_sum.tl, searchstat_sum.leafcnt);
            game.makemove(sp, 0);
        }
        else puts("-1 -1"),puts(""), game.col=!game.col, game.board.cswap();
        puts("\n");
        printf(">>>BOTZONE_REQUEST_KEEP_RUNNING<<<\n");
        fflush(stdout);
    }
    return 0;
}
