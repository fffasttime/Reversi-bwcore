#include "board.h"
#include <iostream>
#include "cui.h"
#include "search.h"
#include "evalptn.h"
using std::cin; using std::cout;

void global_init(){
    initPtnConfig();
    loadPtnData();
    loadPCData();
}

int main(){
    srand(0);
    global_init();
    search_delta=0.5;
    think_maxd=11;
    runConsole();
    return 0;
}
