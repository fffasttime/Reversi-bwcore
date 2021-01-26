#pragma once

#include "util.h"
#include "bitptn.h"
#include <string>

/*
bitmove: 
UL U UR      >>9 >>8 >>7
 L .  R  =>  >>1  .  <<1
DL D DR      <<7 <<8 <<9
*/
class Board{
public:
	u64 b[2];

	void setStart(){
		b[0]=0x810000000u;
		b[1]=0x1008000000u;
	}
	bool operator==(const Board &v) const{return b[0]==v.b[0] && b[1]==v.b[1];}

	std::string repr() const;
	std::string str() const;

	u64 genmove(int col) const{
		// This part of code is brought from Zebra & stdrick
		cu64 b_cur = b[col], b_opp = b[!col];
		u64 b_opp_inner = b_opp & 0x7E7E7E7E7E7E7E7Eu;
		u64 moves = 0;
		u64 b_flip, b_opp_adj;

		#define GENMOVE(arrow, p, opp) \
		b_flip = (b_cur arrow p) & opp; \
		b_flip |= (b_flip arrow p) & opp; \
		b_opp_adj = opp & (opp arrow p); \
		b_flip |= (b_flip arrow (p+p)) & b_opp_adj; \
		b_flip |= (b_flip arrow (p+p)) & b_opp_adj; \
		moves |= b_flip arrow p
		GENMOVE(>>, 1, b_opp_inner); GENMOVE(<<, 1, b_opp_inner);
		GENMOVE(>>, 8, b_opp); GENMOVE(<<, 8, b_opp);
		GENMOVE(>>, 7, b_opp_inner); GENMOVE(<<, 7, b_opp_inner);
		GENMOVE(>>, 9, b_opp_inner); GENMOVE(<<, 9, b_opp_inner);
		#undef GENMOVE

		return moves & ~(b_cur | b_opp);
	}
	
	bool testmove(int p, int col) const{
		using namespace bitptn;
		bool f=0;
		#define FLIP_OP(dir)\
		f|=ptn_canflip[pl_##dir [p]][pext(b[ col],pm_##dir [p])][pext(b[!col],pm_##dir [p])];
		FLIP_OP(h); FLIP_OP(v); FLIP_OP(d1); FLIP_OP(d2);
		#undef FLIP_OP
		return f;
	}
	void makemove(int p, int col){
		using namespace bitptn;
		u64 n_cur=0, n_opp=0, lb, lw;

		#define FLIP_OP(dir)\
		lb=pext(b[ col],pm_##dir [p]);\
		lw=pext(b[!col],pm_##dir [p]);\
		n_cur|=pdep(ptn_flip[pl_##dir [p]][lb][lw][0],pm_##dir [p]);\
		n_opp|=pdep(ptn_flip[pl_##dir [p]][lb][lw][1],pm_##dir [p]);
		FLIP_OP(h); FLIP_OP(v); FLIP_OP(d1); FLIP_OP(d2);
		#undef FLIP_OP

		b[ col]=(b[ col]&p_umask[p])|n_cur;
		b[!col]=(b[!col]&p_umask[p])|n_opp;
	}
	
	int cnt(int col) const{return popcnt(b[col]);}	
};

class Game{
public:
	Board board;
	Board board_before[60];
	int step;
	int col;
	Game(){
		step=0; col=0;
		board.setStart();
	}
	void makemove(int p){
		assert(p>=0 && p<64, "invalid move at %d\n", p);
		assert(board.testmove(p, col),"invalid move at %d\n", p);
		assert(step<60, "invalid makemove steps");
		board_before[step++]=board;
		board.makemove(p, col);
		col=!col; if (!hasmove()) col=!col;
	}
	void unmakemove(){
		assert(step>=0, "unmakemove outbound\n");
		board=board_before[--step];
	}
	u64 genmove(){return board.genmove(col);}
	bool hasmove(){return popcnt(board.genmove(col));}
	void print();
};
