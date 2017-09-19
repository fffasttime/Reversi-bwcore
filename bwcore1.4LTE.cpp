#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <vector>
#include <algorithm>
#include <windows.h>
//#include "jsoncpp/json.h"
using namespace std;

#define LOG_PRINT

stringstream fout;
#ifdef LOG_PRINT
ofstream filelog("bwcore.log");
#endif
void logRefrsh()
{
	string s;
#ifdef LOG_PRINT
	while (!fout.eof())
	{
		getline(fout,s);
		filelog<<s<<'\n';
	}
	fout.clear();
#endif
}

typedef long long ll;
typedef unsigned long long ull;
typedef char Bit;

ull popcount(ull x)
{
    x = (x & 0x5555555555555555ull) + ((x >> 1) & 0x5555555555555555ull);
    x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
    x = (x & 0x0F0F0F0F0F0F0F0Full) + ((x >> 4) & 0x0F0F0F0F0F0F0F0Full);
    x = (x & 0x00FF00FF00FF00FFull) + ((x >> 8) & 0x00FF00FF00FF00FFull);
    x = (x & 0x0000FFFF0000FFFFull) + ((x >> 16) & 0x0000FFFF0000FFFFull);
    x = (x & 0x00000000FFFFFFFFull) + ((x >> 32) & 0x00000000FFFFFFFFull);
    return x;
}

inline void setbit(ull &x,ull c,Bit p)
{
	x=(x&~(1ull<<p))|(c<<p);
}

inline ull lowbit(ull x)
{
	return x&(-x);
}

/*
bwcore1.3
made by fffasttime
start at 2016/7/7
last modify to 2016/7/15
a naive black_white chess AI
Maxminsearch
*/
namespace bwcore
{

#define FLOAT_INF (1e10)

typedef Bit Col;
#define C_E 0
#define C_W 1
#define C_B 2

inline Col col_f(Col col) {
	return col%2+1;
}

/*
 0  1  2  3  4  5  6  7
 8  9 10 11 12 13 14 15
16 17 18 19 20 21 22 23
24 25 26 27 28 29 30 31
32 33 34 35 36 37 38 39
40 41 42 43 44 45 46 47
48 49 50 51 52 53 54 55
56 57 58 59 60 61 62 63
*/
typedef Bit MP;
const MP MP_F=0,MP_E=64;
const MP MP_UL=-9,MP_U=-8,MP_UR=-7,MP_L=-1,MP_R=1,MP_DL=7,MP_D=8,MP_DR=9;

struct Ploc
{
	Bit x,y;
	Ploc(){}
	Ploc(MP mp){
		x=mp/8; y=mp%8;
	}
	Ploc(Bit _x, Bit _y):x(_x),y(_y){}
	MP toMP(){
		return x*8+y;
	}
	Ploc& operator+=(const Ploc &p1)
	{
		x += p1.x;
		y += p1.y;
		return *this;
	}
	Ploc operator+(const Ploc &p1) const
	{
		return Ploc(x + p1.x,y + p1.y);
	}
	Ploc& operator++()
	{
		if (y==8) x++;
		else y++;
		return *this;
	}
	Ploc& operator-=(const Ploc &p1)
	{
		x -= p1.x; y -= p1.y;
		return *this;
	}
	Ploc operator-(const Ploc &p1) const
	{
		return Ploc(x - p1.x,y - p1.y);
	}
	bool operator==(const Ploc &p) const
	{
		return x == p.x && y == p.y;
	}
	bool operator!=(const Ploc &p) const
	{
		return x != p.x || y != p.y;
	}
	friend std::ostream& operator<<(std::ostream& i, const Ploc &v)
	{
		i << '<' << (int)v.x << ',' << (int)v.y << '>';
		return i;
	}
	bool inBorder()
	{
		return x >= 0 && x < 8 && y >= 0 && y < 8;
	}
};

#define MC_W 0
#define MC_B 1
typedef ull Map_0;
class Map
{
public:
	//m[0] w; m[1] b
	Map_0 m[2];
	Map(){}
	Map(Col _m[64])
	{
		clear();
		for (int i=0;i<64;i++)
		{
			if (_m[i]==C_W) m[0]|=1ull<<i;
			else if (_m[i]==C_B) m[1]|=1ull<<i;
		}
	}
	void clear(){
		m[0]=m[1]=0;
	}
	inline Col operator[](MP p){
		return ((m[0]>>p)&1) + ((m[1]>>p)&1) * 2;
	}
	inline Col operator[](Ploc p){
		return (*this)[p.toMP()];
	}
	inline bool operator==(Map &op)
	{
		return m[0]==op.m[0] && m[1]==op.m[1];
	}
	inline bool isPlaced(MP p){
		return ((m[0]|m[1])>>p)&1;
	}
	inline void set(MP p, Col col){
		setbit(m[0],col==C_W,p);
		setbit(m[1],col==C_B,p);
	}
	string toString()
	{
		string s;
		for (auto p=MP_F;p<MP_E;p++)
		{
			s+=(*this)[p]+'0';
			s+=' ';
			if (p%8==7) s=s+"\n";
		}
		return s;
	}
	ull hashcode(){
		return (m[0])*(m[1]);
	}
	bool testPiece(MP p, Col col);
	void setPiece(MP p, Col col);
	bool testAll(Col col);
	void countPiece(Bit cnt[3]);
	void getCanDoList(Col col, vector<MP> &pList);
	Bit getFlipCount(MP p,Col col);
	
	static const Map Map_Start;
};

Col MP_SS[64]=
{
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,1,2,0,0,0,
0,0,0,2,1,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0
};
const Map Map::Map_Start=Map(MP_SS);

void Map::getCanDoList(Col col, vector<MP> &pList)
{
	for (auto p=MP_F;p<MP_E;p++)
		if (testPiece(p, col))
			pList.push_back(p);
}

/*
bool Map::testPiece()
{
	
}

void Map::setPiece(MP p, Col col)
{
	
}*/

//L test: tp%8==0
//U test: tp<8
//R test: tp%8==7
//D test: tp>56

//shoule keep the postion is empty
//test if a postion can put piece

bool Map::testPiece(MP p,Col col)
{
	if (isPlaced(p)) return false;
	MP tp; auto co1=col_f(col);
	//1:UL
	if (p%8>1 && p>=16 && (*this)[p+MP_UL]==co1)
		for (tp=p+MP_UL*2;;tp+=MP_UL){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp%8==0 || tp<8) break;
		}
	//2:U
	if (p>=16 && (*this)[p+MP_U]==co1)
		for (tp=p+MP_U*2;;tp+=MP_U){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp<8) break;
		}
	//3:UR
	if (p%8<6 && p>=16 && (*this)[p+MP_UR]==co1)
		for (tp=p+MP_UR*2;;tp+=MP_UR){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp%8==7|| tp<8) break;
		}
	//4:L
	if (p%8>1 && (*this)[p+MP_L]==co1)
		for (tp=p+MP_L*2;;tp+=MP_L){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp%8==0) break;
		}
	//5:R
	if (p%8<6 && (*this)[p+MP_R]==co1)
		for (tp=p+MP_R*2;;tp+=MP_R){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp%8==7) break;
		}
	//5:DL
	if (p%8>1 && p<48 && (*this)[p+MP_DL]==co1)
		for (tp=p+MP_DL*2;;tp+=MP_DL){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp%8==0 || tp>=56) break;
		}
	//6:D
	if (p<48 && (*this)[p+MP_D]==co1)
		for (tp=p+MP_D*2;;tp+=MP_D){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp>=56) break;
		}
	//7:DR
	if (p%8<6 && p<48 && (*this)[p+MP_DR]==co1)
		for (tp=p+MP_DR*2;;tp+=MP_DR){
			Col cc=(*this)[tp];
			if (cc==col) return true;
			else if (cc==C_E) break;
			if (tp%8==7 || tp>=56) break;
		}
	return false;
}

Bit Map::getFlipCount(MP p,Col col)
{
	if (isPlaced(p)) return 0;
	MP tp; auto co1=col_f(col);
	Bit ans=0;
	//1:UL
	if (p%8>1 && p>=16 && (*this)[p+MP_UL]==co1)
		for (tp=p+MP_UL*2;;tp+=MP_UL){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_UL-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==0 || tp<8) break;
		}
	//2:U
	if (p>=16 && (*this)[p+MP_U]==co1)
		for (tp=p+MP_U*2;;tp+=MP_U){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_U-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp<8) break;
		}
	//3:UR
	if (p%8<6 && p>=16 && (*this)[p+MP_UR]==co1)
		for (tp=p+MP_UR*2;;tp+=MP_UR){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_UR-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==7|| tp<8) break;
		}
	//4:L
	if (p%8>1 && (*this)[p+MP_L]==co1)
		for (tp=p+MP_L*2;;tp+=MP_L){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_L-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==0) break;
		}
	//5:R
	if (p%8<6 && (*this)[p+MP_R]==co1)
		for (tp=p+MP_R*2;;tp+=MP_R){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_R-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==7) break;
		}
	//6:DL
	if (p%8>1 && p<48 && (*this)[p+MP_DL]==co1)
		for (tp=p+MP_DL*2;;tp+=MP_DL){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_DL-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==0 || tp>=56) break;
		}
	//7:D
	if (p<48 && (*this)[p+MP_D]==co1)
		for (tp=p+MP_D*2;;tp+=MP_D){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_D-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp>=56) break;
		}
	//8:DR
	if (p%8<6 && p<48 && (*this)[p+MP_DR]==co1)
		for (tp=p+MP_DR*2;;tp+=MP_DR){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				ans+=(tp-p)/MP_DR-1;
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==7 || tp>=56) break;
		}
	return ans;
}

void Map::setPiece(MP p, Col col)
{
	MP tp; auto co1=col_f(col);
	set(p,col);
	//1:UL
	if (p%8>1 && p>=16 && (*this)[p+MP_UL]==co1)
		for (tp=p+MP_UL*2;;tp+=MP_UL){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_UL;tp!=p;tp-=MP_UL) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==0 || tp<8) break;
		}
	//2:U
	if (p>=16 && (*this)[p+MP_U]==co1)
		for (tp=p+MP_U*2;;tp+=MP_U){
			Col cc=(*this)[tp];
			if (cc==col) 
			{
				for (tp-=MP_U;tp!=p;tp-=MP_U) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp<8) break;
		}
	//3:UR
	if (p%8<6 && p>=16 && (*this)[p+MP_UR]==co1)
		for (tp=p+MP_UR*2;;tp+=MP_UR){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_UR;tp!=p;tp-=MP_UR) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==7|| tp<8) break;
		}
	//4:L
	if (p%8>1 && (*this)[p+MP_L]==co1)
		for (tp=p+MP_L*2;;tp+=MP_L){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_L;tp!=p;tp-=MP_L) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==0) break;
		}
	//5:R
	if (p%8<6 && (*this)[p+MP_R]==co1)
		for (tp=p+MP_R*2;;tp+=MP_R){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_R;tp!=p;tp-=MP_R) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==7) break;
		}
	//6:DL
	if (p%8>1 && p<48 && (*this)[p+MP_DL]==co1)
		for (tp=p+MP_DL*2;;tp+=MP_DL){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_DL;tp!=p;tp-=MP_DL) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==0 || tp>=56) break;
		}
	//7:D
	if (p<48 && (*this)[p+MP_D]==co1)
		for (tp=p+MP_D*2;;tp+=MP_D){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_D;tp!=p;tp-=MP_D) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp>=56) break;
		}
	//8:DR
	if (p%8<6 && p<48 && (*this)[p+MP_DR]==co1)
		for (tp=p+MP_DR*2;;tp+=MP_DR){
			Col cc=(*this)[tp];
			if (cc==col)
			{
				for (tp-=MP_DR;tp!=p;tp-=MP_DR) (*this).set(tp, col);
				break;
			}
			else if (cc==C_E) break;
			if (tp%8==7 || tp>=56) break;
		}
}

bool Map::testAll(Col col)
{
	for (auto p=MP_F;p<MP_E;p++)
		if (testPiece(p, col)) return true;
	return false;
}

void Map::countPiece(Bit cnt[3])
{
	cnt[C_W]=popcount(m[MC_W]);
	cnt[C_B]=popcount(m[MC_B]);
	cnt[C_E]=64-cnt[C_W]-cnt[C_B];
}

template <typename Type,typename Data>
class Cache
{
public:
	struct Node
	{
		Type s;
		Data data;
	};
private:
	static const int P=999983, MAX_TB=2000000;
	int size;
	Node *tbl;
public:
	Cache(int _size)
	{
		size=_size;
		assert(size>0);
		if (size>MAX_TB) size=MAX_TB;
		tbl=new Node[size];
	}
	~Cache()
	{
		delete[] tbl;
	}
	Node& find(ull hashcode)
	{
		hashcode%=P;
		return tbl[hashcode];
	}
	void clear()
	{
		memset(tbl,0,sizeof(Node)*size);
	}
};

#define DEBUG_TREE_FILE "debugtreeinfo.txt"
	struct Tree
	{
		struct Node
		{
			float val; int fa; int p;
			vector<int> child;
			friend istream &operator>>(istream& in, Node &node)
			{
				in>>node.val>>node.fa>>node.p;
				int size; in>>size;
				for (int i=0;i<size;i++)
				{
					int num; in>>num;
					node.child.push_back(num);
				}
				return in;
			}
			friend ostream &operator<<(ostream& out, Node &node)
			{
				out<<node.val<<' '<<node.fa<<' '<<node.p<<' '<<node.child.size()<<' ';
				for (auto &num:node.child)
					out<<num<<' ';
				return out;
			}
		};
		int cur;
		vector<Node> nodes;
		Tree()
		{
			Node node; node.fa=-1;
			nodes.push_back(node);
			cur=0;
		}
		void addChild(int p)
		{
			Node node;
			node.fa=cur;
			node.p=p;
			nodes[cur].child.push_back(nodes.size());
			cur=nodes.size();
			nodes.push_back(node);
		}
		void goback()
		{
			cur=nodes[cur].fa;
		}
		friend istream &operator>>(istream& in, Tree &tree)
		{
			int size;
			in>>size;
			for (int i=0;i<size;i++)
			{
				Node node;
				in>>node;
				tree.nodes.push_back(node);
			}
			return in;
		}
		friend ostream &operator<<(ostream& out, Tree &tree)
		{
			out<<tree.nodes.size()<<' ';
			for (auto &node:tree.nodes)
				out<<node<<' ';
			return out;
		}
		void savefile()
		{
			ofstream fout(DEBUG_TREE_FILE);
			fout<<(*this);
			fout.close();
		}
	};
	
	class Cmp_BW
	{
	public:
		enum class Mode
		{
			exact = 1,
			abnormal = 2
		};
		int search_deep = 14;
	private:
		Mode mode;
		typedef float Val;
		struct LType
		{
			Map m;Col col;
			inline ull hashcode() {
				return m.hashcode() + col;
			}
		};
		struct TbType {
			Val lbound, rbound; MP maxmove;
		};
		//Cache<LType,TbType> tbl_moved;
		//Run max time (ms)
		const float MAX_TIME = 1000.921;
		static const int DEFAULT_DEEP = 0;
		bool outtime;
		Map map_s; Col mcol;
		Bit cnt_s[3];
		clock_t time_s;
		int search_cnt;
		Bit board[128];

		Val MiniMaxSearch_Normal(Map &fmap, Col col, int deep, Val alpha, Val beta);
		Val MiniMaxSearch_WinLoss(Map &fmap, Col col, int deep, Val alpha, Val beta);
		Val MiniMaxSearch_Exact(Map &fmap, Col col, int deep, Val alpha, Val beta);
		Val getSceneVal(Map &map, Col col);
		Val getEndVal(Map &map, Col col);
		int evalPtn(Map &map, Col col);
		void maptoBoard(Map &map, Col col)
		{
			if (col==C_B)
			{
				for (int i=0;i<8;i++)
					for (int j=0;j<8;j++)
						board[i*10+j+11]=map[i*8+j];
			}
			else
			{
				for (int i=0;i<8;i++)
					for (int j=0;j<8;j++)
					{
						Col col=map[i*8+j];
						if (col!=C_E)
							board[i*10+j+11]=col_f(col);
						else
							board[i*10+j+11]=0;
					}
			}
		}
		int getMob(Map &map, Col col);
	public:
		Cmp_BW(Mode _mode) :mode(_mode) {}//,tbl_moved(1000000){}
		Cmp_BW(int _mode) :mode((Mode)_mode) {}//,tbl_moved(1000000){}
		Ploc solve(const Map &_map, Col _col);
		void set(const Map &_map, Col _col);

		Ploc run();
		Ploc run_Exact();
		Val run_ExactSco();
		Ploc run_NormalRandom();
		Ploc run_Random();
		Ploc run_Normal();
		Ploc run_WinLoss();
	};

	Ploc Cmp_BW::solve(const Map &_map, Col _col)
	{
		//tbl_moved.clear();
		map_s = _map; mcol = _col;
		time_s = clock();
		map_s.countPiece(cnt_s);
		search_cnt = 0; outtime = false;
		/*
		Map mpc8(MP_SK8);
		if (mpc8==map_s) return Ploc(7,2);
		Map mpc7(MP_SK7);
		if (mpc7==map_s) return Ploc(7,7);
		Map mpc6(MP_SK6);
		if (mpc6==map_s) return Ploc(0,3);
		Map mpc5(MP_SK5);
		if (mpc5==map_s) return Ploc(1,3);
		Map mpc4(MP_SK4);
		if (mpc4==map_s) return Ploc(3,7);
		Map mpc3(MP_SK3);
		if (mpc3==map_s) return Ploc(2,4);
		Map mpc2(MP_SK2);
		if (mpc2==map_s) return Ploc(4,1);
		Map mpc(MP_SK);
		if (mpc==map_s) return Ploc(7,2);*/
		search_deep=6;
		switch (mode)
		{
		case Mode::exact: return run_Normal();
		default:
			return Ploc(-1, -1);
		}
	}

	void Cmp_BW::set(const Map &_map, Col _col)
	{
		//tbl_moved.clear();
		map_s = _map; mcol = _col;
		time_s = clock();
		map_s.countPiece(cnt_s);
		search_cnt = 0; outtime = false;
	}

	Ploc Cmp_BW::run_Random()
	{
		vector<MP> cdlist;
		for (auto p = MP_F;p<MP_E;p++)
			if (map_s.testPiece(p, mcol))
				cdlist.push_back(p);
		return Ploc(cdlist[rand() % cdlist.size()]);
	}

	Ploc Cmp_BW::run()
	{
		if (!map_s.testAll(mcol)) return Ploc(-1, -1);
		search_deep = 6;
		Ploc p = run_Normal();
		if (cnt_s[0] <= 15)
		{
			Ploc p1 = run_Exact();
			if (!outtime) return p1;
		}
		if (outtime) return p;
		search_deep = 6;
		while (1)
		{
			search_deep++;
			Ploc p1 = run_Normal();
			if (outtime) break;
			p = p1;
			if (clock() - time_s>0.29*CLOCKS_PER_SEC) break;
		}
		fout << " " << clock() - time_s << " ms";
		return p;
	}

	Ploc Cmp_BW::run_NormalRandom()
	{
		fout << "Now score: " << getSceneVal(map_s, mcol) << '\n';
		Val alpha = -FLOAT_INF;
		Val beta = FLOAT_INF;
		vector<pair<Val, MP>> rlist;
		auto co1 = col_f(mcol); MP maxp;
		for (auto p = MP_F;p<MP_E;p++)
			if (map_s.testPiece(p, mcol))
			{
				Map tmap = map_s;
				tmap.setPiece(p, mcol);
				Val ret = -MiniMaxSearch_Normal(tmap, co1, 0, -FLOAT_INF, FLOAT_INF);
				rlist.push_back(make_pair(ret, p));
				if (ret>alpha) alpha = ret, maxp = p;
			}
		vector<MP> declist;
		for (auto &it : rlist) fout << (Ploc)it.second << ":" << it.first << " | ";
		for (auto &it : rlist)
			if (it.first>alpha - 8)
				declist.push_back(it.second);
		int dec = rand() % declist.size();
		fout << "Excepting score: " << alpha << '\n';
		fout << "search info:" << search_deep << ' ' << search_cnt << "  TL:" << clock() - time_s << " ms" << '\n';
		return Ploc(declist[dec]);
	}
	//#define DEBUG_TREE
	#ifdef DEBUG_TREE
	Tree* debugtree;
	#endif
	
	Ploc Cmp_BW::run_Normal()
	{
		#ifdef DEBUG_TREE
		ofstream ff("debugtreerootmap.txt",ios::app);
		ff<<map_s;
		ff.close();
		debugtree=new Tree;
		#endif
		fout << "Now score: " << getSceneVal(map_s, mcol) << '\n';
		Val alpha = -FLOAT_INF;
		Val beta = FLOAT_INF;
		auto co1 = col_f(mcol); MP maxp;
		for (auto p = MP_F;p<MP_E;p++)
			if (map_s.testPiece(p, mcol))
			{
				Map tmap = map_s;
				tmap.setPiece(p, mcol);
				#ifdef DEBUG_TREE
				debugtree->addChild(p);
				#endif
				Val ret = -MiniMaxSearch_Normal(tmap, co1, 0, -beta, -alpha);
				#ifdef DEBUG_TREE
				debugtree->nodes[debugtree->cur].val=ret;
				debugtree->goback();
				#endif
				if (ret>alpha) alpha = ret, maxp = p;
			}
		fout << "Excepting score: " << alpha << '\n';
		fout << "search info:" << search_deep << ' ' << search_cnt << "  TL:" << clock() - time_s << " ms" << '\n';
		
		#ifdef DEBUG_TREE
		debugtree->nodes[debugtree->cur].val=alpha;
		debugtree->savefile();
		delete debugtree;
		#endif
		return Ploc(maxp);
	}

	int getEdgeStable(Map &map, Col col)
	{
		Col mcol = col - 1;
		int cnt = 0;
		if (map[0] == col)
		{
			MP p;
			for (p = 1;p<7;p += MP_R)
				if (map[p] == col) cnt++;
				else break;
				if (p<6 && map[7] == col)
					for (p = 6;p>0;p += MP_L)
						if (map[p] == col) cnt++;
						else break;
						for (p = 8;p<56;p += MP_D)
							if (map[p] == col) cnt++;
							else break;
							if (p<48 && map[56] == col)
								for (p = 48;p>0;p += MP_U)
									if (map[p] == col) cnt++;
									else break;
		}
		if (map[63] == col)
		{
			MP p;
			for (p = 62;p>56;p += MP_L)
				if (map[p] == col) cnt++;
				else break;
				if (p>57 && map[56] == col)
					for (p = 57;p<63;p += MP_R)
						if (map[p] == col) cnt++;
						else break;
						for (p = 55;p>15;p += MP_U)
							if (map[p] == col) cnt++;
							else break;
							if (p>15 && map[7] == col)
								for (p = 15;p<63;p += MP_D)
									if (map[p] == col) cnt++;
									else break;
		}
		return cnt;
	}
	
	Cmp_BW::Val Cmp_BW::MiniMaxSearch_Normal(Map &fmap, Col col, int deep, Val alpha, Val beta)
	{
		if (outtime) return 0;
		search_cnt++;
		if (deep == search_deep)
		{
			if (!fmap.testAll(col))
			{
				if (!fmap.testAll(col_f(col))) return getEndVal(fmap, col);
				return -getSceneVal(fmap, col_f(col));
			}
			return getSceneVal(fmap, col);
		}
		Val lyval = 0; Col co1 = col_f(col);
		Bit cnt[3]; fmap.countPiece(cnt);
		//if (deep == search_deep - 1 && cnt[0] >1) lyval = 0.6*getSceneVal(fmap, col) + 0.5*(getEdgeStable(fmap, col) - getEdgeStable(fmap, co1));
		if (0)
		{
			outtime = true;
			return 0;
		}
		if (search_deep>4 && deep<3)
		{
			vector<pair<Val, MP>> cdlist;
			for (auto p = MP_F;p<MP_E;p++)
			if (fmap.testPiece(p, col))
			{
				Map tmap = fmap;
				tmap.setPiece(p, col);
				float ret=getSceneVal(tmap, col_f(col));
				cdlist.push_back({ret, p});
			}
			if (cdlist.empty())
			{
				if (!fmap.testAll(co1)) return getEndVal(fmap, col) + lyval;
				return -MiniMaxSearch_Normal(fmap, co1, deep, -beta, -alpha);
			}
			sort(cdlist.begin(),cdlist.end());
			for (auto &it:cdlist)
			{
				MP p=it.second;
				Map tmap = fmap;
				tmap.setPiece(p, col);
				Val ret = -MiniMaxSearch_Normal(tmap, co1, deep + 1, -beta, -alpha) + lyval;
				if (ret >= beta) return beta;
				if (ret >= alpha) alpha = ret;
			}
			return alpha;
		}
		bool cando = false;
		for (auto p = MP_F;p<MP_E;p++)
			if (fmap.testPiece(p, col))
			{
				Map tmap = fmap;
				tmap.setPiece(p, col);
				#ifdef DEBUG_TREE
				debugtree->addChild(p);
				#endif
				Val ret = -MiniMaxSearch_Normal(tmap, co1, deep + 1, -beta, -alpha) + lyval;
				#ifdef DEBUG_TREE
				debugtree->nodes[debugtree->cur].val=ret;
				debugtree->goback();
				#endif
				if (ret >= beta) return beta;
				if (ret >= alpha) alpha = ret;
				if (!cando) cando = true;
			}
		if (!cando)
		{
			if (!fmap.testAll(co1)) return getEndVal(fmap, col) + lyval;
			return -MiniMaxSearch_Normal(fmap, co1, deep, -beta, -alpha) + 1.5*lyval;
		}
		
		return alpha;
	}


	Ploc Cmp_BW::run_WinLoss()
	{
		search_deep = cnt_s[0];
		bool win = false; MP mp = -1;

		for (auto p = MP_F;p<MP_E;p++)
			if (map_s.testPiece(p, mcol))
			{
				Map tmap = map_s;
				tmap.setPiece(p, mcol);
				if (-MiniMaxSearch_WinLoss(tmap, col_f(mcol), 0, -50, 50) >= 0) win = true, mp = p;
				if (win) break;
			}
		if (win)
		{
			fout << "Win\n";
			return Ploc(mp);
		}
		else
		{
			fout << "Loss\n";
			return Ploc(-1, -1);
		}
	}

	Cmp_BW::Val Cmp_BW::MiniMaxSearch_WinLoss(Map &fmap, Col col, int deep, Val alpha, Val beta)
	{
		if (outtime) return 0;
		search_cnt++;
		if (deep == search_deep) throw; //amazing!
		bool cando = false; Col co1 = col_f(col);
		Bit scnt[3];
		fmap.countPiece(scnt);
		if (scnt[0] == 1)
		{
			for (auto p = MP_F;p<MP_E;p++)
				if (!fmap.isPlaced(p))
				{
					Bit maxp = fmap.getFlipCount(p, col);
					if (maxp>0)
						return scnt[col] - scnt[co1] + 2 * maxp + 1 >= 0 ? 100 : -100;
					else if ((maxp = fmap.getFlipCount(p, co1))>0)
						return scnt[col] - scnt[co1] - 2 * maxp - 1 >= 0 ? 100 : -100;
					else return scnt[col] - scnt[co1] >= 0 ? 100 : -100;
				}
		}
		if (clock() - time_s>MAX_TIME*CLOCKS_PER_SEC)
		{
			outtime = true;
			return 0;
		}
		for (auto p = MP_F;p<MP_E;p++)
			if (fmap.testPiece(p, col))
			{
				Map tmap = fmap;
				tmap.setPiece(p, col);
				Val ret = -MiniMaxSearch_WinLoss(tmap, co1, deep + 1, -beta, -alpha);
				if (ret >= beta) return beta;
				if (ret>alpha) alpha = ret;
				if (!cando) cando = true;
			}
		if (!cando)
		{
			if (!fmap.testAll(co1)) return getEndVal(fmap, col) >= 0 ? 100 : -100;
			return -MiniMaxSearch_WinLoss(fmap, co1, deep, -beta, -alpha);
		}
		return alpha;
	}

	Cmp_BW::Val Cmp_BW::run_ExactSco()
	{
		search_deep = cnt_s[0];
		Val alpha = -FLOAT_INF;
		Val beta = FLOAT_INF;
		auto co1 = col_f(mcol); MP maxp;
		for (auto p = MP_F;p<MP_E;p++)
			if (map_s.testPiece(p, mcol))
			{
				Map tmap = map_s;
				tmap.setPiece(p, mcol);
				Val ret = -MiniMaxSearch_Exact(tmap, co1, 0, -beta, -alpha);
				if (ret>alpha) alpha = ret, maxp = p;
			}
		return alpha;
	}

	//*
	Ploc Cmp_BW::run_Exact()
	{
		search_deep = cnt_s[0];
		Val alpha = -FLOAT_INF;
		Val beta = FLOAT_INF;
		auto co1 = col_f(mcol); MP maxp;
		for (auto p = MP_F;p<MP_E;p++)
			if (map_s.testPiece(p, mcol))
			{
				Map tmap = map_s;
				tmap.setPiece(p, mcol);
				Val ret = -MiniMaxSearch_Exact(tmap, co1, 0, -beta, -alpha);
				if (ret>alpha) alpha = ret, maxp = p;
			}
		fout << "Excepting score: " << alpha << '\n';
		fout << "search info:" << search_deep << ' ' << search_cnt << "  TL:" << clock() - time_s << " ms" << '\n';
		return Ploc(maxp);
	}
	//*/
	/*
	Ploc Cmp_BW::run_Exact()
	{
	search_deep=cnt_s[0];
	auto co1=col_f(mcol); MP maxp;
	int ans=-10000;
	int minn=-64,maxn=64;
	//MiniMaxSearch_Exact(map_s, mcol, 0, -11.5, -11.1);
	//MiniMaxSearch_Exact(map_s, mcol, 0, 11.1, 11.5);
	while (minn+1<maxn)
	{
	int mid=(minn+maxn)/2;
	Val ret=MiniMaxSearch_Exact(map_s, mcol, 0, mid+0.5, mid+1.5);
	fout<<search_cnt<<' '<<mid<<' '<<ret<<'\n';
	if (ret<mid+0.9) maxn=mid;
	else if (ret>mid+1.1) minn=mid+2;
	else {ans=mid;break;};
	}
	fout<<"Excepting score: "<<maxn<<'\n';
	fout<< "search info:"<<search_deep << ' ' << search_cnt <<"  TL:"<<clock()-time_s<<" ms" << '\n';
	int usecnt=0;
	for (int i=0;i<1000000;i++)
	if(tbl_moved.find(i).s.m[0]!=0) usecnt++;
	fout<<usecnt<<'\n';
	return run_Random();
	}
	//*/
	//http://blog.csdn.net/fsdev/article/details/7294902
	Cmp_BW::Val Cmp_BW::MiniMaxSearch_Exact(Map &fmap, Col col, int deep, Val alpha, Val beta)
	{
		if (outtime) return 0;
		search_cnt++;
		if (deep == search_deep)
		{
			throw; //amazing!
		}
		bool cando = false; Col co1 = col_f(col);
		Bit scnt[3];
		fmap.countPiece(scnt);
		if (scnt[0] == 1)
		{
			for (auto p = MP_F;p<MP_E;p++)
				if (!fmap.isPlaced(p))
				{
					Bit maxp = fmap.getFlipCount(p, col);
					if (maxp>0)
						return scnt[col] - scnt[co1] + 2 * maxp + 1;
					else if ((maxp = fmap.getFlipCount(p, co1))>0)
						return scnt[col] - scnt[co1] - 2 * maxp - 1;
					else return scnt[col] - scnt[co1];
				}

		}
		if (clock() - time_s>MAX_TIME*CLOCKS_PER_SEC)
		{
			outtime = true;
			return 0;
		}
		/*
		bool saved=false;
		Cache<LType, TbType>::Node& tblp = tbl_moved.find(LType({fmap,col}).hashcode());
		if (deep<search_deep-3)
		{
		if (tblp.s.m==fmap && tblp.s.col==col)
		{
		saved=true;
		if (tblp.data.rbound<beta) beta=tblp.data.rbound;
		if (tblp.data.lbound>alpha) alpha=tblp.data.lbound;
		if (alpha>=beta) return beta;
		Map tmap=fmap;
		tmap.setPiece(tblp.data.maxmove, col);
		Val ret=-MiniMaxSearch_Exact(tmap, co1, deep+1, -beta, -alpha);
		if (ret>=beta)
		{
		if (beta>tblp.data.lbound) tblp.data.lbound=ret;
		return beta;
		}
		if (ret>alpha) alpha=ret;
		}
		}//*/
		MP maxp;
		for (auto p = MP_F;p<MP_E;p++)
			if (fmap.testPiece(p, col))
			{
				Map tmap = fmap;
				tmap.setPiece(p, col);
				Val ret = -MiniMaxSearch_Exact(tmap, co1, deep + 1, -beta, -alpha);
				if (ret >= beta)
				{/*
				 if (deep<search_deep-3)
				 {
				 if (!saved)
				 {
				 tblp.s.m=fmap;
				 tblp.s.col=col;
				 tblp.data.maxmove=p;
				 tblp.data.lbound=beta;
				 tblp.data.rbound=FLOAT_INF;
				 }
				 else
				 {
				 if (beta>tblp.data.lbound)
				 {
				 tblp.data.lbound=beta;
				 tblp.data.maxmove=p;
				 }
				 }}//*/
					return beta;
				}
				if (ret >= alpha) alpha = ret, maxp = p;
				if (!cando) cando = true;
			}
		if (!cando)
		{
			if (!fmap.testAll(co1)) return getEndVal(fmap, col);
			return -MiniMaxSearch_Exact(fmap, co1, deep, -beta, -alpha);
		}/*

		 if (deep<search_deep-3){
		 if (!saved)
		 {
		 tblp.s.m=fmap;
		 tblp.s.col=col;
		 tblp.data.rbound=alpha;
		 tblp.data.lbound=-FLOAT_INF;
		 tblp.data.maxmove=maxp;
		 }
		 else
		 {
		 if (alpha<tblp.data.rbound)
		 tblp.data.rbound=alpha;
		 }}//*/
		return alpha;
	}

	int getT(Map &map, Col col)
	{
		Col co1 = col_f(col);
		Col mcol = col - 1;
		ull final = 0;
		ull cr = map.m[mcol] & 0xFEFEFEFEFEFEFEFEull;
		final |= (cr >> MP_R) ^ cr;
		ull cl = map.m[mcol] & 0x7F7F7F7F7F7F7F7Full;
		final |= (cl << -MP_L) ^ cl;
		ull cd = map.m[mcol] & 0xFFFFFFFFFFFFFF00ull;
		final |= (cd >> MP_D) ^ cd;
		ull cu = map.m[mcol] & 0x00FFFFFFFFFFFFFFull;
		final |= (cu << -MP_U) ^ cu;
		ull cul = map.m[mcol] & 0x007F7F7F7F7F7F7Full;
		final |= (cul << -MP_UL) ^ cul;
		ull cur = map.m[mcol] & 0x00FEFEFEFEFEFEFEull;
		final |= (cur << -MP_UR) ^ cur;
		ull cdl = map.m[mcol] & 0x7F7F7F7F7F7F7F00ull;
		final |= (cdl >> MP_DL) ^ cdl;
		ull cdr = map.m[mcol] & 0xFEFEFEFEFEFEFE00ull;
		final |= (cdr >> MP_DR) ^ cdr;
		final &= ~(map.m[0] | map.m[1] | 0x42000000004200ull);
		return popcount(final);
	}
	
	const int pow3[13]={1,3,9,27,81,243,729,2187,6561,19683,59049,177147,531441};

void readShort(FILE *stream, short &tar)
{
	fread(&tar, 2, 1, stream);
}

void writeShort(FILE *stream, short tar)
{
	fwrite(&tar, 2, 1, stream);
}
/*
e1x2: edge+2x
c52: ed_cor2*5
c33: ed_cor3*3
e2: line2
e3: line3
e4: line4
k8, k7, k6, k5, k4: / 
wb: const 
wodd: parity
wmob: mobibity
*/
struct CoeffPack
{
	short e1[59049], c52[59049], c33[19683],
		e2[6561], e3[6561], e4[6561], k8[6561], k7[2187], k6[729], k5[243], k4[81],
		wb, wodd, wmob;
	
	void clear()
	{
		wb=wodd=wmob=0;
		for (int j=0;j<59049;j++)
			e1[j]=c52[j]=0;
		for (int j=0;j<19683;j++)
			c33[j]=0;
		for (int j=0;j<6561;j++)
			e2[j]=e3[j]=e4[j]=k8[j]=0;
		for (int j=0;j<2187;j++)
			k7[j]=0;
		for (int j=0;j<729;j++)
			k6[j]=0;
		for (int j=0;j<243;j++)
			k5[j]=0;
		for (int j=0;j<81;j++)
			k4[j]=0;
	}
};

struct GameCoeff
{
	
	#define COEFF_PARTCNT 11
	CoeffPack dat[COEFF_PARTCNT];
	
	#define EVAL_FILE "reversicoeff.bin"
	#define EVAL_FILE_S "reversicoeff_temp.bin"
	
	void initPtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE, "rb");
		short part_cnt; readShort(eval_stream, part_cnt);
		assert(part_cnt == COEFF_PARTCNT);
		for (int i=0;i<part_cnt;i++)
		{
			readShort(eval_stream,pdata[i].wb);
			readShort(eval_stream,pdata[i].wodd);
			readShort(eval_stream,pdata[i].wmob);
			for (int j=0;j<59049;j++)
				readShort(eval_stream,pdata[i].e1[j]);
			for (int j=0;j<59049;j++)
				readShort(eval_stream,pdata[i].c52[j]);
			for (int j=0;j<19683;j++)
				readShort(eval_stream,pdata[i].c33[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata[i].e2[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata[i].e3[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata[i].e4[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata[i].k8[j]);
			for (int j=0;j<2187;j++)
				readShort(eval_stream,pdata[i].k7[j]);
			for (int j=0;j<729;j++)
				readShort(eval_stream,pdata[i].k6[j]);
			for (int j=0;j<243;j++)
				readShort(eval_stream,pdata[i].k5[j]);
			for (int j=0;j<81;j++)
				readShort(eval_stream,pdata[i].k4[j]);
			cout<<pdata[i].wb<<'\n';
			cout<<pdata[i].k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	void savePtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE_S, "wb");
		short part_cnt=COEFF_PARTCNT; writeShort(eval_stream, part_cnt);
		for (int i=0;i<part_cnt;i++)
		{
			writeShort(eval_stream,pdata[i].wb);
			writeShort(eval_stream,pdata[i].wodd);
			writeShort(eval_stream,pdata[i].wmob);
			for (int j=0;j<59049;j++)
				writeShort(eval_stream,pdata[i].e1[j]);
			for (int j=0;j<59049;j++)
				writeShort(eval_stream,pdata[i].c52[j]);
			for (int j=0;j<19683;j++)
				writeShort(eval_stream,pdata[i].c33[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata[i].e2[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata[i].e3[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata[i].e4[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata[i].k8[j]);
			for (int j=0;j<2187;j++)
				writeShort(eval_stream,pdata[i].k7[j]);
			for (int j=0;j<729;j++)
				writeShort(eval_stream,pdata[i].k6[j]);
			for (int j=0;j<243;j++)
				writeShort(eval_stream,pdata[i].k5[j]);
			for (int j=0;j<81;j++)
				writeShort(eval_stream,pdata[i].k4[j]);
			cout<<pdata[i].wb<<'\n';
			cout<<pdata[i].k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	void clear()
	{
		for (int i=0;i<part_cnt;i++)
			pdata[i].clear();
	}
	
	const int Eval_PrTable[61]=
		{-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,
		6,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,9,10,10,10,10,
		10,10,10,10,10,10,10,10,10,10,10,10,10}; 
	}coeff_data;
	
	int Cmp_BW::getMob(Map &map, Col col)
	{
		int ret = 0;
		for (auto i = MP_F;i<MP_E;i++)
			if (map.testPiece(i, col))
				ret++;
		return ret;
	}
	
	int Cmp_BW::evalPtn(Map &map, Col col)
	{
		Bit cnt[3];
		map.countPiece(cnt);
		maptoBoard(map, col);
		int eval_pr=Eval_PrTable[cnt[0]];
		if (eval_pr==10) eval_pr=9;
		int score=pdata[eval_pr].wb;
		
		int cmob = getMob(map, col);
		int codd = cnt[0]%2;
		
		score += pdata[eval_pr].wodd*codd;
		score += pdata[eval_pr].wmob*cmob;
		
		int ptn;
		
		ptn = board[81];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[71];
		ptn = 3 * ptn + board[61];
		ptn = 3 * ptn + board[51];
		ptn = 3 * ptn + board[41];
		ptn = 3 * ptn + board[31];
		ptn = 3 * ptn + board[21];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[11];
		score += pdata[eval_pr].e1[ptn];
		
		ptn = board[88];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[78];
		ptn = 3 * ptn + board[68];
		ptn = 3 * ptn + board[58];
		ptn = 3 * ptn + board[48];
		ptn = 3 * ptn + board[38];
		ptn = 3 * ptn + board[28];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[18];
		score += pdata[eval_pr].e1[ptn];
		
		ptn = board[18];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[17];
		ptn = 3 * ptn + board[16];
		ptn = 3 * ptn + board[15];
		ptn = 3 * ptn + board[14];
		ptn = 3 * ptn + board[13];
		ptn = 3 * ptn + board[12];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[11];
		score += pdata[eval_pr].e1[ptn];
		
		ptn = board[88];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[87];
		ptn = 3 * ptn + board[86];
		ptn = 3 * ptn + board[85];
		ptn = 3 * ptn + board[84];
		ptn = 3 * ptn + board[83];
		ptn = 3 * ptn + board[82];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[81];
		score += pdata[eval_pr].e1[ptn];
		
		ptn = board[82];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[62];
		ptn = 3 * ptn + board[52];
		ptn = 3 * ptn + board[42];
		ptn = 3 * ptn + board[32];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[12];
		score += pdata[eval_pr].e2[ptn];
		
		ptn = board[87];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[67];
		ptn = 3 * ptn + board[57];
		ptn = 3 * ptn + board[47];
		ptn = 3 * ptn + board[37];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[17];
		score += pdata[eval_pr].e2[ptn];
		
		ptn = board[28];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[26];
		ptn = 3 * ptn + board[25];
		ptn = 3 * ptn + board[24];
		ptn = 3 * ptn + board[23];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[21];
		score += pdata[eval_pr].e2[ptn];
		
		ptn = board[78];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[76];
		ptn = 3 * ptn + board[75];
		ptn = 3 * ptn + board[74];
		ptn = 3 * ptn + board[73];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[71];
		score += pdata[eval_pr].e2[ptn];
		
		ptn = board[83];
		ptn = 3 * ptn + board[73];
		ptn = 3 * ptn + board[63];
		ptn = 3 * ptn + board[53];
		ptn = 3 * ptn + board[43];
		ptn = 3 * ptn + board[33];
		ptn = 3 * ptn + board[23];
		ptn = 3 * ptn + board[13];
		score += pdata[eval_pr].e3[ptn];
		
		ptn = board[86];
		ptn = 3 * ptn + board[76];
		ptn = 3 * ptn + board[66];
		ptn = 3 * ptn + board[56];
		ptn = 3 * ptn + board[46];
		ptn = 3 * ptn + board[36];
		ptn = 3 * ptn + board[26];
		ptn = 3 * ptn + board[16];
		score += pdata[eval_pr].e3[ptn];
		
		ptn = board[38];
		ptn = 3 * ptn + board[37];
		ptn = 3 * ptn + board[36];
		ptn = 3 * ptn + board[35];
		ptn = 3 * ptn + board[34];
		ptn = 3 * ptn + board[33];
		ptn = 3 * ptn + board[32];
		ptn = 3 * ptn + board[31];
		score += pdata[eval_pr].e3[ptn];
		
		ptn = board[68];
		ptn = 3 * ptn + board[67];
		ptn = 3 * ptn + board[66];
		ptn = 3 * ptn + board[65];
		ptn = 3 * ptn + board[64];
		ptn = 3 * ptn + board[63];
		ptn = 3 * ptn + board[62];
		ptn = 3 * ptn + board[61];
		score += pdata[eval_pr].e3[ptn];
		
		ptn = board[84];
		ptn = 3 * ptn + board[74];
		ptn = 3 * ptn + board[64];
		ptn = 3 * ptn + board[54];
		ptn = 3 * ptn + board[44];
		ptn = 3 * ptn + board[34];
		ptn = 3 * ptn + board[24];
		ptn = 3 * ptn + board[14];
		score += pdata[eval_pr].e4[ptn];
		
		ptn = board[85];
		ptn = 3 * ptn + board[75];
		ptn = 3 * ptn + board[65];
		ptn = 3 * ptn + board[55];
		ptn = 3 * ptn + board[45];
		ptn = 3 * ptn + board[35];
		ptn = 3 * ptn + board[25];
		ptn = 3 * ptn + board[15];
		score += pdata[eval_pr].e4[ptn];
		
		ptn = board[48];
		ptn = 3 * ptn + board[47];
		ptn = 3 * ptn + board[46];
		ptn = 3 * ptn + board[45];
		ptn = 3 * ptn + board[44];
		ptn = 3 * ptn + board[43];
		ptn = 3 * ptn + board[42];
		ptn = 3 * ptn + board[41];
		score += pdata[eval_pr].e4[ptn];
		
		ptn = board[58];
		ptn = 3 * ptn + board[57];
		ptn = 3 * ptn + board[56];
		ptn = 3 * ptn + board[55];
		ptn = 3 * ptn + board[54];
		ptn = 3 * ptn + board[53];
		ptn = 3 * ptn + board[52];
		ptn = 3 * ptn + board[51];
		score += pdata[eval_pr].e4[ptn];
		
		ptn = board[88];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[66];
		ptn = 3 * ptn + board[55];
		ptn = 3 * ptn + board[44];
		ptn = 3 * ptn + board[33];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[11];
		score += pdata[eval_pr].k8[ptn];
		
		ptn = board[81];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[63];
		ptn = 3 * ptn + board[54];
		ptn = 3 * ptn + board[45];
		ptn = 3 * ptn + board[36];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[18];
		score += pdata[eval_pr].k8[ptn];
		
		ptn = board[78];
		ptn = 3 * ptn + board[67];
		ptn = 3 * ptn + board[56];
		ptn = 3 * ptn + board[45];
		ptn = 3 * ptn + board[34];
		ptn = 3 * ptn + board[23];
		ptn = 3 * ptn + board[12];
		score += pdata[eval_pr].k7[ptn];
		
		ptn = board[87];
		ptn = 3 * ptn + board[76];
		ptn = 3 * ptn + board[65];
		ptn = 3 * ptn + board[54];
		ptn = 3 * ptn + board[43];
		ptn = 3 * ptn + board[32];
		ptn = 3 * ptn + board[21];
		score += pdata[eval_pr].k7[ptn];
		
		ptn = board[71];
		ptn = 3 * ptn + board[62];
		ptn = 3 * ptn + board[53];
		ptn = 3 * ptn + board[44];
		ptn = 3 * ptn + board[35];
		ptn = 3 * ptn + board[26];
		ptn = 3 * ptn + board[17];
		score += pdata[eval_pr].k7[ptn];
		
		ptn = board[82];
		ptn = 3 * ptn + board[73];
		ptn = 3 * ptn + board[64];
		ptn = 3 * ptn + board[55];
		ptn = 3 * ptn + board[46];
		ptn = 3 * ptn + board[37];
		ptn = 3 * ptn + board[28];
		score += pdata[eval_pr].k7[ptn];
		
		ptn = board[68];
		ptn = 3 * ptn + board[57];
		ptn = 3 * ptn + board[46];
		ptn = 3 * ptn + board[35];
		ptn = 3 * ptn + board[24];
		ptn = 3 * ptn + board[13];
		score += pdata[eval_pr].k6[ptn];
		
		ptn = board[86];
		ptn = 3 * ptn + board[75];
		ptn = 3 * ptn + board[64];
		ptn = 3 * ptn + board[53];
		ptn = 3 * ptn + board[42];
		ptn = 3 * ptn + board[31];
		score += pdata[eval_pr].k6[ptn];
		
		ptn = board[61];
		ptn = 3 * ptn + board[52];
		ptn = 3 * ptn + board[43];
		ptn = 3 * ptn + board[34];
		ptn = 3 * ptn + board[25];
		ptn = 3 * ptn + board[16];
		score += pdata[eval_pr].k6[ptn];
		
		ptn = board[83];
		ptn = 3 * ptn + board[74];
		ptn = 3 * ptn + board[65];
		ptn = 3 * ptn + board[56];
		ptn = 3 * ptn + board[47];
		ptn = 3 * ptn + board[38];
		score += pdata[eval_pr].k6[ptn];
		
		ptn = board[58];
		ptn = 3 * ptn + board[47];
		ptn = 3 * ptn + board[36];
		ptn = 3 * ptn + board[25];
		ptn = 3 * ptn + board[14];
		score += pdata[eval_pr].k5[ptn];
		
		ptn = board[85];
		ptn = 3 * ptn + board[74];
		ptn = 3 * ptn + board[63];
		ptn = 3 * ptn + board[52];
		ptn = 3 * ptn + board[41];
		score += pdata[eval_pr].k5[ptn];
		
		ptn = board[51];
		ptn = 3 * ptn + board[42];
		ptn = 3 * ptn + board[33];
		ptn = 3 * ptn + board[24];
		ptn = 3 * ptn + board[15];
		score += pdata[eval_pr].k5[ptn];
		
		ptn = board[84];
		ptn = 3 * ptn + board[75];
		ptn = 3 * ptn + board[66];
		ptn = 3 * ptn + board[57];
		ptn = 3 * ptn + board[48];
		score += pdata[eval_pr].k5[ptn];
	
		ptn = board[48];
		ptn = 3 * ptn + board[37];
		ptn = 3 * ptn + board[26];
		ptn = 3 * ptn + board[15];
		score += pdata[eval_pr].k4[ptn];
		
		ptn = board[84];
		ptn = 3 * ptn + board[73];
		ptn = 3 * ptn + board[62];
		ptn = 3 * ptn + board[51];
		score += pdata[eval_pr].k4[ptn];
		
		ptn = board[41];
		ptn = 3 * ptn + board[32];
		ptn = 3 * ptn + board[23];
		ptn = 3 * ptn + board[14];
		score += pdata[eval_pr].k4[ptn];
		
		ptn = board[85];
		ptn = 3 * ptn + board[76];
		ptn = 3 * ptn + board[67];
		ptn = 3 * ptn + board[58];
		score += pdata[eval_pr].k4[ptn];
		
		ptn = board[11];
		ptn = 3 * ptn + board[12];
		ptn = 3 * ptn + board[13];
		ptn = 3 * ptn + board[21];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[23];
		ptn = 3 * ptn + board[31];
		ptn = 3 * ptn + board[32];
		ptn = 3 * ptn + board[33];
		score += pdata[eval_pr].c33[ptn];
		
		ptn = board[81];
		ptn = 3 * ptn + board[82];
		ptn = 3 * ptn + board[83];
		ptn = 3 * ptn + board[71];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[73];
		ptn = 3 * ptn + board[61];
		ptn = 3 * ptn + board[62];
		ptn = 3 * ptn + board[63];
		score += pdata[eval_pr].c33[ptn];
		
		ptn = board[18];
		ptn = 3 * ptn + board[17];
		ptn = 3 * ptn + board[16];
		ptn = 3 * ptn + board[28];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[26];
		ptn = 3 * ptn + board[38];
		ptn = 3 * ptn + board[37];
		ptn = 3 * ptn + board[36];
		score += pdata[eval_pr].c33[ptn];
		
		ptn = board[88];
		ptn = 3 * ptn + board[87];
		ptn = 3 * ptn + board[86];
		ptn = 3 * ptn + board[78];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[76];
		ptn = 3 * ptn + board[68];
		ptn = 3 * ptn + board[67];
		ptn = 3 * ptn + board[66];
		score += pdata[eval_pr].c33[ptn];
		
		ptn = board[11];
		ptn = 3 * ptn + board[12];
		ptn = 3 * ptn + board[13];
		ptn = 3 * ptn + board[14];
		ptn = 3 * ptn + board[15];
		ptn = 3 * ptn + board[21];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[23];
		ptn = 3 * ptn + board[24];
		ptn = 3 * ptn + board[25];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[81];
		ptn = 3 * ptn + board[82];
		ptn = 3 * ptn + board[83];
		ptn = 3 * ptn + board[84];
		ptn = 3 * ptn + board[85];
		ptn = 3 * ptn + board[71];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[73];
		ptn = 3 * ptn + board[74];
		ptn = 3 * ptn + board[75];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[18];
		ptn = 3 * ptn + board[17];
		ptn = 3 * ptn + board[16];
		ptn = 3 * ptn + board[15];
		ptn = 3 * ptn + board[14];
		ptn = 3 * ptn + board[28];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[26];
		ptn = 3 * ptn + board[25];
		ptn = 3 * ptn + board[24];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[88];
		ptn = 3 * ptn + board[87];
		ptn = 3 * ptn + board[86];
		ptn = 3 * ptn + board[85];
		ptn = 3 * ptn + board[84];
		ptn = 3 * ptn + board[78];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[76];
		ptn = 3 * ptn + board[75];
		ptn = 3 * ptn + board[74];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[11];
		ptn = 3 * ptn + board[21];
		ptn = 3 * ptn + board[31];
		ptn = 3 * ptn + board[41];
		ptn = 3 * ptn + board[51];
		ptn = 3 * ptn + board[12];
		ptn = 3 * ptn + board[22];
		ptn = 3 * ptn + board[32];
		ptn = 3 * ptn + board[42];
		ptn = 3 * ptn + board[52];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[18];
		ptn = 3 * ptn + board[28];
		ptn = 3 * ptn + board[38];
		ptn = 3 * ptn + board[48];
		ptn = 3 * ptn + board[58];
		ptn = 3 * ptn + board[17];
		ptn = 3 * ptn + board[27];
		ptn = 3 * ptn + board[37];
		ptn = 3 * ptn + board[47];
		ptn = 3 * ptn + board[57];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[81];
		ptn = 3 * ptn + board[71];
		ptn = 3 * ptn + board[61];
		ptn = 3 * ptn + board[51];
		ptn = 3 * ptn + board[41];
		ptn = 3 * ptn + board[82];
		ptn = 3 * ptn + board[72];
		ptn = 3 * ptn + board[62];
		ptn = 3 * ptn + board[52];
		ptn = 3 * ptn + board[42];
		score += pdata[eval_pr].c52[ptn];
		
		ptn = board[88];
		ptn = 3 * ptn + board[78];
		ptn = 3 * ptn + board[68];
		ptn = 3 * ptn + board[58];
		ptn = 3 * ptn + board[48];
		ptn = 3 * ptn + board[87];
		ptn = 3 * ptn + board[77];
		ptn = 3 * ptn + board[67];
		ptn = 3 * ptn + board[57];
		ptn = 3 * ptn + board[47];
		score += pdata[eval_pr].c52[ptn];
		
		return score;
	}

	Cmp_BW::Val Cmp_BW::getSceneVal(Map &map, Col col)
	{
		Bit cnt[3]; map.countPiece(cnt);
		if (cnt[0]==0) return cnt[col] - cnt[col_f(col)];
		if (cnt[0]==1)
		{
			Map tmap=map;
			for (auto p = MP_F;p<MP_E;p++)
				if (map[p]==C_E)
					tmap.setPiece(p, col);
			Bit tcnt[3]; tmap.countPiece(tcnt);
			return tcnt[col] - tcnt[col_f(col)];
		}
		return evalPtn(map, col)/256.0;
		/*
		Col co1 = col_f(col);
		int loc_sco = 0;
		if (map[9] == col) loc_sco -= 3.5;
		else if (map[9] == co1) loc_sco += 3.5;
		if (map[14] == col) loc_sco -= 3.5;
		else if (map[14] == co1) loc_sco += 3.5;
		if (map[49] == col) loc_sco -= 3.5;
		else if (map[49] == co1) loc_sco += 3.5;
		if (map[54] == col) loc_sco -= 3.5;
		else if (map[54] == co1) loc_sco += 3.5;
		if (map[0] == col) loc_sco += 9;
		else if (map[0] == co1) loc_sco -= 9;
		if (map[7] == col) loc_sco += 9;
		else if (map[7] == co1) loc_sco -= 9;
		if (map[56] == col) loc_sco += 9;
		else if (map[56] == co1) loc_sco -= 9;
		if (map[63] == col) loc_sco += 9;
		else if (map[63] == co1) loc_sco -= 9;
		if (map[1] == col || map[6] == col) loc_sco -= 0.8;
		else if (map[1] == co1 || map[6] == co1) loc_sco += 0.8;
		if (map[8] == col || map[48] == col) loc_sco -= 0.8;
		else if (map[8] == co1 || map[48] == co1) loc_sco += 0.8;
		if (map[15] == col || map[55] == col) loc_sco -= 0.8;
		else if (map[15] == co1 || map[55] == co1) loc_sco += 0.8;
		if (map[57] == col || map[62] == col) loc_sco -= 0.8;
		else if (map[57] == co1 || map[62] == co1) loc_sco += 0.8;
		return 0.8*(getT(map, co1) - getT(map, col)) + loc_sco;
		*/
	}

	Cmp_BW::Val Cmp_BW::getEndVal(Map &map, Col col)
	{
		Bit cnt[3]; map.countPiece(cnt);
		return (cnt[col] - cnt[col_f(col)]);
	}

}

namespace LinReg
{
GameCoeff rweight;
int ptne1[4][10],ptne2[4][8],ptne3[4][8],ptne4[4][8],ptnk8[2][8],ptnk7[4][7],ptnk6[4][6],ptnk5[4][5],ptnk4[4][4];
int ptne1_[4][10],ptne2_[4][8],ptne3_[4][8],ptne4_[4][8],ptnk8_[2][8],ptnk7_[4][7],ptnk6_[4][6],ptnk5_[4][5],ptnk4_[4][4];

int mm[8][8];

void getptns(int num)
{
	ptne1[0][0]=mm[0][0];
	ptne1[0][1]=mm[1][1];
	ptne1[0][2]=mm[0][1];
	ptne1[0][3]=mm[0][2];
	ptne1[0][4]=mm[0][3];
	ptne1[0][5]=mm[0][4];
	ptne1[0][6]=mm[0][5];
	ptne1[0][7]=mm[0][6];
	ptne1[0][8]=mm[1][6];
	ptne1[0][9]=mm[0][7];
	
	ptne1[1][0]=mm[7][0];
	ptne1[1][1]=mm[6][1];
	ptne1[1][2]=mm[7][1];
	ptne1[1][3]=mm[7][2];
	ptne1[1][4]=mm[7][3];
	ptne1[1][5]=mm[7][4];
	ptne1[1][6]=mm[7][5];
	ptne1[1][7]=mm[7][6];
	ptne1[1][8]=mm[6][6];
	ptne1[1][9]=mm[7][7];

	ptne1[2][0]=mm[0][0];
	ptne1[2][1]=mm[1][1];
	ptne1[2][2]=mm[1][0];
	ptne1[2][3]=mm[2][0];
	ptne1[2][4]=mm[3][0];
	ptne1[2][5]=mm[4][0];
	ptne1[2][6]=mm[5][0];
	ptne1[2][7]=mm[6][0];
	ptne1[2][8]=mm[6][1];
	ptne1[2][9]=mm[7][0];

	ptne1[3][0]=mm[0][7];
	ptne1[3][1]=mm[1][6];
	ptne1[3][2]=mm[1][7];
	ptne1[3][3]=mm[2][7];
	ptne1[3][4]=mm[3][7];
	ptne1[3][5]=mm[4][7];
	ptne1[3][6]=mm[5][7];
	ptne1[3][7]=mm[6][7];
	ptne1[3][8]=mm[7][6];
	ptne1[3][9]=mm[7][7];

	ptne2[0][0] = mm[1][0];
	ptne2[0][1] = mm[1][1];
	ptne2[0][2] = mm[1][2];
	ptne2[0][3] = mm[1][3];
	ptne2[0][4] = mm[1][4];
	ptne2[0][5] = mm[1][5];
	ptne2[0][6] = mm[1][6];
	ptne2[0][7] = mm[1][7];

	ptne2[1][0] = mm[6][0];
	ptne2[1][1] = mm[6][1];
	ptne2[1][2] = mm[6][2];
	ptne2[1][3] = mm[6][3];
	ptne2[1][4] = mm[6][4];
	ptne2[1][5] = mm[6][5];
	ptne2[1][6] = mm[6][6];
	ptne2[1][7] = mm[6][7];

	ptne2[2][0] = mm[0][1];
	ptne2[2][1] = mm[1][1];
	ptne2[2][2] = mm[2][1];
	ptne2[2][3] = mm[3][1];
	ptne2[2][4] = mm[4][1];
	ptne2[2][5] = mm[5][1];
	ptne2[2][6] = mm[6][1];
	ptne2[2][7] = mm[7][1];

	ptne2[3][0] = mm[0][6];
	ptne2[3][1] = mm[1][6];
	ptne2[3][2] = mm[2][6];
	ptne2[3][3] = mm[3][6];
	ptne2[3][4] = mm[4][6];
	ptne2[3][5] = mm[5][6];
	ptne2[3][6] = mm[6][6];
	ptne2[3][7] = mm[7][6];

	ptne3[0][0] = mm[2][0];
	ptne3[0][1] = mm[2][1];
	ptne3[0][2] = mm[2][2];
	ptne3[0][3] = mm[2][3];
	ptne3[0][4] = mm[2][4];
	ptne3[0][5] = mm[2][5];
	ptne3[0][6] = mm[2][6];
	ptne3[0][7] = mm[2][7];

	ptne3[1][0] = mm[5][0];
	ptne3[1][1] = mm[5][1];
	ptne3[1][2] = mm[5][2];
	ptne3[1][3] = mm[5][3];
	ptne3[1][4] = mm[5][4];
	ptne3[1][5] = mm[5][5];
	ptne3[1][6] = mm[5][6];
	ptne3[1][7] = mm[5][7];

	ptne3[2][0] = mm[0][2];
	ptne3[2][1] = mm[1][2];
	ptne3[2][2] = mm[2][2];
	ptne3[2][3] = mm[3][2];
	ptne3[2][4] = mm[4][2];
	ptne3[2][5] = mm[5][2];
	ptne3[2][6] = mm[6][2];
	ptne3[2][7] = mm[7][2];

	ptne3[3][0] = mm[0][5];
	ptne3[3][1] = mm[1][5];
	ptne3[3][2] = mm[2][5];
	ptne3[3][3] = mm[3][5];
	ptne3[3][4] = mm[4][5];
	ptne3[3][5] = mm[5][5];
	ptne3[3][6] = mm[6][5];
	ptne3[3][7] = mm[7][5];

	ptne4[0][0] = mm[3][0];
	ptne4[0][1] = mm[3][1];
	ptne4[0][2] = mm[3][2];
	ptne4[0][3] = mm[3][3];
	ptne4[0][4] = mm[3][4];
	ptne4[0][5] = mm[3][5];
	ptne4[0][6] = mm[3][6];
	ptne4[0][7] = mm[3][7];

	ptne4[1][0] = mm[4][0];
	ptne4[1][1] = mm[4][1];
	ptne4[1][2] = mm[4][2];
	ptne4[1][3] = mm[4][3];
	ptne4[1][4] = mm[4][4];
	ptne4[1][5] = mm[4][5];
	ptne4[1][6] = mm[4][6];
	ptne4[1][7] = mm[4][7];

	ptne4[2][0] = mm[0][3];
	ptne4[2][1] = mm[1][3];
	ptne4[2][2] = mm[2][3];
	ptne4[2][3] = mm[3][3];
	ptne4[2][4] = mm[4][3];
	ptne4[2][5] = mm[5][3];
	ptne4[2][6] = mm[6][3];
	ptne4[2][7] = mm[7][3];

	ptne4[3][0] = mm[0][4];
	ptne4[3][1] = mm[1][4];
	ptne4[3][2] = mm[2][4];
	ptne4[3][3] = mm[3][4];
	ptne4[3][4] = mm[4][4];
	ptne4[3][5] = mm[5][4];
	ptne4[3][6] = mm[6][4];
	ptne4[3][7] = mm[7][4];

	ptnk8[0][0] = mm[0][0];
	ptnk8[0][1] = mm[1][1];
	ptnk8[0][2] = mm[2][2];
	ptnk8[0][3] = mm[3][3];
	ptnk8[0][4] = mm[4][4];
	ptnk8[0][5] = mm[5][5];
	ptnk8[0][6] = mm[6][6];
	ptnk8[0][7] = mm[7][7];

	ptnk8[1][0] = mm[0][7];
	ptnk8[1][1] = mm[1][6];
	ptnk8[1][2] = mm[2][5];
	ptnk8[1][3] = mm[3][4];
	ptnk8[1][4] = mm[4][3];
	ptnk8[1][5] = mm[5][2];
	ptnk8[1][6] = mm[6][1];
	ptnk8[1][7] = mm[7][0];

	ptnk7[0][0] = mm[0][1];
	ptnk7[0][1] = mm[1][2];
	ptnk7[0][2] = mm[2][3];
	ptnk7[0][3] = mm[3][4];
	ptnk7[0][4] = mm[4][5];
	ptnk7[0][5] = mm[5][6];
	ptnk7[0][6] = mm[6][7];

	ptnk7[1][0] = mm[1][0];
	ptnk7[1][1] = mm[2][1];
	ptnk7[1][2] = mm[3][2];
	ptnk7[1][3] = mm[4][3];
	ptnk7[1][4] = mm[5][4];
	ptnk7[1][5] = mm[6][5];
	ptnk7[1][6] = mm[7][6];

	ptnk7[2][0] = mm[0][6];
	ptnk7[2][1] = mm[1][5];
	ptnk7[2][2] = mm[2][4];
	ptnk7[2][3] = mm[3][3];
	ptnk7[2][4] = mm[4][2];
	ptnk7[2][5] = mm[5][1];
	ptnk7[2][6] = mm[6][0];

	ptnk7[3][0] = mm[6][0];
	ptnk7[3][1] = mm[5][1];
	ptnk7[3][2] = mm[4][2];
	ptnk7[3][3] = mm[3][3];
	ptnk7[3][4] = mm[2][4];
	ptnk7[3][5] = mm[1][5];
	ptnk7[3][6] = mm[0][6];

	ptnk6[0][0] = mm[0][2];
	ptnk6[0][1] = mm[1][3];
	ptnk6[0][2] = mm[2][4];
	ptnk6[0][3] = mm[3][5];
	ptnk6[0][4] = mm[4][6];
	ptnk6[0][5] = mm[5][7];

	ptnk6[1][0] = mm[2][0];
	ptnk6[1][1] = mm[3][1];
	ptnk6[1][2] = mm[4][2];
	ptnk6[1][3] = mm[5][3];
	ptnk6[1][4] = mm[6][4];
	ptnk6[1][5] = mm[7][5];
	
	ptnk6[2][0] = mm[0][5];
	ptnk6[2][1] = mm[1][4];
	ptnk6[2][2] = mm[2][3];
	ptnk6[2][3] = mm[3][2];
	ptnk6[2][4] = mm[4][1];
	ptnk6[2][5] = mm[5][0];
	
	ptnk6[3][0] = mm[5][0];
	ptnk6[3][1] = mm[4][1];
	ptnk6[3][2] = mm[3][2];
	ptnk6[3][3] = mm[2][3];
	ptnk6[3][4] = mm[1][4];
	ptnk6[3][5] = mm[0][5];

	ptnk5[0][0] = mm[0][3];
	ptnk5[0][1] = mm[1][4];
	ptnk5[0][2] = mm[2][5];
	ptnk5[0][3] = mm[3][6];
	ptnk5[0][4] = mm[4][7];
	
	ptnk5[1][0] = mm[3][0];
	ptnk5[1][1] = mm[4][1];
	ptnk5[1][2] = mm[5][2];
	ptnk5[1][3] = mm[6][3];
	ptnk5[1][4] = mm[7][4];
	
	ptnk5[2][0] = mm[0][4];
	ptnk5[2][1] = mm[1][3];
	ptnk5[2][2] = mm[2][2];
	ptnk5[2][3] = mm[3][1];
	ptnk5[2][4] = mm[4][0];
	
	ptnk5[3][0] = mm[4][0];
	ptnk5[3][1] = mm[3][1];
	ptnk5[3][2] = mm[2][2];
	ptnk5[3][3] = mm[1][3];
	ptnk5[3][4] = mm[0][4];

	ptnk4[0][0] = mm[0][4];
	ptnk4[0][1] = mm[1][5];
	ptnk4[0][2] = mm[2][6];
	ptnk4[0][3] = mm[3][7];
	
	ptnk4[1][0] = mm[4][0];
	ptnk4[1][1] = mm[5][1];
	ptnk4[1][2] = mm[6][2];
	ptnk4[1][3] = mm[7][3];
	
	ptnk4[2][0] = mm[0][3];
	ptnk4[2][1] = mm[1][2];
	ptnk4[2][2] = mm[2][1];
	ptnk4[2][3] = mm[3][0];
	
	ptnk4[3][0] = mm[3][0];
	ptnk4[3][1] = mm[2][1];
	ptnk4[3][2] = mm[1][2];
	ptnk4[3][3] = mm[0][3];
}

void getpartner()
{
	for (int i=0;i<4;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			ptne1_[i][j] = ptne1[i][7 - j];
			ptne2_[i][j] = ptne2[i][7 - j];
			ptne3_[i][j] = ptne3[i][7 - j];
			ptne4_[i][j] = ptne4[i][7 - j];
		}
		for (int j = 0;j < 7;j++) ptnk7_[i][j] = ptnk7[i][6 - j];
		for (int j = 0;j < 6;j++) ptnk6_[i][j] = ptnk6[i][5 - j];
		for (int j = 0;j < 5;j++) ptnk5_[i][j] = ptnk5[i][4 - j];
		for (int j = 0;j < 4;j++) ptnk4_[i][j] = ptnk4[i][3 - j];
	}
	for (int i=0;i<2;i++)
		for (int j = 0;j < 8;j++)
			ptnk8_[i][j] = ptnk8[i][7 - j];
}

void printb(int num)
{
	for (int i=0;i<7;i++,cout<<'\n')
		for (int j=0;j<7;j++)
			cout<<mm[i][j]<<' ';
}

float ssig=0,sse=0;
Map mmap;
Bit mcnt[3];

int getMob(int num)
{
	int ret=0;
	for (auto i=MP_F;i<MP_E;i++)
		if (mmap.testPiece(i, mcol[num]))
			ret++;
	return ret;
}

int getOdd(int num)
{
	return mcnt[0]%2;
}
/*
void valid(int num)
{
	mmap.resetByArr(mm); mmap.countPiece(mcnt);
	//cout<<mmap.toString(); 
	getptns(num);
	int pe1[4],pe2[4],pe3[4],pe4[4],pk8[2],pk7[4],pk6[4],pk5[4],pk4[4];
	for (int i=0;i<4;i++)
	{
		pe1[i] = ptnhash(ptne1[i], 10);
		pe2[i] = ptnhash(ptne2[i], 8);
		pe3[i] = ptnhash(ptne3[i], 8);
		pe4[i] = ptnhash(ptne4[i], 8);
		pk7[i] = ptnhash(ptnk7[i], 7);
		pk6[i] = ptnhash(ptnk6[i], 6);
		pk5[i] = ptnhash(ptnk5[i], 5);
		pk4[i] = ptnhash(ptnk4[i], 4);
	}
	for (int i = 0;i < 2;i++)
		pk8[i] = ptnhash(ptnk8[i], 8);
	float sigma=wb;
	for (int i=0;i<4;i++)
	{
		sigma += we1[pe1[i]];
		sigma += we2[pe2[i]];
		sigma += we3[pe3[i]];
		sigma += we4[pe4[i]];
		sigma += wk7[pk7[i]];
		sigma += wk6[pk6[i]];
		sigma += wk5[pk5[i]];
		sigma += wk4[pk4[i]];
	}
	for (int i = 0;i < 2;i++) 
		sigma += wk8[pk8[i]];
	int cmob = getMob(num);
	int codd = getOdd(num);
	sigma += wmob*cmob;
	sigma += wib*codd;

	float mse=(sigma-mval[num])*(sigma-mval[num])/2, delta=mval[num]-sigma;
	sse+=fabs(delta);
}
*/
void updateArg()
{
	float ee=le/batch_size;
	for (int k=0;k<COEFF_PACK_SIZE;k++)
	{
		CoeffPack &w=coeff_data.pdata[k];
		CoeffPack &rw=rweight.pdata[k];
		for (int i = 0;i < 59049;i++)
			w.e1[i] += rw.e1[i] * ee;
		for (int i = 0;i < 6561;i++)
		{
			w.e2[i] += rw.e2[i] * ee;
			w.e3[i] += rw.e3[i] * ee;
			w.e4[i] += rw.e4[i] * ee;
			w.k8[i] += rw.k8[i] * ee;
		}
		for (int i=0;i<2187;i++)
			w.k7[i] += rw.k7[i] * ee;
		for (int i=0;i<729;i++)
			w.k6[i] += rw.k6[i] * ee;
		for (int i=0;i<243;i++)
			w.k5[i] += rw.k5[i] * ee;
		for (int i=0;i<81;i++)
			w.k4[i] += rw.k4[i] * ee;
		w.wmob += rw.wmob*ee;
		w.wodd += rw.wodd*ee;
		w.wb += rw.wb*ee;
	}
	
	rw.clear();
}

void accuGrad(Board mmap)
{
	mmap.countPiece(mcnt);
	//cout<<mmap.toString(); 
	getptns(num);
	getpartner();
	int pe1[4],pe2[4],pe3[4],pe4[4],pk8[2],pk7[4],pk6[4],pk5[4],pk4[4];
	int pe1_[4], pe2_[4], pe3_[4], pe4_[4], pk8_[2], pk7_[4], pk6_[4], pk5_[4], pk4_[4];
	
	
	CoeffPack &w=coeff_data.pdata[eval_pr];
	CoeffPack &rw=rweight.pdata[eval_pr];
	for (int i=0;i<4;i++)
	{
		pe1[i] = ptnhash(ptne1[i], 10);
		pe2[i] = ptnhash(ptne2[i], 8);
		pe3[i] = ptnhash(ptne3[i], 8);
		pe4[i] = ptnhash(ptne4[i], 8);
		pk7[i] = ptnhash(ptnk7[i], 7);
		pk6[i] = ptnhash(ptnk6[i], 6);
		pk5[i] = ptnhash(ptnk5[i], 5);
		pk4[i] = ptnhash(ptnk4[i], 4);

		pe1_[i] = ptnhash(ptne1_[i], 10);
		pe2_[i] = ptnhash(ptne2_[i], 8);
		pe3_[i] = ptnhash(ptne3_[i], 8);
		pe4_[i] = ptnhash(ptne4_[i], 8);
		pk7_[i] = ptnhash(ptnk7_[i], 7);
		pk6_[i] = ptnhash(ptnk6_[i], 6);
		pk5_[i] = ptnhash(ptnk5_[i], 5);
		pk4_[i] = ptnhash(ptnk4_[i], 4);
	}

	for (int i = 0;i < 2;i++)
	{
		pk8[i] = ptnhash(ptnk8[i], 8);
		pk8_[i] = ptnhash(ptnk8_[i], 8);
	}
	float sigma=w.wb;
	for (int i=0;i<4;i++)
	{
		sigma += w.e1[pe1[i]];
		sigma += w.e2[pe2[i]];
		sigma += w.e3[pe3[i]];
		sigma += w.e4[pe4[i]];
		sigma += w.k7[pk7[i]];
		sigma += w.k6[pk6[i]];
		sigma += w.k5[pk5[i]];
		sigma += w.k4[pk4[i]];
	}
	for (int i = 0;i < 2;i++) 
		sigma += w.k8[pk8[i]];
	int cmob = getMob(num);
	int codd = getOdd(num);
	sigma += w.wmob*cmob;
	sigma += w.wodd*codd;

	float mse=(sigma-mval[num])*(sigma-mval[num])/2, delta=mval[num]-sigma;
	sse+=fabs(delta);
	for (int i=0;i<4;i++)
	{
		rw.e1[pe1[i]]+=delta;
		rw.[pe2[i]]+=delta;
		rw.e3[pe3[i]]+=delta;
		rw.e4[pe4[i]]+=delta;
		rw.k7[pk7[i]]+=delta;
		rw.k6[pk6[i]]+=delta;
		rw.k5[pk5[i]]+=delta;
		rw.k4[pk4[i]]+=delta;
		
		rw.e1[pe1_[i]]+=delta;
		rw.e2[pe2_[i]]+=delta;
		rw.e3[pe3_[i]]+=delta;
		rw.e4[pe4_[i]]+=delta;
		rw.k7[pk7_[i]]+=delta;
		rw.k6[pk6_[i]]+=delta;
		rw.k5[pk5_[i]]+=delta;
		rw.k4[pk4_[i]]+=delta;
	}
	for (int i = 0;i < 2;i++)
	{
		rw.k8[pk8[i]] += delta;
		rw.k8[pk8_[i]] += delta;
	}
	rw.wb += delta;
	rw.wmob += delta*cmob;
	rw.wodd += delta*codd;
}
}

using namespace bwcore;

HANDLE hOut, hIn;
const COORD winsize={80,25};

void gotoXY(Ploc mp)
{
	SetConsoleCursorPosition(hOut, { (SHORT)mp.x,(SHORT)mp.y });
}

void minit()
{
	HANDLE consolehwnd;
	consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consolehwnd, 127 + 128 - 15);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	SMALL_RECT rc={0,0,winsize.X,winsize.Y};
	SetConsoleScreenBufferSize(hOut, winsize);
	SetConsoleWindowInfo(hOut, TRUE, &rc);
	
	SetConsoleCursorPosition(hOut, { 0,0 });
	for (int i=0;i<winsize.Y;i++) 
		for (int j=0;j<winsize.X;j++)
			putchar(' ');
	gotoXY({30,11}); printf("?????   1.3");
	Sleep(400);
}

void mexit()
{
	CloseHandle(hOut);
	CloseHandle(hIn);
}

Ploc getCurClick()
{
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	INPUT_RECORD    mouseRec;
	DWORD           res;
	COORD           crPos, crHome = { 0, 0 };
	while (1)
	{
		ReadConsoleInput(hIn, &mouseRec, 1, &res);
		if (mouseRec.EventType == MOUSE_EVENT)
		{
			crPos = mouseRec.Event.MouseEvent.dwMousePosition;
			switch (mouseRec.Event.MouseEvent.dwButtonState)
			{
			case FROM_LEFT_1ST_BUTTON_PRESSED:
				return{ (Bit)crPos.Y, (Bit)crPos.X };
			}
		}
	}
}

const int M_SIZE=8;

class Game_BW
{
private:
	Map map;
	int pcnt, nplayer, sel_b, sel_w;
	static const int PL_CMP = 1, PL_PLY = 2;
	int dPly[3];
	Bit pCnt[3];
	
	Ploc MlocToPloc(const Ploc &p){
		if (p.y >= 4 * M_SIZE || p.x >= 2 * M_SIZE ||
			p.y % 4 == 0 || p.y % 4 == 1 || p.x % 2 == 0) return{ -1,-1 };
		return{ p.x / 2,p.y / 4 };
	}
	Ploc PlocToMloc(const Ploc &p){
		return{ p.y * 4 + 2,p.x * 2 + 1 };
	}
	int rPlayer(){
		if (nplayer == C_B) return C_W;
		else return C_B;
	}
	void showCanDo();
	void showAbout();
	void writeSelect(int col);
	void iPrint();
	int splash();
	void Start();
	void Play();
	void End();
	void SelfPlay();
public:
	void RunTests();
	void UserPlay();
}game;

void Game_BW::iPrint()
{
	gotoXY({0,0});
	printf("?X"); for (int i = 1; i < M_SIZE; i++) printf("?T?j"); printf("?T?[\n");
	for (Bit i = 0; i < M_SIZE; i++)
	{
		printf("?U");
		for (Bit j = 0; j < M_SIZE; j++)
		{
			if (map[{i, j}] == C_W) printf("??");
			else if (map[{i, j}] == C_B) printf("??");
			else printf("  ");
			printf("?U");
		}
		printf("\n");
		if (i < M_SIZE - 1)
		{
			printf("?d");
			for (int j = 0; j < 7; j++) printf("?T?p"); printf("?T?g");
		}
		else
		{
			printf("?^");
			for (int j = 0; j < 7; j++) printf("?T?m"); printf("?T?a");
		}
		printf("\n");
	}
	
	for (Bit i = 0; i < M_SIZE; i++)
	{
		for (Bit j = 0; j < M_SIZE; j++)
			if (map[{i, j}] == C_W) fout<<"??";
			else if (map[{i, j}] == C_B) fout<<"??";
			else fout<<"??";
		fout<<'\n';
	}/*
	fout<<"{\n";
	for (auto p=MP_F;p<MP_E;p++)
	{
		fout<<(int)map[p]<<',';
		if (p%8==7) fout<<'\n';
	}
	fout<<"}";*/
	fout<<'\n';
}

void Game_BW::showCanDo()
{
	vector<MP> clist;
	map.getCanDoList(nplayer, clist);
	for (auto &p : clist)
	{
		gotoXY(PlocToMloc(Ploc(p)));
		printf("??");
	}
}

void Game_BW::SelfPlay()
{
	map=Map::Map_Start;
	pcnt=4;
	Cmp_BW ccmp_b(1),ccmp_w(1);
	ccmp_b.search_deep=ccmp_w.search_deep=1;
	while (pcnt < M_SIZE*M_SIZE)
	{
		map.countPiece(pCnt);
		printf("W:%2d  B:%2d\n", pCnt[C_W], pCnt[C_B]);
		if (nplayer == C_W) printf("now:White");
		else printf("now:Black");
		Ploc sp;
		if (nplayer == C_B) sp = ccmp_b.solve(map, nplayer);
		else sp = ccmp_w.solve(map, nplayer);
		
		map.setPiece(sp.toMP(), nplayer);
		if (!map.testAll(rPlayer())){
			if (!map.testAll(nplayer))
				break;
		}
		else nplayer = rPlayer();
		gotoXY({ 0,0 });
		pcnt++;
		//fout<<"Score of black: "<<ccmp_b.getSenceVal(map, C_B)+ccmp_b.getSenceVal_agg(map, C_B)<<"\n"; //Black
		map.countPiece(pCnt);
		fout<<"Step: "<<pcnt<<"  W: "<<(int)pCnt[C_W]<<"  B: "<<(int)pCnt[C_B]<<'\n';
		//iPrint();
		logRefrsh();
	}
}

void Game_BW::RunTests()
{
	int run_cnt=100;
	ofstream fresult("result.txt");
	int wcnt=0;
	for (int i=0;i<run_cnt/2;i++)
	{
		nplayer=C_B;
		SelfPlay();
		fresult<<"W: "<<(int)pCnt[C_W]<<"  B: "<<(int)pCnt[C_B]<<'\n';
		if (pCnt[C_W]>pCnt[C_B]) wcnt++;
		fresult.flush();
	}
	fresult<<wcnt<<'\n';
	for (int i=0;i<run_cnt/2;i++)
	{
		nplayer=C_W;
		SelfPlay();
		fresult<<"W: "<<(int)pCnt[C_W]<<"  B: "<<(int)pCnt[C_B]<<'\n';
		if (pCnt[C_W]>pCnt[C_B]) wcnt++;
		fresult.flush();
	}
	fresult<<wcnt<<'\n';
}

const int EXP_BUFFER_SIZE = 100000;

struct ExpBuffer
{
	float targetV;
	Board board;
	Col col;
}expbuffer[EXP_BUFFER_SIZE];
bool pro_train=true; int pro_listsize;

void insertFrame(float val, Board &board, int col)
{
	if (pro_train)
	{
		expbuffer[pro_listsize]={val,board, col};
		pro_listsize++;
		if (pro_listsize >= EXP_BUFFER_SIZE) pro_train=false;
	}
	else
	{
		int sr=rand()*rand() % EXP_BUFFER_SIZE;
		expbuffer[sr]={val,board, col};
	}
}

void playEposide()
{
	map=Map::Map_Start;
	pcnt=4;
	Cmp_BW ccmp(1);
	listsize=0;
	while (pcnt < M_SIZE*M_SIZE)
	{
		map.countPiece(pCnt);
		Ploc sp;
		ccmp.search_deep=4;
		ccmp.solve(map, nplayer);
		int val=ccmp.runSco(map, nplayer);
		insert(val, map, nplayer);
		
		map.setPiece(sp.toMP(), nplayer);
		if (!map.testAll(rPlayer())){
			if (!map.testAll(nplayer))
				break;
		}
		else nplayer = rPlayer();
		
		pcnt++;
		listsize++;
		map.countPiece(pCnt);
		
		logRefrsh();
	}
}

ll framePoint;

void trainCoeff()
{
	for (;;framePoint++)
	{
		int nowp=framePoint % EXP_BUFFER_SIZE;
		accuGrad();
		if (framePoint%batch_size==0)
		{
			updateArg();
		}
	}
}

void Game_BW::RunLearning()
{
	const int run_cnt=1000, batch_size=1;
	framePoint=0;
	pro_train=true;
	while (pro_train)
	{
		playEposide();
	}
	
	for (int i=1;i<=run_cnt;i++)
	{
		const int per_playcount=100;
		for (int j=1;i<=per_playcount;i++)
			playEposide();
		
		trainCoeff();
	}
}

void Game_BW::Play()
{
	Cmp_BW ccmp_b(1),ccmp_w(1);
	while (pcnt < M_SIZE*M_SIZE)
	{
		map.countPiece(pCnt);
		printf("W:%2d  B:%2d\n", pCnt[C_W], pCnt[C_B]);
		if (nplayer == C_W) printf("now:White");
		else printf("now:Black");
		Ploc sp;
		if (dPly[nplayer] == PL_PLY)
		{
			sp = MlocToPloc(getCurClick());
			bool flag = false;
			while (!sp.inBorder() || !map.testPiece(sp.toMP(), nplayer))
			{
				if (!flag) showCanDo(), 
				flag = true;
				sp = MlocToPloc(getCurClick());
			}
		}
		else
		{
			if (nplayer == C_B)
			sp = ccmp_b.solve(map, nplayer);
			else sp = ccmp_w.solve(map, nplayer);
			Sleep(1);
		}
		map.setPiece(sp.toMP(), nplayer);
		gotoXY(PlocToMloc(sp));
		if (nplayer == C_W) printf("??");
		else if (nplayer == C_B) printf("??");
		Sleep(1);
		if (!map.testAll(rPlayer())){
			if (!map.testAll(nplayer))
				break;
		}
		else nplayer = rPlayer();
		gotoXY({ 0,0 });
		pcnt++;
		//fout<<"Score of black: "<<ccmp_b.getSenceVal(map, C_B)+ccmp_b.getSenceVal_agg(map, C_B)<<"\n"; //Black
		map.countPiece(pCnt);
		fout<<"Step: "<<pcnt<<"  W: "<<(int)pCnt[C_W]<<"  B: "<<(int)pCnt[C_B]<<'\n';
		iPrint();
		logRefrsh();
	}
}

void Game_BW::End()
{
	gotoXY({0,0});
	iPrint();
		logRefrsh();
	map.countPiece(pCnt);
	printf("W:%2d  B:%2d\n", pCnt[C_W], pCnt[C_B]);
	if (pCnt[C_W] > pCnt[C_B])
		printf(" Winner:White");
	else if (pCnt[C_W] == pCnt[C_B])
		printf(" Equal");
	else
		printf(" Winner:Black");
	gotoXY({0,20});
	printf("[????]");
	while (1)
	{
		Ploc p=getCurClick();
		if (p.x==20 && p.y<8) break;
	}
}

void Game_BW::writeSelect(int col)
{
	if (col==C_B)
	{
		gotoXY({32,11}); printf("                ");
		if (sel_b==-1) {gotoXY({35,11}); printf("????????");}
		else
		{
			gotoXY({32,11}); printf("?????????");
			switch(sel_b)
			{
				case 0:printf("(????)"); break;
				case 1:printf("(????)"); break;
				case 2:printf("(????)"); break;
				case 3:printf("(????)"); break;
			}
		}
	}
	else if (col==C_W)
	{
		gotoXY({32,12}); printf("                 ");
		if (sel_w==-1) {gotoXY({35,12}); printf("????????");}
		else
		{
			gotoXY({32,12}); printf("?????????");
			switch(sel_w)
			{
				case 0:printf("(????)"); break;
				case 1:printf("(????)"); break;
				case 2:printf("(????)"); break;
				case 3:printf("(????)"); break;
			}
		}
	}
}

void Game_BW::showAbout()
{
	for (Bit i=1;i<winsize.Y-2;i++)
	{
		gotoXY({2,i});
		for (int j=0;j<winsize.X-4;j++) putchar(' ');
	}
	gotoXY({2,1}); printf("BlackWhiteChess(Reversi) 1.3 improve0");
	gotoXY({2,3}); printf("Appearacne: Untitled");
	gotoXY({2,4}); printf("  (version=1.0   date=2016-6-20) fffasttime");
	gotoXY({2,6}); printf("Kernel: bwcore1.2_i4 ");
	gotoXY({2,7}); printf("  (version=1.3.0   date=2016-7-10) fffasttime");
	gotoXY({2,8}); printf("  Description: a weak reversi AI");
	gotoXY({36,10}); printf("[back]");
	while (1)
	{
		Ploc p=getCurClick();
		if (p.x==10 && p.y>=36 && p.y<42) break;
	}
}

int Game_BW::splash()
{
	gotoXY({0,0});
	printf("?X"); for (int i = 1; i < winsize.X/2-1; i++) printf("?T"); printf("?[");
	for (int i=1;i<winsize.Y-2;i++)
	{
		printf("?U");
		for (int j=1;j<winsize.X/2-1;j++)
			printf("  ");
		printf("?U");
	}
	printf("?^"); for (int i = 1; i < winsize.X/2-1; i++) printf("?T"); printf("?a");
	gotoXY({30,11}); printf("               ");
	gotoXY({32,8}); printf("?????   1.3");
	gotoXY({36,10}); printf(">>???<<");
	gotoXY({34,13}); printf("????    ???");
	sel_b=-1; writeSelect(C_B);
	sel_w=-1; writeSelect(C_W);
	gotoXY({0,0});
	while (1)
	{
		Ploc p;
		p = getCurClick();
		if (p.x==10 && p.y>=35 && p.y<45) break;
		if (p.x==11 && p.y>=32 && p.y<48)
		{
			sel_b++; if (sel_b>3) sel_b=-1;
			writeSelect(C_B);
		}
		if (p.x==12 && p.y>=32 && p.y<48)
		{
			sel_w++; if (sel_w>3) sel_w=-1;
			writeSelect(C_W);
		}
		if (p.x==13 && p.y>=34 && p.y<38)
		{
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

void Game_BW::Start()
{
	pcnt = 4; nplayer = C_B;
	if (sel_b==-1) dPly[C_B]=PL_PLY,fout<<"Black:Player  ";
	else dPly[C_B]=PL_CMP,fout<<"Black:Computer,"<<sel_b<<"   ";
	if (sel_w==-1) dPly[C_W]=PL_PLY,fout<<"White:Player  ";
	else dPly[C_W]=PL_CMP,fout<<"White:Computer,"<<sel_w<<" \n";
	map=Map::Map_Start;
	//map.m[0]=18341126030338196998ull;
	//map.m[1]=33560449333409808ull;
	iPrint();
}

void Game_BW::UserPlay()
{
	minit();
	while (1)
	{
		int ret = game.splash();
		if (ret == 0) break;
		else if (ret<0) continue;
		game.Start();
		game.Play();
		game.End();
	}
	mexit();
}

int main()
{
	logRefrsh();
	//initPtnData();
	//before srand(1)
	srand(2);
	//game.UserPlay();
	game.RunLearning();
	return 0;
}

