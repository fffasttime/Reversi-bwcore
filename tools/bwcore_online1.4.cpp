// bwcore1.4, botzone keep running version
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
using namespace std;

//#define LOG_PRINT

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
		filelog.flush();
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
		i << '<' << v.x << ',' << v.y << '>';
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
			s+='0'+(*this)[p];
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
		const float MAX_TIME = 0.88;
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
		case Mode::exact: return run();
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
		fout << "Now score: " << getSceneVal(map_s, mcol) << '\n';
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
		if (clock() - time_s>MAX_TIME*CLOCKS_PER_SEC)
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
	}pdata[12];
	
	#define EVAL_FILE "data/reversicoeff_old_botzone.bin"
	//#define EVAL_FILE "trained11_13.bin"
	
	void readShort(FILE *stream, short &tar)
	{
		fread(&tar, 2, 1, stream);
	}
	
	void writeShort(FILE *stream, short tar)
	{
		fwrite(&tar, 2, 1, stream);
	}
	
	int pow3[13]={1,3,9,27,81,243,729,2187,6561,19683,59049,177147,531441};
	
	void initPtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE, "rb");
		short part_cnt; readShort(eval_stream, part_cnt);
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
			//cout<<pdata[i].wb<<'\n';
			//cout<<pdata[i].k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	int Cmp_BW::getMob(Map &map, Col col)
	{
		int ret = 0;
		for (auto i = MP_F;i<MP_E;i++)
			if (map.testPiece(i, col))
				ret++;
		return ret;
	}
	int Eval_PrTable[61]=
	{-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,
	6,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,9,9,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10}; 
	int Cmp_BW::evalPtn(Map &map, Col col)
	{
		Bit cnt[3];
		map.countPiece(cnt);
		maptoBoard(map, col);
		int eval_pr=Eval_PrTable[cnt[0]];
		//if (eval_pr==10) eval_pr=9;
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
		Col co1 = col_f(col);
		Val loc_sco = 0;
		if (map[9] == col) loc_sco -= 1.5;
		else if (map[9] == co1) loc_sco += 1.5;
		if (map[14] == col) loc_sco -= 1.5;
		else if (map[14] == co1) loc_sco += 1.5;
		if (map[49] == col) loc_sco -= 1.5;
		else if (map[49] == co1) loc_sco += 1.5;
		if (map[54] == col) loc_sco -= 1.5;
		else if (map[54] == co1) loc_sco += 1.5;
		
		if (map[0] == col) loc_sco += 2.5;
		else if (map[0] == co1) loc_sco -= 2.5;
		if (map[7] == col) loc_sco += 2.5;
		else if (map[7] == co1) loc_sco -= 2.5;
		if (map[56] == col) loc_sco += 2.5;
		else if (map[56] == co1) loc_sco -= 2.5;
		if (map[63] == col) loc_sco += 2.5;
		else if (map[63] == co1) loc_sco -= 2.5;
		if (cnt[0]<40) loc_sco*=(cnt[0]+1.0f)/40.0f;
		/*
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
		return evalPtn(map, col)/256.0 + loc_sco;
	}

	Cmp_BW::Val Cmp_BW::getEndVal(Map &map, Col col)
	{
		Bit cnt[3]; map.countPiece(cnt);
		return (cnt[col] - cnt[col_f(col)]);
	}

}

using namespace bwcore;
 
int main()
{
	initPtnData();
	Map map=MP_SS;
	int _, col=2, n=0; cin>>_;
 	Cmp_BW cmp(Cmp_BW::Mode::exact);
	while (1){
		int x, y; cin>>x>>y;
		if (x!=-1) map.setPiece(x*8+y, col), col=col%2+1;
		else if (n) col=col%2+1;
		
		if (!map.testAll(col))
			cout<<"-1 -1\n";
		else{
			Ploc p=cmp.solve(map,col);
			cout<<(int)p.x<<' '<<(int)p.y<<'\n';
			map.setPiece(p.toMP(), col);
		}
		cout<<"debug col "<<col<<"\n";
		col=col%2+1;
		cout<<"\n\n";
		cout<<">>>BOTZONE_REQUEST_KEEP_RUNNING<<<\n";
		fflush(stdout);
		n++;
		#ifdef LOG_PRINT
		logRefrsh();
		#endif 
	}
}
