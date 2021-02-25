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
    ifstream fin("data/reversi_pc_data.txt");
    u64 b0,b1; float val;
    ofstream fout("data/reversi_pc_ndata.txt");
    int d1,d,cnt,n=0;
    while (fin>>std::hex>>b0>>b1>>std::dec>>val>>d1>>d>>cnt){
        pcnt[cnt][d]++;
        fout<<d<<','<<cnt<<','<<val<<',';
        search_root(d,Board(b0,b1));
        fout<<searchstat.maxv<<'\n';
        n++;
        if (n%10==0) cout<<n<<'\n';
    }
    ofstream flog("data/reversi_pc_ndata.log",std::ios::app);
    inc(i,64){
        flog<<"cnt:"<<i<<" ";
        for (int j=3;j<=MPC_MAXD;j++)
            flog<<pcnt[j]<<" ";
        flog<<'\n';
    }
}

void gendata_endgame(){
    std::string foldername="data/rawdata1/";
    const int p_begins[] = {2,5, 8,11,14,18,22,27,32,38,44};
    const int p_ends[]   = {4,7,10,13,17,21,26,31,37,43,60};
    const int phase=4;
    int p_begin=p_begins[phase];
    int p_end=p_ends[phase];
    ofstream fout(foldername+"data"+std::to_string(p_begin)+"_"+std::to_string(p_end)+".txt", std::ios::app);
    if (!fout.is_open()){
        puts("file doesn't exist");
        exit(1);
    }
    int game_cnt=100000;
    for (int i=0;i<game_cnt;i++) {
        Game game;
        while (!game.isend()){
            int remain=popcnt(game.board.emptys());
            if (remain>=p_begin && remain<=p_end && i%(p_end-p_begin+1)+p_begin==remain){
                search_delta=0;
                think_maxd=12;
                think_choice(game.board);
                int val=searchstat.maxv;
                fout<<game.board.repr()<<' '<<val<<'\n';
                fout.flush();
                break;
            }
            think_maxd=5;
            search_delta=5+game.step/2;
            // [0, n//20] all stronger
            // [n//20, n//10] one side stronger
            if (i<game_cnt/10 && game.col==i%2) search_delta/=4;
            if (i<game_cnt/20 && game.col+1==i%2) search_delta/=4;
            game.makemove(think_choice(game.board));
        }
        if (i%10==0) std::cout<<i<<' ';
    }
}

int main(){
    srand(time(0));
    initPtnConfig();
    loadPtnData();

    //gendata_PC();
    //remakedata_PC();
    gendata_endgame();

    return 0;
}
