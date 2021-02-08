#pragma once
#include <algorithm>
#include <array>
#include "util.h"

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

    CEXPR_ARRAY_DEF(pl_h, 64, x%8);
    CEXPR_ARRAY_DEF(pl_v, 64, x/8);
    CEXPR_ARRAY_DEF(pl_d1, 64, x/8<x%8?x/8:x%8);
    CEXPR_ARRAY_DEF(pl_d2, 64, x/8<(7-x%8)?x/8:(7-x%8));
    CEXPR_ARRAY_DEF(pm_h, 64, h[x/8]);
    CEXPR_ARRAY_DEF(pm_v, 64, v[x%8]);
    CEXPR_ARRAY_DEF(pm_d1, 64, d1[x/8-x%8+7]);
    CEXPR_ARRAY_DEF(pm_d2, 64, d2[x/8+x%8]);
    CEXPR_ARRAY_DEF(p_umask, 64, ~(pm_h[x]|pm_v[x]|pm_d1[x]|pm_d2[x]));
#undef CEXPR_ARRAY_DEF

constexpr u64 edge2x=0x42ff;
constexpr u64 c33=0x70707;
constexpr u64 c52=0x1f1f;

extern u8 ptn_flip[8][256][256][2];
extern bool ptn_canflip[8][256][256];

void initPtnFlip();

}