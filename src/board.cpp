#include "board.h"
#include <sstream>
#include <iostream>

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
std::string Game::str(){
    std::ostringstream o;
    o<<board.str(col);
    o<<"step: "<<step<<"  col: "<<col<<"  B: "<<board.cnt0()<<" W: "<<board.cnt1()<<std::endl;
    return o.str();
}
#endif //ONLINE

std::string Game::repr(){
    std::ostringstream o;
    o<<"board "<<board.repr()<<" col "<<col;
    return o.str();
}
