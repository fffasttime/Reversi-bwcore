#pragma once
#include <stdarg.h>

#define PBLACK 0
#define PWHITE 1
#define BSIZE 8
#define BSIZE2 64

typedef unsigned char byte;
typedef unsigned long long ull;

void assert(bool val, const char *fmt, va_list args);

inline ull popcnt(ull x);
inline ull pext(ull x, ull mask);
inline ull pdep(ull x, ull mask);
inline bool bget(ull x, int p);
inline void bts(ull &x, int p);
inline void btr(ull &x, int p);
inline int ctz(ull x);

void showMask(ull x);

struct u64iter{
	ull x;
    u64iter(x):x(x){}
    struct iter;
	iter begin();
	iter end();
};
struct u64iter::iter{
	u64iter x;
	int operator*()const{return tzcnt(x.x);}
	bool operator!=(const iter &v) const{return x!=v.x;}
	iter &operator++(){
		x.x&=x.x-1;
		return *this;
	}
};
u64iter::iter u64iter::begin(){
	iter it; it.x=*this;
	return ++it;
}
u64iter::iter u64iter::end(){
	iter it; it.x.x=0;
	return it;
}
