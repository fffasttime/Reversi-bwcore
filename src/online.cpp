#include "board.h"
#include "search.h"
#include "evalptn.h"
#include "time.h"
#include <stdio.h>

void global_init(){
    initPtnConfig();
    loadPtnData();
}
int main(){
    global_init();
    Game game;
    int n,x,y; scanf("%d", &n);
    inc(i,2*n-1){
        scanf("%d%d",&y,&x); 
        if (x!=-1) game.makemove(pos(x,y), 0);
        else if (i) game.col=!game.col;
    }
    clock_t tstart=clock();
    if (game.hasmove()){
        int sp=random_choice(game.board, game.col);
        printf("%d %d\n", sp%8, sp/8);
    }
    else puts("-1 -1");

    //debug
    printf("board: %s, col: %d, tt: %d", game.board.repr().c_str(), game.col, clock()-tstart);

    return 0;
}
