#include "board.h"
#include <iostream>
#include "cui.h"
#include "evalptn.h"
using std::cin; using std::cout;

void global_init(){
    bitptn::initPtnFlip();
    initPtnConfig();
    loadPtnData();
}

int main(){
    global_init();
    runConsole();
    return 0;
}
