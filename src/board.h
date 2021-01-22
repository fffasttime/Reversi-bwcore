#pragma once

#include "util.h"
#include "bitptn.h"

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

	template<int col>
	ull genmove() const{
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
	
	template<int col>
	bool testmove() const{
		bool f=0;
		#define FILP_OP(dir)
		f|=ptn_flip[pl_##dir[p]][b[ col],pm_##dir[p]][b[!col],pm_##dir[p]][0];
		FILP_OP(h); FLIP_OP(v); FLIP_OP(d1); FLIP_OP(d2);
		#undef FLIP_OP
		return f;
	}
	template<int col>
	void makemove(int p){
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
	
	template<int col>
	int cnt(){return popcnt(b[col]);}	
};
