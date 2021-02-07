#include <cstdio>
#include <iostream>
#include <locale>
#include "util.h"
#include "board.h"
#include "evalptn.h"
#include "search.h"
#include "debugtree.h"
using std::cout;

void check(bool val, const char *str){
    if (!val) printf("fail ");
    else printf("pass ");
    printf(str); puts("");
    if (!val) exit(1);
}
#define CHECK(expr) check(expr, #expr) 

int t_start, t_end;
#define TSCTEST(expr) \
asm volatile("rdtsc":"=A" (t_start)::"%rdx");\
expr; \
asm volatile("rdtsc":"=A" (t_end)::"%rdx");\
printf(#expr); printf(" TSC=%d\n",t_end-t_start);

int main(){
    bitptn::initPtnFlip();
    initPtnConfig();

    //util.h
    CHECK(popcnt(~0)==64);
    CHECK(popcnt( 0)==0);
    CHECK(pext(~0,0xFF)==0xFF);
    CHECK(pext(0xFF00,~0)==0xFF00);
    
    u64 x=2;
    bts(x,45);
    CHECK(x==(1ull<<45|2));
    btr(x,1);
    CHECK(x==(1ull<<45));

    x|=2;
    int cnt=0;
    for (auto p:u64iter(x)){
        if (cnt==0) CHECK(p==1);
        else if (cnt==1) CHECK(p==45);
        cnt++;
    }
    CHECK(cnt==2);

    CHECK(bget(x,45)==1);
    CHECK(bget(x,46)==0);

    //board.h, on start board
    Board board;
    board.setStart();

    CHECK(popcnt(board.genmove(PBLACK)));

    CHECK(bitptn::pm_d1[1]==0x80402010080402);
    CHECK(bitptn::p_umask[10]==~0x4844424150eff0eu);

    CHECK(bitptn::v[4]==bitptn::pm_v[pos(3,4)]);
    CHECK(pext(board.b[0], bitptn::pm_v[pos(5,4)])==0x8);
    CHECK(pext(board.b[1], bitptn::pm_v[pos(5,4)])==0x10);

    CHECK(pext(board.b[0], bitptn::pm_d2[pos(0,7)])==0x18);
    CHECK(pext(board.b[1], bitptn::pm_d1[pos(0,0)])==0x18);

    CHECK(bitptn::ptn_canflip[5][0x8][0x10]);

    CHECK(bitptn::pl_v[pos(5,4)]==5);
    CHECK(board.testmove(pos(5,4), PBLACK));

    auto testmoves=[&](Board &b, int col){
        u64 x=0;
        for (int i=0;i<64;i++) 
            if (b.testmove(i, col)) 
                bts(x, i);
        return x;
    };

    inc(i,5){
        TSCTEST(inc(j,10) board.testmove(j, PBLACK)); //optimized out! 
    }
    inc(i,5){
        TSCTEST(inc(j,10) board.makemove(44, PBLACK)); 
    }
    inc(i,5){
        TSCTEST(inc(j,10) x=board.genmove(PBLACK));
    }
    Board b2; b2.b[0]=rand(); b2.b[1]=rand(); 
    inc(i,20){
        b2.b[0]+=231689789646623;b2.b[1]+=123456123656123; 
 		b2.b[1]^=b2.b[0]&b2.b[1];
        TSCTEST(evalPtn(b2, 1));
    }
    inc(i,5){
        TSCTEST(inc(j,10) popcnt(board.genmove(PBLACK)));
    }
    CHECK(testmoves(board, PBLACK)==x);
    CHECK(testmoves(board, PWHITE)==board.genmove(PWHITE));

    board.makemove(44, PBLACK);
    CHECK(board.b[0]==0x101810000000);
    CHECK(board.b[1]==0x8000000);
    CHECK(board.genmove(PWHITE)==0x280020000000);

    Game game;
    game.makemove(pos(5,4));
    CHECK(game.board==board);
    CHECK(board.genmove(PWHITE)==game.genmove());

    //first game
    while (game.hasmove()){
        auto firstmove=*u64iter(game.genmove()).begin();
        game.makemove(firstmove);
        //CHECK(testmoves(game.board, game.col)==game.genmove());
    }
    CHECK(game.board.b[0]==0xffd7ebf3f3fbffffu &&
          game.board.b[1]==0x28140c0c040000u);
    
    //second game
    while (game.step) game.unmakemove();
    while (game.hasmove()){
        auto firstmove=*u64iter(game.genmove()).begin();
        game.makemove(firstmove);
        //CHECK(testmoves(game.board, game.col)==game.genmove());
    }
    CHECK(game.board.b[0]==0x3fb0888090a0c080u &&
          game.board.b[1]==0xc04f777f6f5f3f7fu);

    CHECK(pow3to4(10,59048)==0xFFC00);
    CHECK(pow4to3_10[0xFFC00]==59048);
    CHECK(pow3to4(9,19682)==0x3FE00);
    CHECK(pow4to3_9[0x3FE00]==19682);
    CHECK(pow3to4(9,17222)==0x2AAAA);
    CHECK(pow4to3_9[0x2AAAA]==17222);
    CHECK(pow3to4(9,50)==0x280A0);
    CHECK(pow4to3_9[0x280A0]==50);
    CHECK(pow3to4(9,17010)==0xA0A);
    CHECK(pow4to3_9[0xA0A]==17010);

    //bitwise transform
    x=0x1f1f;
    u64 x_fv=x; flip_v(x_fv);
    CHECK(x_fv==0xf8f8);
    u64 x_r90=x; rotate_r(x_r90);
    CHECK(x_r90==0xc0c0c0c0c0);
    u64 x_l90=x; rotate_l(x_l90);
    CHECK(x_l90==0x303030303000000);

    flip_h(x);flip_h(x_fv);flip_h(x_l90);flip_h(x_r90);
    CHECK(x==0x1f1f000000000000);
    CHECK(x_fv==0xf8f8000000000000);
    CHECK(x_l90==0x0000000303030303);
    CHECK(x_r90==0xc0c0c0c0c0000000);

    board.b[0]=0x1f19156f4d112100; board.b[1]=0xa0e6ea90b2eede9c;
    
    Board b1=board.makemove_r(pos(0,5),PWHITE);
    b1.makemove(pos(0,0),PBLACK);
    b1.makemove(pos(0,1),PWHITE);
    CHECK(search_end2(b1,PBLACK)==16);
    CHECK(search_end2(b1,PWHITE)==-16);
    CHECK(search_exact(5, board, 1, -INF, INF)==-16);

    loadPtnData();
    board.setStart();
    cout<<search_normal(4, board, PBLACK, -INF, INF)<<std::endl;

#ifdef DEBUGTREE
    debug_tree=new DebugTree;
    cout<<"log tree "<<think_choice(board, PBLACK)<<'\n';
    debug_tree->write_html("debugtree.html");
    delete debug_tree; debug_tree=nullptr;
#endif 

    printf("All test passed\n");
    return 0;
}
