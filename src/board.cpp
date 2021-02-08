#include "board.h"
#include <sstream>
#include <iostream>

std::string Board::repr() const{
    std::ostringstream o;
    o<<std::hex<<"0x"<<b[0]<<" 0x"<<b[1]<<'\n';
    return o.str();
}

std::string Board::str() const{
    std::ostringstream o;
    for (int i=0;i<BSIZE2;i++){
        if (bget(b[0]&b[1], i)) assert(false, "pos %d has two pieces\n", i);
        else if (bget(b[0], i)) o<<"¡ñ";
        else if (bget(b[1], i)) o<<"¡ð";
        else o<<"¡¤";
        if (i%8==7) o<<"\n";
    }
    return o.str();
}

void Game::print(){
    std::cout<<board.str();
    std::cout<<"step: "<<step<<"  col: "<<col<<"  B: "<<board.cnt(0)<<" W: "<<board.cnt(1)<<std::endl;
}

std::string Game::str(){
    std::ostringstream o;
    o<<board.str();
    o<<"step: "<<step<<"  col: "<<col<<"  B: "<<board.cnt(0)<<" W: "<<board.cnt(1)<<std::endl;
    return o.str();
}
