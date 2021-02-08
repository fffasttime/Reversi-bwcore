#include "board.h"
#include <iostream>
#include "cui.h"
#include "evalptn.h"
using std::cin; using std::cout;

void global_init(){
    initPtnConfig();
    loadPtnData();
}

int main(){
    global_init();
    runConsole();
    return 0;
}
