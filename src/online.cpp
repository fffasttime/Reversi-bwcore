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
        scanf("%d%d",&x,&y); 
        if (x!=-1) game.makemove(pos(x,y), 0);
        else if (i) game.col=!game.col;
    }
    if (game.hasmove()){
        int sp=think_choice(game.board, game.col);
        printf("%d %d\n", sp/8, sp%8);
    }
    else puts("-1 -1");

    //debug
    printf("%s, %s\n", game.repr().c_str(), searchstat.str().c_str());

    return 0;
}
