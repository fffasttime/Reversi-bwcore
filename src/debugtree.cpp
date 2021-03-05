#include "debugtree.h"
#include <stdio.h>

DebugTree *debug_tree;
void DebugTreeNode::write_board(FILE *out){
    fprintf(out, "<code>");
    int movepos=-1;
    if (fa!=nullptr) movepos=tzcnt(fa->board.occupys()^board.occupys());
    inc(i,64){
        if (i==movepos) fprintf(out, "<font style='background: pink'>");
        if (bget(board.b[0]&board.b[1], i)) assertprintf(false, "pos %d has two pieces\n", i);
        else if (bget(board.b[0], i)) fprintf(out,"¡ñ");
        else if (bget(board.b[1], i)) fprintf(out,"¡ð");
        else if (board.testmove(i)) fprintf(out,"£«");
        else fprintf(out,"&nbsp;");
        if (i==movepos) fprintf(out, "</font>");
        if (i%8==7){
            fprintf(out, "|");
            if (i/8==2) fprintf(out, "   board %s ", board.repr().c_str());
            fprintf(out,"<br>");
        }
    }
    fprintf(out, "</code>");
}

void DebugTreeNode::write_html(FILE *out, int d, int d_limit){
    if (d==d_limit) return;

    fprintf(out,"<details><summary><code>");

    fprintf(out,"%s(depth:%d, alpha:%.2f, beta:%.2f) -ret:",
        fun_name.c_str(), depth, alpha, beta);

    if (ret<=alpha) fprintf(out, "<font style='background: gold'>");
    else if (ret>=beta) fprintf(out, "<font style='background: lightblue'>");
    fprintf(out, "%.2f", -ret);
    if (ret<=alpha || ret>=beta) fprintf(out,"</font>");

    fprintf(out," move:%2d ch:%2d", (int)popcnt(board.genmove()), (int)ch.size());

    if (popcnt(board.genmove())==0) //pass node
        fprintf(out," <font style='background: lightgray'>(pass)</font>");

    fprintf(out,"</code></summary>");
    
    fprintf(out,"<div style=\"margin-left:50px\">\n");
    write_board(out);
    for (auto p:ch) p->write_html(out, d+1, d_limit);
    fprintf(out,"</div></details>\n");
}

void DebugTree::step_in(const std::string &fun_name, int depth, const Board &board, Val alpha, Val beta){
    auto ch=new DebugTreeNode();
    ch->fun_name=fun_name;
    ch->depth=depth;
    ch->board=board;
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
    assertprintf(root!=nullptr, "DebugTree is empty\n");
    FILE *out=fopen(file.c_str(),"w");
    root->write_html(out, 0, d_limit);
    fclose(out);
}
