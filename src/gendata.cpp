#include "util.h"
#include "board.h"
#include "evalptn.h"
#include "search.h"
#include <iostream>
#include <fstream>
using std::cin, std::cout;
using std::ifstream, std::ofstream;

void gendata_PC(){
    int n; cout<<"n_games(10-100): "; cin>>n;
    Game game;
    think_checktime=6000;
    think_maxtime=15000;
    search_delta=4;
    for (int i=0;i<n;i++){
        game.reset();
        cout<<"Game "<<i<<'\n';
        while (game.hasmove()){
            int mv=think_choice_td(game.board);
            game.makemove(mv);
            cout<<mv<<' ';
        }
        cout<<'\n';
    }
}

int pcnt[64][MPC_MAXD+1];
void remakedata_PC(){
    ifstream fin("data/pc_data.txt");
    u64 b0,b1; float val;
    ofstream fout("data/pc_ndata.txt");
    int d1,d,cnt,n=0;
    while (fin>>std::hex>>b0>>b1>>std::dec>>val>>d1>>d>>cnt){
        pcnt[cnt][d]++;
        fout<<d<<','<<cnt<<','<<val<<',';
        search_root(d,Board(b0,b1));
        fout<<searchstat.maxv<<'\n';
        n++;
        if (n%10==0) cout<<n<<'\n';
    }
    ofstream flog("data/pc_ndata.log",std::ios::app);
    inc(i,64){
        flog<<"cnt:"<<i<<" ";
        for (int j=3;j<=MPC_MAXD;j++)
            flog<<pcnt[j]<<" ";
        flog<<'\n';
    }
}

int main(){
    srand(time(0));
    initPtnConfig();
    loadPtnData();

    //gendata_PC();
    remakedata_PC();
}
