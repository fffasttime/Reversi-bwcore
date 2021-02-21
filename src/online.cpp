#include "board.h"
#include "search.h"
#include "evalptn.h"
#include "time.h"
#include <stdio.h>

int main(){
    srand(time(nullptr));
    initPtnConfig();
    loadPtnData();
    loadPCData();
    Game game;
#if 0
    int n,x,y; scanf("%d", &n);
    inc(i,2*n-1){
        scanf("%d%d",&x,&y); 
        if (x!=-1) game.makemove(pos(x,y), 0); // no auto pass
        else if (i) game.col=!game.col; // opp pass
    }
    if (game.hasmove()){
        int sp=think_choice(game.board, game.col);
        printf("%d %d\n", sp/8, sp%8);
    }
    else puts("-1 -1");
    //debug
    printf("%s, %s\n", game.repr().c_str(), searchstat.str().c_str());
#else // long time running
    int n,x,y; scanf("%d", &n); // skip
    for(n=0;;n++){
        scanf("%d%d", &x, &y);
        if (x!=-1) game.makemove(pos(x,y), 0);
        else if (n) game.col=!game.col, game.board.cswap();
        if (game.hasmove()){
            int sp=think_choice_td(game.board);
            printf("%d %d\n", sp/8, sp%8);
            printf("%s, %s, st:%d, scnt: %d\n", game.repr().c_str(), searchstat.str().c_str(),
                searchstat_sum.tl, searchstat_sum.leafcnt);
            game.makemove(sp, 0);
        }
        else puts("-1 -1"),puts(""), game.col=!game.col, game.board.cswap();
        //debug
        puts(""); // data
        puts(""); // global data
        printf(">>>BOTZONE_REQUEST_KEEP_RUNNING<<<\n");
        fflush(stdout);
    }
#endif
    return 0;
}
