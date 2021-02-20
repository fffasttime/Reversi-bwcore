#include <iostream>
#include <string>
#include "util.h"
#include "board.h"
using std::cin, std::cout, std::endl;

int main(){
    Game game;
    cout<<"request\n-1 -1"<<endl;
    while (1){
        int x,y; cin>>x>>y;
        if (x==-1 && y==-1){
            if (game.hasmove()){
                cout<<"finish\n"<<!game.col<<'\n'<<"worngly pass turn"<<endl;
                return 0;
            }
            game.col=!game.col; game.board.cswap();
        }
        else{
            if (x<0 || x>=8 || y<0 || y>=8 || !game.testmove(pos(x,y))){
                cout<<"finish\n"<<!game.col<<'\n'<<"invalid move at "<<x<<','<<y<<endl;
                return 0;
            }
            game.makemove(pos(x,y), false);
            if (!game.hasmove() && !game.board.genmove<1>()){
                cout<<"finish\n"<<game.winner()<<'\n'
                    <<"game end, B:"<<game.cnt(0)<<",W:"<<game.cnt(1)<<endl;
                return 0;
            }
        }
        cout<<"request\n"<<x<<' '<<y<<endl;
    }
    return 0;
}
