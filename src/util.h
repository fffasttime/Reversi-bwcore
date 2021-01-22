#pragma once

#define PBLACK 0
#define PWHITE 1
#define BSIZE 8
#define BSIZE2 64

typedef unsigned char byte;
typedef unsigned long long ull;

inline ull popcnt(ull x);
inline ull pext(ull x, ull mask);
inline ull pdep(ull x, ull mask);

void showMask(ull x);
