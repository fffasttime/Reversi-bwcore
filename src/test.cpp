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

int runTests(){
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

    board.setStart();
    cout<<search_normal(4, board, PBLACK, -INF, INF)<<std::endl;

    CHECK(search_end<4>(Board(0x3160756b5d257f01,0xa9e8a94a2da80fe), PBLACK, -INF, INF, false)==-6);

    CHECK(think_choice(Board(0x3160752b1d051f01,0xa9e8ad4e2fa20fe), PBLACK)==58);

#ifdef DEBUGTREE
    board.b[0]=0x3160756b5d257f01;
    board.b[1]=0xa9e8a94a2da80fe;
    debug_tree=new DebugTree;
    cout<<"log tree "<<search_end<4>(board, PBLACK, -INF, INF, false)<<'\n';
    debug_tree->write_html("debugtree.html", 6);
    delete debug_tree; debug_tree=nullptr;
#endif 

    printf("All test passed\n");
    return 0;
}

void global_init(){
    initPtnConfig();
    loadPtnData();
}

void runDebugMode(){
    Game game;
    std::string cmd;
    cout<<"> ";
    bool echo=true;
    auto displayGame=[&](){
        cout<<game.str();
    };
    while (std::cin>>cmd){
        if (cmd=="p" || cmd=="print") displayGame();
        else if (cmd=="ph" || cmd=="printhex") cout<<game.board.repr()<<'\n';
        else if (cmd=="q" || cmd=="quit" || cmd=="exit") return;
        else if (cmd=="m" || cmd=="makemove"){
            int x, y; std::cin>>x>>y;
            if (game.testmove(pos(x,y))){
                game.makemove(pos(x,y));
                if (echo) displayGame();
            }
            else
                puts("invalid move");
        }
        else if (cmd=="u" || cmd=="undo" || cmd=="unmakemove"){
            if (game.step){
                game.unmakemove();
                if (echo) displayGame();
            }
            else puts("invalid");
        }
        else if (cmd=="echo"){
            echo=!echo;
            printf("echo %s\n", echo?"on":"off");
        }
        else if (cmd=="evalptn"){
            printf("evalptn: %f\n", evalPtn(game.board, game.col)/256.0);
        }
        else if (cmd=="tree"){
        #ifdef DEBUGTREE
            debug_tree=new DebugTree;
            if (game.isend()) puts("no move");
            else{
                int p=think_choice(game.board, game.col);
                printf("%d %d\n", p/8, p%8);
            }
            debug_tree->write_html("debugtree.html", 6);
            delete debug_tree; debug_tree=nullptr;
        #else
            printf("macro DEBUGTREE hasn't defined\n");
        #endif
        }
        else if (cmd=="think"){
            if (game.isend()) puts("no move");
            else{
                int p=think_choice(game.board, game.col);
                printf("%d %d\n", p/8, p%8);
                printf("%s\n",searchstat.str().c_str());
            }
        }
        else if (cmd=="play"){
            if (game.isend()) puts("no move");
            else{
                int p=think_choice(game.board, game.col);
                game.makemove(p);
                if (echo) displayGame();
            }
        }
        else if (cmd=="cnt" || cmd=="count"){
            printf("B:%2d  W:%2d\n", game.cnt(0), game.cnt(1));
        }
        else if (cmd=="ld" || cmd=="board"){
            u64 x, y;
            std::cin>>std::hex>>x>>y;
            if (x&y){
                puts("invalid");
            }
            else{
                game.reset();
                game.board.b[0]=x;
                game.board.b[1]=y;
                if (echo) displayGame();
            }
        }
        else if (cmd=="flipv"){
            game.board.flip_v();
            if (echo) displayGame();
        }
        else if (cmd=="fliph"){
            game.board.flip_h();
            if (echo) displayGame();
        }
        else if (cmd=="rotatel"){
            game.board.rotate_l();
            if (echo) displayGame();
        }
        else if (cmd=="rotater"){
            game.board.rotate_r();
            if (echo) displayGame();
        }
        else if (cmd=="cswap"){
            game.board.cswap();
            if (echo) displayGame();
        }
        else if (cmd=="fcol"){
            game.col=!game.col;
            printf("now col: %d\n",game.col);
        }
        else if (cmd=="col"){
            std::cin>>game.col;
        }
        else if (cmd=="reset"){
            game.reset();
            if (echo) displayGame();
        }
        else{
            puts("unknown command");
        }
        cout<<"> ";
    }
}

int main(int argc, char **argv){
    global_init();
    if (argc==2)
        runTests();
    else
        runDebugMode();
    return 0;
}