#pragma once

#define PBLACK 0
#define PWHITE 1
#define BSIZE 8
#define BSIZE2 64
#define inc(i,n) for(int i=0;i<n;i++)

typedef unsigned char u8;
typedef unsigned long long u64;
typedef const u64& cu64;

#ifdef DEBUG
void assert(bool val, const char *fmt, ...);
#else
inline void assert(bool val, const char *fmt, ...){}
#endif

inline int pos(int x, int y){assert(x>=0 && x<8 && y>=0 && y<8, "invalid pos\n"); return x*8+y;}

inline u64 popcnt(cu64 x){
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
inline int ctz(u64 x){assert(x, "ctz 0 is undefined behavior\n"); return __builtin_ctzll(x);}
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


void showMask(u64 x);

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
