#pragma once

#define PBLACK 0
#define PWHITE 1
#define BSIZE 8
#define BSIZE2 64

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
