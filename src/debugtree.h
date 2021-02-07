#pragma once

#define DEBUGTREE_WARPPER_BEGIN \
if (debug_tree)\
debug_tree->step_in(__func__,depth, cboard, col, alpha, beta);\
auto warpper=[&]()->Val{
#define DEBUGTREE_WARPPER_END \
}; auto ret=warpper(); if (debug_tree) debug_tree->step_out(ret); return ret;

#include "board.h"
#include <vector>

typedef float Val;

struct DebugTreeNode{
    std::string fun_name;
    Board board;
    int depth;
    int col;
    Val alpha, beta;
    Val ret;
    std::vector<DebugTreeNode*> ch;
    DebugTreeNode* fa;
    DebugTreeNode(){}
    ~DebugTreeNode(){for(auto p:ch) delete p;}
    void write_board(FILE *out);
    void write_html(FILE *out, int d, int d_limit);
};
struct DebugTree{
    DebugTreeNode *root;
    DebugTreeNode *cur;
    void step_in(const std::string &fun_name, int depth, const Board &board, int col, Val alpha, Val beta);
    void step_out(Val ret){
        cur->ret=ret;
        cur=cur->fa;
    }
    DebugTree(){
        root=cur=nullptr;
    }
    ~DebugTree(){delete root;}
    void write_html(std::string file, int d_limit=5);
};
extern DebugTree *debug_tree;
