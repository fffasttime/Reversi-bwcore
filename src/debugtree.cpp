#include "debugtree.h"
#include <stdio.h>

DebugTree *debug_tree;
void DebugTreeNode::write_board(FILE *out){
    fprintf(out, "<code>");
    int movepos=-1;
    if (fa!=nullptr) movepos=tzcnt(fa->board.occupys()^board.occupys());
    for (int i=0;i<BSIZE2;i++){
        if (i==movepos) fprintf(out, "<font style=\"background: pink\">");
        if (bget(board.b[0]&board.b[1], i)) assert(false, "pos %d has two pieces\n", i);
        else if (bget(board.b[0], i)) fprintf(out,"●");
        else if (bget(board.b[1], i)) fprintf(out,"○");
        else if (board.testmove(i, col)) fprintf(out,"＋");
        else fprintf(out,"&nbsp;");
        if (i==movepos) fprintf(out, "</font>");
        if (i%8==7) fprintf(out,"<br>");
    }
    fprintf(out, "</code>");
}

void DebugTreeNode::write_html(FILE *out, int d, int d_limit){
    if (d==d_limit) return;

    char summary_style[40]="";
    if(ret<=alpha) //cut node
        sprintf(summary_style," style=\"background: %s;\"", "gold");
    fprintf(out,"<details><summary><code%s>",summary_style);

    fprintf(out,"%s(depth:%d, col:%d, alpha:%.2f, beta:%.2f) ch:%2d\n move:%2d  ret:%.2f",
        fun_name.c_str(), depth, col, alpha, beta, (int)ch.size(), (int)popcnt(board.genmove(col)), ret);
    if (popcnt(board.genmove(col))==0) //pass node
        fprintf(out," (pass)");
    fprintf(out,"</code></summary>");
    
    fprintf(out,"<div style=\"margin-left:50px\">\n");
    write_board(out);
    for (auto p:ch) p->write_html(out, d+1, d_limit);
    fprintf(out,"</div></details>\n");
}

void DebugTree::step_in(const std::string &fun_name, int depth, const Board &board, int col, Val alpha, Val beta){
    auto ch=new DebugTreeNode();
    ch->fun_name=fun_name;
    ch->depth=depth;
    ch->board=board;
    ch->col=col;
    ch->alpha=alpha;
    ch->beta=beta;
    ch->fa=cur;
    if (root==nullptr)
        root=ch;
    else
        cur->ch.push_back(ch);
    cur=ch;
}

void DebugTree::write_html(std::string file, int d_limit){
    assert(root!=nullptr, "DebugTree is empty\n");
    FILE *out=fopen(file.c_str(),"w");
    root->write_html(out, 0, d_limit);
    fclose(out);
}