#include <stdio.h>
#include "util.h"

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

void showMask(ull x){
	for(int i=0;i<8;i++,puts(""))
		for(int j=0;j<8;j++)
			putchar((x>>(i*8+j)&1)?'*':'.');
}