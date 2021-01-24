#include "board.h"
#include <sstream>
#include <iostream>

std::string board::repr(){
    std::ostringstream o;
    o<<std::hex<<b[0]<<' '<<b[1]<<'\n';
    return o.str();
}

std::string board::str(){
    std::ostringstream o;
    for (int i=0;i<BSIZE2;i++){
        if (bget(b[0]&b[1], i)) abort();
        else if (bget(b[0])) o<<"\u25cf";
        else if (bget(b[1])) o<<"\u25cb";
        else o<<"  ";
        if (i%8==7) o<<"\n";
    }
    return o.str();
}

void print(){
    std::cout<<board.str();
    std::cout<<"step: "<<step<<"  col: "<<col<<"  B: "<<board.cnt(0)<<"  W: "<<board.cnt(1)<<std::endl;
}
