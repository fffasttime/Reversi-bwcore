#pragma once

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define PBLACK 0
#define PWHITE 1
#define inc(i,n) for(int i=0;i<n;i++)

typedef unsigned char u8;
typedef unsigned long long u64;
typedef const u64& cu64;

#ifdef DEBUG
void assertprintf(bool val, const char *fmt, ...);
#define DEBUGTREE
#else
inline void assertprintf(bool val, const char *fmt, ...){}
#endif

inline int pos(int x, int y){assertprintf(x>=0 && x<8 && y>=0 && y<8, "invalid pos\n"); return x*8+y;}

inline int popcnt(cu64 x){
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
inline int ctz(u64 x){assertprintf(x, "ctz 0 is undefined behavior\n"); return __builtin_ctzll(x);}
inline int tzcnt(u64 x){if (x==0) return 64; return __builtin_ctzll(x);}
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
#ifndef ONLINE
void showMask(u64 x);
#endif //ONLINE
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

#include <array>
namespace bitptn{
// generate conxtexpr arrays
template<class Function, std::size_t... Indices>
constexpr auto make_array_helper(Function f, std::index_sequence<Indices...>) 
-> std::array<typename std::result_of<Function(std::size_t)>::type, sizeof...(Indices)> {
    return {{ f(Indices)... }};
}
template<int N, class Function>
constexpr auto make_array(Function f)
-> std::array<typename std::result_of<Function(std::size_t)>::type, N>{
    return make_array_helper(f, std::make_index_sequence<N>{});    
}
#define CEXPR_ARRAY_DEF(name, N, expr) \
    constexpr static auto f##name(int x){return (expr);}\
    constexpr auto name = make_array<N>(f##name)
    // common mask '-' '|' '\' '/', ascending order
    CEXPR_ARRAY_DEF(h, 8, 0xffull<<(x*8));
    CEXPR_ARRAY_DEF(v, 8, 0x101010101010101ull<<x);
    CEXPR_ARRAY_DEF(d1, 15, x>7?0x8040201008040201u<<((x-7)*8):0x8040201008040201u>>((7-x)*8));
    CEXPR_ARRAY_DEF(d2, 15, x>7?0x0102040810204080u<<((x-7)*8):0x0102040810204080u>>((7-x)*8));
#undef CEXPR_ARRAY_DEF

constexpr u64 edge2x=0x42ff, c33=0x70707, c52=0x1f1f, ccor=0x8100000000000081,
cx22=0x0042000000004200, pedge=0x7e8181818181817e, pinner=0x003c7e7e7e7e3c00;
}
