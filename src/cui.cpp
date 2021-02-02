#include "cui.h"
#include "search.h"
#include "board.h"
#include <Windows.h>
#include <cstdio>
using namespace std;

extern HANDLE hOut, hIn;
extern const COORD winsize;

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
	SetConsoleCursorPosition(hOut, { mp.x,mp.y });
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
	gotoXY({ 30,11 }); printf("ºÚ°×Æå   1.3");
	Sleep(100);
}

void mexit(){
	CloseHandle(hOut);
	CloseHandle(hIn);
}

Ploc getCurClick(){
	INPUT_RECORD    mouseRec;
	DWORD           res;
	COORD           crPos, crHome = { 0, 0 };
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
	printf("¨X"); for (int i = 1; i < BSIZE; i++) printf("¨T¨j"); printf("¨T¨[\n");
	inc(i,BSIZE){
		printf("¨U");
		inc(j, BSIZE){
			if (game[pos(i,j)] == PWHITE) printf("¡ð");
			else if (game[pos(i,j)] == PBLACK) printf("¡ñ");
			else printf("  ");
			printf("¨U");
		}
		printf("\n");
		if (i < BSIZE - 1){
			printf("¨d");
			for (int j = 0; j < 7; j++) printf("¨T¨p"); printf("¨T¨g");
		}
		else{
			printf("¨^");
			for (int j = 0; j < 7; j++) printf("¨T¨m"); printf("¨T¨a");
		}
		printf("\n");
	}
}

Ploc PlocToMloc(const Ploc &p){
	return { p.y * 4 + 2,p.x * 2 + 1 };
}
Ploc MlocToPloc(const Ploc &p){
	if (p.y >= 4 * BSIZE || p.x >= 2 * BSIZE ||
		p.y % 4 == 0 || p.y % 4 == 1 || p.x % 2 == 0) return { -1,-1 };
	return { p.x / 2,p.y / 4 };
}
void showCanDo(){
	for (auto p : u64iter(game.genmove())){
		gotoXY(PlocToMloc(Ploc(p)));
		printf("¡¤");
	}
}

void gameEnd()
{
	gotoXY({0,0});
	iPrint();
	printf("B:%2d  W:%2d\n", game.cnt(PBLACK), game.cnt(PWHITE));
	if (game.winner()==PWHITE)
		printf(" Winner:White");
	else if (game.winner()==PBLACK)
		printf(" Winner:Black");
	else
		printf(" Equal");
	gotoXY({0,20});
	printf("[·µ»Ø]");
	while (1){
		Ploc p=getCurClick();
		if (p.x==20 && p.y<8) break;
	}
}

void gamePlay(){
	while (1){
		printf("B:%2d  W:%2d\n", game.cnt(PBLACK), game.cnt(PWHITE));
		if (game.col == PWHITE) printf("now:White");
		else printf("now:Black");
		int sp;
		if (gamemode[game.col] == -1){
			sp = MlocToPloc(getCurClick()).pos();
			bool flag = false;
			while (!Ploc(sp).inBorder() || !game.testmove(sp))
			{
				if (!flag) showCanDo(), 
				flag = true;
				sp = MlocToPloc(getCurClick()).pos();
			}
		}
		else{
			sp=randomPolicy(game.board, game.col);
			Sleep(1);
		}
		game.makemove(sp);
		gotoXY(PlocToMloc(sp));
		if (game.col == PWHITE) printf("¡ð");
		else if (game.col == PWHITE) printf("¡ñ");
		gotoXY({ 0,0 });
		iPrint();
	}
}

void gameStart(){
	game.reset();
	iPrint();
}

void showAbout(){
	for (int i=1;i<winsize.Y-2;i++)	{
		gotoXY({2,i});
		for (int j=0;j<winsize.X-4;j++) putchar(' ');
	}
	gotoXY({2,1}); printf("BlackWhiteChess(Reversi) 1.3 improve0");
	gotoXY({2,3}); printf("Appearance: Console UI");
	gotoXY({2,4}); printf("  (version=1.0   date=2016-6-20) fffasttime");
	gotoXY({2,6}); printf("Kernel: bwcore1.2_i4 ");
	gotoXY({2,7}); printf("  (version=1.3.0   date=2016-7-10) fffasttime");
	gotoXY({2,8}); printf("  Description: a weak reversi AI");
	gotoXY({36,10}); printf("[back]");
	while (1){
		Ploc p=getCurClick();
		if (p.x==10 && p.y>=36 && p.y<42) break;
	}
}

void writeSelect(int col){
	if (col==PBLACK){
		gotoXY({32,11}); printf("                ");
		if (gamemode[PBLACK]==-1) {gotoXY({35,11}); printf("ºÚÉ«£ºÍæ¼Ò");}
		else{
			gotoXY({32,11}); printf("ºÚÉ«£ºµçÄÔ");
			switch(gamemode[PBLACK]){
				case 0:printf("(²âÊÔ1)"); break;
				case 1:printf("(²âÊÔ2)"); break;
				case 2:printf("(²âÊÔ3)"); break;
				case 3:printf("(²âÊÔ4)"); break;
			}
		}
	}
	else if (col==PWHITE){
		gotoXY({32,12}); printf("                 ");
		if (gamemode[PWHITE]==-1) {gotoXY({35,12}); printf("°×É«£ºÍæ¼Ò");}
		else{
			gotoXY({32,12}); printf("°×É«£ºµçÄÔ");
			switch(gamemode[PWHITE]){
				case 0:printf("(²âÊÔ1)"); break;
				case 1:printf("(²âÊÔ2)"); break;
				case 2:printf("(²âÊÔ3)"); break;
				case 3:printf("(²âÊÔ4)"); break;
			}
		}
	}
}

int splashScreen()
{
	gotoXY({0,0});
	printf("¨X"); for (int i = 1; i < winsize.X/2-1; i++) printf("¨T"); printf("¨[");
	for (int i=1;i<winsize.Y-2;i++){
		printf("¨U");
		for (int j=1;j<winsize.X/2-1;j++) printf("  ");
		printf("¨U");
	}
	printf("¨^"); for (int i = 1; i < winsize.X/2-1; i++) printf("¨T"); printf("¨a");
	gotoXY({30,11}); printf("               ");
	gotoXY({32,8}); printf("ºÚ°×Æå   1.3");
	gotoXY({36,10}); printf(">>¿ªÊ¼<<");
	gotoXY({34,13}); printf("¹ØÓÚ    ÍË³ö");
	gamemode[0]=-1; writeSelect(PBLACK);
	gamemode[1]=-1; writeSelect(PWHITE);
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
	while (1){
		int ret=splashScreen();
		if (ret==0) break;
		else if (ret<0) continue;
		gameStart();
		gamePlay();
		gameEnd();
	}
	mexit();
}
