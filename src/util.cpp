#include <stdio.h>
#include "util.h"

void assert(bool val, const char *fmt, va_list args){
#ifdef DEBUG
	if (!val){
		vprintf(fmt, args);
		fflush(stdout);
		abort();
	}
#endif
}

inline ull popcnt(ull x){
	ull ret;
	asm("popcnt %1,%0":"=r"(ret):"r"(x));
	return ret;
}
inline ull pext(ull x, ull mask){
	ull ret;
	asm("pext %1,%2,%0":"=r"(ret):"r"(mask), "r"(x));
	return ret;
}
inline ull pdep(ull x, ull mask){
	ull ret;
	asm("pdep %1,%2,%0":"=r"(ret):"r"(mask), "r"(x));
	return ret;
}
inline bool bget(ull x, int p){return x>>p&1;}
inline void bts(ull &x, int p){asm("bts %1,%0\n":"+r"(x):"r"(p));}
inline void btr(ull &x, int p){asm("btr %1,%0\n":"+r"(x):"r"(p));}
inline int ctz(ull x){return __builtin_ctzll(x);}

void showMask(ull x){
	for(int i=0;i<8;i++,puts(""))
		for(int j=0;j<8;j++)
			putchar((x>>(i*8+j)&1)?'*':'.');
}
