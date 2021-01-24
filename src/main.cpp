#include "board.h"
#include <iostream>
using std::cin; using std::cout;

int main(){
    Game game;
    while (1){
        char c; int x,y;
        cin>>c;
        if (c=='s'){
            showMask(game.genmove());
        }
        else{
            if (c=='m'){
                int x,y; cin>>x>>y;
                game.makemove(x,y);
            }
            else if (c=='u') game.unmakemove();
            game.print();
        }
    }
}
