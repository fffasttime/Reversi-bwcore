#pragma once

#include "util.h"
#include <string>
#include <algorithm>

/*
bitmove: 
UL U UR      >>9 >>8 >>7
 L .  R  =>  >>1  .  <<1
DL D DR      <<7 <<8 <<9
*/
class Board{
public:
	u64 b[2];
	Board(){}
	Board(u64 _0, u64 _1):b{_0,_1}{}
	void setStart(){b[0]=0x810000000,b[1]=0x1008000000;}
	bool operator==(const Board &v) const{return b[0]==v.b[0] && b[1]==v.b[1];}
	std::string repr() const;
#ifndef ONLINE
	std::string str() const;
#endif
	u64 genmove(int col) const{
		// This part of code is brought from Zebra & stdrick
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
	bool testmove(int p, int col) const{return bget(genmove(col),p);}
	bool makemove(int p, int col){
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
	Board makemove_r(int p, int col) const{
		Board r=*this;
		r.makemove(p, col);
		return r;
	}
	int cnt(int col) const{return popcnt(b[col]);}
	u64 hash() const{return (b[0]*19260817)^b[1];}

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
#ifndef ONLINE
	std::string str();
#endif
	std::string repr();
	Board board;
	Board board_before[60];
	int col_before[60];
	int step,col;
	Game(){ step=col=0; board.setStart(); }
	bool testmove(int p){return p>=0 && p<64 && board.testmove(p, col);}
	void makemove(int p, bool autopass=1){
		assertprintf(p>=0 && p<64, "invalid move at %d\n", p);
		assertprintf(board.testmove(p, col),"invalid move at %d\n", p);
		assertprintf(step<60, "invalid makemove steps");
		col_before[step]=col;
		board_before[step]=board;
		step++;
		board.makemove(p, col);
		col=!col; if (autopass && !hasmove()) col=!col;
	}
	void unmakemove(){
		assertprintf(step>=0, "unmakemove outbound\n");
		--step;
		board=board_before[step];
		col=col_before[step];
	}
	void reset(){
		step=0, col=0;
		board.setStart();
	}
	bool isend(){return !hasmove();}
	int cnt(int col) const{return board.cnt(col);}
	int winner() const{
		if (cnt(PBLACK)>cnt(PWHITE)) return PBLACK;
		else if (cnt(PBLACK)<cnt(PWHITE)) return PWHITE;
		return 2;
	}
	u64 genmove() const{return board.genmove(col);}
	bool hasmove() const{return popcnt(board.genmove(col));}
	
	int operator[](int p) const{
		assertprintf(p>=0 && p<64, "invalid pos call at [%d]\n", p); 
		return board[p];
	}
};
