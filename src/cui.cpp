#include "cui.h"
#include "search.h"
#include "board.h"
#include "stdio.h"
#include "time.h"
#include <Windows.h>
#include <cstdio>
using namespace std;

#define BSIZE 8

extern HANDLE hOut, hIn;
extern const COORD winsize;

FILE *log_out;

void logprintf(const char *fmt, ...){
	if (log_out==nullptr) return;
	va_list args;
	va_start(args, fmt);
	vfprintf(log_out, fmt, args);
	va_end(args);
	fflush(log_out);
}

struct Ploc{
	int x,y;
	Ploc(){}
	Ploc(int x, int y):x(x),y(y){}
	Ploc(int p){x=p/8; y=p%8;}
	int pos(){return ::pos(x,y);}
	bool inBorder(){return x>=0 && y>=0 && x<BSIZE && y<BSIZE;}
};

HANDLE hOut, hIn;
const COORD winsize = { 80,25 };

int gamemode[2]; // -1 stands for user play
Game game;

void gotoXY(Ploc mp){
	SetConsoleCursorPosition(hOut, { (short)mp.x, (short)mp.y });
}

void minit(){
	HANDLE consolehwnd;
	consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consolehwnd, 127 + 128 - 15);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	SMALL_RECT rc = { 0,0,winsize.X,winsize.Y };
	SetConsoleScreenBufferSize(hOut, winsize);
	SetConsoleWindowInfo(hOut, TRUE, &rc);

	SetConsoleCursorPosition(hOut, { 0,0 });
	for (int i = 0; i<winsize.Y; i++)
		for (int j = 0; j<winsize.X; j++)
			putchar(' ');
	gotoXY({ 30,11 }); printf("黑白棋   1.5");
	Sleep(100);
}

void mexit(){
	CloseHandle(hOut);
	CloseHandle(hIn);
}

Ploc getCurClick(){
	INPUT_RECORD    mouseRec;
	DWORD           res;
	COORD           crPos;
	while (1){
		ReadConsoleInput(hIn, &mouseRec, 1, &res);
		if (mouseRec.EventType == MOUSE_EVENT){
			crPos = mouseRec.Event.MouseEvent.dwMousePosition;
			switch (mouseRec.Event.MouseEvent.dwButtonState){
			case FROM_LEFT_1ST_BUTTON_PRESSED:
				return { crPos.Y, crPos.X };
			}
		}
	}
}

void iPrint(){
	gotoXY({0,0});
	printf("╔"); for (int i = 1; i < BSIZE; i++) printf("═╦"); 
	printf("═╗\n");
	inc(i,BSIZE){
		printf("║");
		inc(j, BSIZE){
			if (game[pos(i,j)] == !game.col) printf("○");
			else if (game[pos(i,j)] == game.col) printf("●");
			else printf("  ");
			printf("║");
		}
		printf("\n");
		if (i < BSIZE - 1){
			printf("╠"); for (int j = 0; j < 7; j++) printf("═╬"); 
			printf("═╣");
		}
		else{
			printf("╚"); for (int j = 0; j < 7; j++) printf("═╩"); 
			printf("═╝");
		}
		printf("\n");
	}
	gotoXY({0, 17});
	printf(" B:%2d W:%2d ", game.cnt(PBLACK), game.cnt(PWHITE));
	if (game.col == PWHITE) printf("current: White\n");
	else printf("current: Black\n");
	gotoXY({0,20});
	printf(" [返回]    [撤销]");
}

Ploc PlocToMloc(const Ploc &p){
	return { p.y * 4 + 2,p.x * 2 + 1 };
}
Ploc MlocToPloc(const Ploc &p){
	if (p.y >= 4 * BSIZE || p.x >= 2 * BSIZE ||
		p.y % 4 == 0 || p.y % 4 == 1 || p.x % 2 == 0){
		if (p.x==20 && p.y>=1 && p.y<=6) return {-1, 0}; // exit
		if (p.x==20 && p.y>=11 && p.y<=16) return {-1, 1}; // unmakemove
		return { -1,-1 };
	}
	return { p.x / 2,p.y / 4 };
}
void showCanDo(){
	for (auto p : u64iter(game.genmove())){
		gotoXY(PlocToMloc(Ploc(p)));
		printf("·");
	}
}

void gameEnd(){
	gotoXY({0,0});
	iPrint();
	gotoXY({0, 17});
	printf(" B:%2d  W:%2d  ", game.cnt(PBLACK), game.cnt(PWHITE));
	if (game.winner()==PWHITE)
		printf(" White Win!  "), logprintf("white win\n");
	else if (game.winner()==PBLACK)
		printf(" Black Win!  "), logprintf("black win\n");
	else
		printf(" game draw..."), logprintf("game draw\n");
	gotoXY({0,20});
	printf(" [返回]");
	while (1){
		Ploc p=getCurClick();
		if (p.x==20 && p.y<8) break;
	}
}

void gamePlay(){
	while (!game.isend()){
		Start:
		int sp;
		if (gamemode[game.col] == -1){
			Ploc p;
			showCanDo();
			do{
				p = MlocToPloc(getCurClick());
				if (p.x==-1 && p.y==1 && game.step){ // unmakemove
					do{
						game.unmakemove();
					}while(gamemode[game.col]!=-1);
					iPrint();
					Sleep(200);
					goto Start;
				}
				else if (p.x==-1 && p.y==0){ // exit
					return;
				}
			}while (!p.inBorder() || !game.testmove(p.pos()));
			sp=p.pos();
		}
		else{
			if (gamemode[game.col] == 0) 
				sp = random_choice(game.board);
			else if (gamemode[game.col] == 1)
				sp = think_choice(game.board);
			else
				sp = think_choice_td(game.board);
			gotoXY({0,18}); inc(i,winsize.X) putchar(' '); 
			gotoXY({1,18}); printf(searchstat.str().c_str());
			logprintf("%s", debugout.str().c_str());
			
		}
		if (gamemode[0]<0 || gamemode[1]<0){
			gotoXY(PlocToMloc(sp));
			if (game.col == PWHITE) printf("○");
			else if (game.col == PBLACK) printf("●");
			Sleep(200);
		}
		logprintf("m %d %d\n", sp/8, sp%8);
		game.makemove(sp);
		logprintf("%s\n%s", game.repr().c_str(), game.str().c_str());

		gotoXY({ 0,0 });
		iPrint();
		if (gamemode[!game.col]==-1) Sleep(200);
	}
	gameEnd();
}

void gameStart(){
	game.reset();
	inc(i, winsize.Y){
		inc(j, winsize.X) putchar(' ');
		puts("");
	}
	iPrint();
	auto t=time(nullptr); logprintf("Game start at %s", ctime(&t));
	logprintf("black mode: %d , white mode: %d\n", gamemode[0], gamemode[1]);
}

void showAbout(){
	for (int i=1;i<winsize.Y-2;i++){
		gotoXY({2,i});
		for (int j=0;j<winsize.X-4;j++) putchar(' ');
	}
	gotoXY({2,1}); printf("Reversi_bwcore 1.5");
	gotoXY({2,3}); printf("Appearance: Console UI");
	gotoXY({2,4}); printf("  (version=1.0.0   date=2016-6-20)");
	gotoXY({2,5}); printf("Kernel: bwcore1.5 ");
	gotoXY({2,6}); printf("  (version=1.5.0   date=2021-2-20)");
	gotoXY({2,8}); printf("Author: fffasttime");
	gotoXY({2,9}); printf("See https://github.com/fffasttime/Reversi-bwcore");
	gotoXY({2,12}); printf("Description: A normal reversi AI");
	gotoXY({42,15}); printf("[back]");
	while (1){
		Ploc p=getCurClick();
		if (p.x==15 && p.y>=42 && p.y<48) break;
	}
}

void writeSelect(int col){
	if (col==PBLACK){
		gotoXY({32,11}); printf("                 ");
		if (gamemode[PBLACK]==-1) {gotoXY({35,11}); printf("黑色：玩家");}
		else{
			gotoXY({32,11}); printf("黑色：电脑");
			switch(gamemode[PBLACK]){
				case 0:printf("(随机)"); break;
				case 1:printf("(深度)"); break;
				case 2:printf("(时限)"); break;
				case 3:printf("(时限)"); break;
			}
		}
	}
	else if (col==PWHITE){
		gotoXY({32,12}); printf("                 ");
		if (gamemode[PWHITE]==-1) {gotoXY({35,12}); printf("白色：玩家");}
		else{
			gotoXY({32,12}); printf("白色：电脑");
			switch(gamemode[PWHITE]){
				case 0:printf("(随机)"); break;
				case 1:printf("(深度)"); break;
				case 2:printf("(时限)"); break;
				case 3:printf("(时限)"); break;
			}
		}
	}
}

int splashScreen(){
	gotoXY({0,0});
	printf("╔"); for (int i = 1; i < winsize.X/2-1; i++) printf("═"); printf("╗\n");
	for (int i=1;i<winsize.Y-2;i++){
		printf("║");
		for (int j=1;j<winsize.X/2-1;j++) printf("  ");
		printf("║\n");
	}
	printf("╚"); for (int i = 1; i < winsize.X/2-1; i++) printf("═"); printf("╝\n");
	gotoXY({30,11}); printf("               ");
	gotoXY({34,8}); printf("黑白棋   1.5");
	gotoXY({36,10}); printf(">>开始<<");
	gotoXY({34,13}); printf("关于    退出");
	gamemode[0]=1; writeSelect(PBLACK);
	gamemode[1]=1; writeSelect(PWHITE);
	gotoXY({0,0});
	while (1){
		Ploc p;
		p = getCurClick();
		if (p.x==10 && p.y>=35 && p.y<45) break;
		if (p.x==11 && p.y>=32 && p.y<48)
		{
			gamemode[PBLACK]++; if (gamemode[PBLACK]>3) gamemode[PBLACK]=-1;
			writeSelect(PBLACK);
		}
		if (p.x==12 && p.y>=32 && p.y<48){
			gamemode[PWHITE]++; if (gamemode[PWHITE]>3) gamemode[PWHITE]=-1;
			writeSelect(PWHITE);
		}
		if (p.x==13 && p.y>=34 && p.y<38){
			showAbout();
			return -1;
		}
		if (p.x==13 && p.y>=42 && p.y<46) return 0;
	}
	SetConsoleCursorPosition(hOut, { 0,0 });
	for (int i=0;i<winsize.Y;i++) 
		for (int j=0;j<winsize.X;j++)
			putchar(' ');
	SetConsoleCursorPosition(hOut, { 0,0 });
	return 1;
}

void runConsole(){
	minit();
	log_out=fopen("bwcore1.5.log", "w");
	auto t=time(nullptr); logprintf("Program start at  %s", ctime(&t));
	while (1){
		int ret=splashScreen();
		if (ret==0) break;
		else if (ret<0) continue;
		gameStart();
		gamePlay();
	}
	fclose(log_out);
	mexit();
}
