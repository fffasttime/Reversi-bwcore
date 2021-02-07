#include "board.h"
#include <iostream>
#include "cui.h"
#include "evalptn.h"
using std::cin; using std::cout;

int main(){
    bitptn::initPtnFlip();
    initPtnConfig();
    loadPtnData();
    runConsole();
    return 0;
}
