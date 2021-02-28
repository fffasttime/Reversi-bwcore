#include "board.h"
#include <sstream>
#include <iostream>
#include <fstream>

std::string Board::repr() const{
    std::ostringstream o;
    o<<std::hex<<"0x"<<b[0]<<" 0x"<<b[1];
    return o.str();
}
#ifndef ONLINE
std::string Board::str(bool fcol) const{
    std::ostringstream o;
    for (int i=0;i<BSIZE2;i++){
    #ifdef _WIN32
        if (bget(b[0]&b[1], i)) assertprintf(false, "pos %d has two pieces\n", i);
        else if (bget(b[fcol], i)) o<<"\xa1\xf1"; //solid circle
        else if (bget(b[!fcol], i)) o<<"\xa1\xf0"; // hollow circle
        else o<<"\xa1\xa4"; //cdot
    #else
        if (bget(b[0]&b[1], i)) assertprintf(false, "pos %d has two pieces\n", i);
        else if (bget(b[fcol], i)) o<<"\u25cf";  // solid circle
        else if (bget(b[!fcol], i)) o<<"\u25cb"; // hollow circle
        else o<<"\xc2\xb7"; // cdot
    #endif //_WIN32
        if (i%8==7) o<<"\n";
    }
    return o.str();
}
std::string Game::str() const{
    std::ostringstream o;
    o<<board.str(col);
    o<<"step: "<<step<<"  col: "<<col<<"  B: "<<board.cnt0()<<" W: "<<board.cnt1()<<std::endl;
    return o.str();
}

char *pos_str(int p){
    static char ret[]="a0";
    ret[0]=p%8+'a'; ret[1]=p/8+'1';
    return ret;
}

void Game::savesgf(std::string filename){
    std::ofstream fout(filename);
    fout<<"(;GM[2]FF[4]\nSZ[8]\n";
    fout<<"AB";
    for (auto p:u64iter(board_begin().b[0])) fout<<"["<<pos_str(p)<<"]";
    fout<<"\nAW";
    for (auto p:u64iter(board_begin().b[1])) fout<<"["<<pos_str(p)<<"]";
    fout<<"\nPL["<<(col_before[0]?'W':'B')<<"]\n";
    for(int i=0;i<step;i++){
        fout<<";"<<(col_before[0]?'W':'B')<<'['
            <<pos_str(move_before[i])<<"]\n";
    }
    fout<<")\n";
}

#endif //ONLINE

std::string Game::repr() const{
    std::ostringstream o;
    o<<"board "<<board.repr()<<" col "<<col;
    return o.str();
}
