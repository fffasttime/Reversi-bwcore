#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"

#ifdef DEBUG
void assertprintf(bool val, const char *fmt, ...){
	if (!val){
		va_list args;
		va_start(args, fmt);
		printf("assertion failed!\n");
		vprintf(fmt, args);
		va_end(args);
		fflush(stdout);
		abort();
	}
}
#endif

void showMask(u64 x){
	for(int i=0;i<8;i++,puts(""))
		for(int j=0;j<8;j++)
			putchar((x>>(i*8+j)&1)?'*':'.');
}
