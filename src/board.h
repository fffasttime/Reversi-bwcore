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
	ull b[2];

	void setStart(){
		b[0]=0x810000000u;
		b[1]=0x1008000000u;
	}

	std::string repr() const;
	std::string str() const;

	ull genmove(int col) const{
		// This part of code is brought from Zebra & stdrick
		const ull& b_cur = b[col];
		const ull& b_opp = b[!col];
		ull moves;
		ull b_opp_inner;
		ull brd_flip;
		ull b_opp_adj;

		b_opp_inner = b_opp & 0x7E7E7E7E7E7E7E7Eu;

		brd_flip = (b_cur >> 1) & b_opp_inner;
		brd_flip |= (brd_flip >> 1) & b_opp_inner;

		b_opp_adj = b_opp_inner & (b_opp_inner >> 1);
		brd_flip |= (brd_flip >> 2) & b_opp_adj;
		brd_flip |= (brd_flip >> 2) & b_opp_adj;

		moves = brd_flip >> 1;

		brd_flip = (b_cur << 1) & b_opp_inner;
		brd_flip |= (brd_flip << 1) & b_opp_inner;

		b_opp_adj = b_opp_inner & (b_opp_inner << 1);
		brd_flip |= (brd_flip << 2) & b_opp_adj;
		brd_flip |= (brd_flip << 2) & b_opp_adj;

		moves |= brd_flip << 1;

		brd_flip = (b_cur >> 8) & b_opp;
		brd_flip |= (brd_flip >> 8) & b_opp;

		b_opp_adj = b_opp & (b_opp >> 8);
		brd_flip |= (brd_flip >> 16) & b_opp_adj;
		brd_flip |= (brd_flip >> 16) & b_opp_adj;

		moves |= brd_flip >> 8;

		brd_flip = (b_cur << 8) & b_opp;
		brd_flip |= (brd_flip << 8) & b_opp;

		b_opp_adj = b_opp & (b_opp << 8);
		brd_flip |= (brd_flip << 16) & b_opp_adj;
		brd_flip |= (brd_flip << 16) & b_opp_adj;

		moves |= brd_flip << 8;

		brd_flip = (b_cur >> 7) & b_opp_inner;
		brd_flip |= (brd_flip >> 7) & b_opp_inner;
		
		b_opp_adj = b_opp_inner & (b_opp_inner >> 7);
		brd_flip |= (brd_flip >> 14) & b_opp_adj;
		brd_flip |= (brd_flip >> 14) & b_opp_adj;
		
		moves |= brd_flip >> 7;

		brd_flip = (b_cur << 7) & b_opp_inner;
		brd_flip |= (brd_flip << 7) & b_opp_inner;

		b_opp_adj = b_opp_inner & (b_opp_inner << 7);
		brd_flip |= (brd_flip << 14) & b_opp_adj;
		brd_flip |= (brd_flip << 14) & b_opp_adj;

		moves |= brd_flip << 7;

		brd_flip = (b_cur >> 9) & b_opp_inner;
		brd_flip |= (brd_flip >> 9) & b_opp_inner;
		
		b_opp_adj = b_opp_inner & (b_opp_inner >> 9);
		brd_flip |= (brd_flip >> 18) & b_opp_adj;
		brd_flip |= (brd_flip >> 18) & b_opp_adj;
		
		moves |= brd_flip >> 9;
		
		brd_flip = (b_cur << 9) & b_opp_inner;
		brd_flip |= (brd_flip << 9) & b_opp_inner;

		b_opp_adj = b_opp_inner & (b_opp_inner << 9);
		brd_flip |= (brd_flip << 18) & b_opp_adj;
		brd_flip |= (brd_flip << 18) & b_opp_adj;

		moves |= brd_flip << 9;

		moves &= ~(b_cur | b_opp);
		return moves;
	}
	
	bool testmove(int p, int col) const{
		bool f=0;
		#define FILP_OP(dir)
		f|=ptn_flip[pl_##dir[p]][b[ col],pm_##dir[p]][b[!col],pm_##dir[p]][0];
		FILP_OP(h); FLIP_OP(v); FLIP_OP(d1); FLIP_OP(d2);
		#undef FLIP_OP
		return f;
	}
	void makemove(int p, int col){
		using namespace bitptn;
		ull n_cur=0, n_opp=0, lb, lw;

		#define FILP_OP(dir)
		lb=pext(b[ col],pm_##dir[p]);\
		lw=pext(b[!col],pm_##dir[p]);\
		n_cur|=pdep(ptn_flip[pl_##dir[p]][lb][lw][0],pm_##dir[p]);\
		n_opp|=pdep(ptn_flip[pl_##dir[p]][lb][lw][1],pm_##dir[p]);
		FILP_OP(h); FLIP_OP(v); FLIP_OP(d1); FLIP_OP(d2);
		#undef FLIP_OP

		b[ col]=(b[ col]&p_umask[p])|n_cur;
		b[!col]=(b[!col]&p_umask[p])|n_opp;
	}
	
	int cnt(int col){return popcnt(b[col]);}	
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
		assert(board.testmove(p, col),"invalid move at %d\n", p);
		board_before[step++]=board;
		board.makemove(p);
		col=!col; if (!hasmove()) col=!col;
	}
	void unmakemove(){
		assert(step>=0, "unmakemove outbound\n");
		board=board_before[--step];
	}
	void genmove(){return board.genmove(col);}
	bool hasmove(){return popcnt(board.genmove(col));}
	void print();
}
