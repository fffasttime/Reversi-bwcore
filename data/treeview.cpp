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
#include <iomanip>
#include <windows.h>
//#include "jsoncpp/json.h"
using namespace std;

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

inline void setbit(ull &x, ull c, Bit p)
{
	x = (x&~(1ull << p)) | (c << p);
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
		return col % 2 + 1;
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
	const MP MP_F = 0, MP_E = 64;
	const MP MP_UL = -9, MP_U = -8, MP_UR = -7, MP_L = -1, MP_R = 1, MP_DL = 7, MP_D = 8, MP_DR = 9;

	struct Ploc
	{
		Bit x, y;
		Ploc() {}
		Ploc(MP mp) {
			x = mp / 8; y = mp % 8;
		}
		Ploc(Bit _x, Bit _y) :x(_x), y(_y) {}
		MP toMP() {
			return x * 8 + y;
		}
		Ploc& operator+=(const Ploc &p1)
		{
			x += p1.x;
			y += p1.y;
			return *this;
		}
		Ploc operator+(const Ploc &p1) const
		{
			return Ploc(x + p1.x, y + p1.y);
		}
		Ploc& operator++()
		{
			if (y == 8) x++;
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
			return Ploc(x - p1.x, y - p1.y);
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
		Map() {}
		Map(Col _m[64])
		{
			clear();
			for (int i = 0;i<64;i++)
			{
				if (_m[i] == C_W) m[0] |= 1ull << i;
				else if (_m[i] == C_B) m[1] |= 1ull << i;
			}
		}
		void clear() {
			m[0] = m[1] = 0;
		}
		inline Col operator[](MP p) {
			return ((m[0] >> p) & 1) + ((m[1] >> p) & 1) * 2;
		}
		inline Col operator[](Ploc p) {
			return (*this)[p.toMP()];
		}
		inline bool operator==(Map &op)
		{
			return m[0] == op.m[0] && m[1] == op.m[1];
		}
		inline bool isPlaced(MP p) {
			return ((m[0] | m[1]) >> p) & 1;
		}
		inline void set(MP p, Col col) {
			setbit(m[0], col == C_W, p);
			setbit(m[1], col == C_B, p);
		}
		string toString()
		{
			string s;
			for (auto p = MP_F;p<MP_E;p++)
			{
				s += (*this)[p] + '0';
				s += ' ';
				if (p % 8 == 7) s = s + "\n";
			}
			return s;
		}
		ull hashcode() {
			return (m[0])*(m[1]);
		}
		bool testPiece(MP p, Col col);
		void setPiece(MP p, Col col);
		bool testAll(Col col);
		void countPiece(Bit cnt[3]);
		void getCanDoList(Col col, vector<MP> &pList);
		Bit getFlipCount(MP p, Col col);
		
		friend istream &operator>>(istream& in, Map &map)
		{
			in>>map.m[0]>>map.m[1];
			return in;
		}
		friend ostream &operator<<(ostream& out, Map &map)
		{
			out<<map.m[0]<<' '<<map.m[1];
			return out;
		}
		static const Map Map_Start;
	};

	Col MP_SS[64] =
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
	const Map Map::Map_Start = Map(MP_SS);

	void Map::getCanDoList(Col col, vector<MP> &pList)
	{
		for (auto p = MP_F;p<MP_E;p++)
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

	bool Map::testPiece(MP p, Col col)
	{
		if (isPlaced(p)) return false;
		MP tp; auto co1 = col_f(col);
		//1:UL
		if (p % 8>1 && p >= 16 && (*this)[p + MP_UL] == co1)
			for (tp = p + MP_UL * 2;;tp += MP_UL) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp % 8 == 0 || tp<8) break;
			}
		//2:U
		if (p >= 16 && (*this)[p + MP_U] == co1)
			for (tp = p + MP_U * 2;;tp += MP_U) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp<8) break;
			}
		//3:UR
		if (p % 8<6 && p >= 16 && (*this)[p + MP_UR] == co1)
			for (tp = p + MP_UR * 2;;tp += MP_UR) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp % 8 == 7 || tp<8) break;
			}
		//4:L
		if (p % 8>1 && (*this)[p + MP_L] == co1)
			for (tp = p + MP_L * 2;;tp += MP_L) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp % 8 == 0) break;
			}
		//5:R
		if (p % 8<6 && (*this)[p + MP_R] == co1)
			for (tp = p + MP_R * 2;;tp += MP_R) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp % 8 == 7) break;
			}
		//5:DL
		if (p % 8>1 && p<48 && (*this)[p + MP_DL] == co1)
			for (tp = p + MP_DL * 2;;tp += MP_DL) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp % 8 == 0 || tp >= 56) break;
			}
		//6:D
		if (p<48 && (*this)[p + MP_D] == co1)
			for (tp = p + MP_D * 2;;tp += MP_D) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp >= 56) break;
			}
		//7:DR
		if (p % 8<6 && p<48 && (*this)[p + MP_DR] == co1)
			for (tp = p + MP_DR * 2;;tp += MP_DR) {
				Col cc = (*this)[tp];
				if (cc == col) return true;
				else if (cc == C_E) break;
				if (tp % 8 == 7 || tp >= 56) break;
			}
		return false;
	}

	Bit Map::getFlipCount(MP p, Col col)
	{
		if (isPlaced(p)) return 0;
		MP tp; auto co1 = col_f(col);
		Bit ans = 0;
		//1:UL
		if (p % 8>1 && p >= 16 && (*this)[p + MP_UL] == co1)
			for (tp = p + MP_UL * 2;;tp += MP_UL) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_UL - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 0 || tp<8) break;
			}
		//2:U
		if (p >= 16 && (*this)[p + MP_U] == co1)
			for (tp = p + MP_U * 2;;tp += MP_U) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_U - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp<8) break;
			}
		//3:UR
		if (p % 8<6 && p >= 16 && (*this)[p + MP_UR] == co1)
			for (tp = p + MP_UR * 2;;tp += MP_UR) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_UR - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 7 || tp<8) break;
			}
		//4:L
		if (p % 8>1 && (*this)[p + MP_L] == co1)
			for (tp = p + MP_L * 2;;tp += MP_L) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_L - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 0) break;
			}
		//5:R
		if (p % 8<6 && (*this)[p + MP_R] == co1)
			for (tp = p + MP_R * 2;;tp += MP_R) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_R - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 7) break;
			}
		//6:DL
		if (p % 8>1 && p<48 && (*this)[p + MP_DL] == co1)
			for (tp = p + MP_DL * 2;;tp += MP_DL) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_DL - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 0 || tp >= 56) break;
			}
		//7:D
		if (p<48 && (*this)[p + MP_D] == co1)
			for (tp = p + MP_D * 2;;tp += MP_D) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_D - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp >= 56) break;
			}
		//8:DR
		if (p % 8<6 && p<48 && (*this)[p + MP_DR] == co1)
			for (tp = p + MP_DR * 2;;tp += MP_DR) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					ans += (tp - p) / MP_DR - 1;
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 7 || tp >= 56) break;
			}
		return ans;
	}

	void Map::setPiece(MP p, Col col)
	{
		MP tp; auto co1 = col_f(col);
		set(p, col);
		//1:UL
		if (p % 8>1 && p >= 16 && (*this)[p + MP_UL] == co1)
			for (tp = p + MP_UL * 2;;tp += MP_UL) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_UL;tp != p;tp -= MP_UL) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 0 || tp<8) break;
			}
		//2:U
		if (p >= 16 && (*this)[p + MP_U] == co1)
			for (tp = p + MP_U * 2;;tp += MP_U) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_U;tp != p;tp -= MP_U) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp<8) break;
			}
		//3:UR
		if (p % 8<6 && p >= 16 && (*this)[p + MP_UR] == co1)
			for (tp = p + MP_UR * 2;;tp += MP_UR) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_UR;tp != p;tp -= MP_UR) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 7 || tp<8) break;
			}
		//4:L
		if (p % 8>1 && (*this)[p + MP_L] == co1)
			for (tp = p + MP_L * 2;;tp += MP_L) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_L;tp != p;tp -= MP_L) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 0) break;
			}
		//5:R
		if (p % 8<6 && (*this)[p + MP_R] == co1)
			for (tp = p + MP_R * 2;;tp += MP_R) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_R;tp != p;tp -= MP_R) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 7) break;
			}
		//6:DL
		if (p % 8>1 && p<48 && (*this)[p + MP_DL] == co1)
			for (tp = p + MP_DL * 2;;tp += MP_DL) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_DL;tp != p;tp -= MP_DL) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 0 || tp >= 56) break;
			}
		//7:D
		if (p<48 && (*this)[p + MP_D] == co1)
			for (tp = p + MP_D * 2;;tp += MP_D) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_D;tp != p;tp -= MP_D) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp >= 56) break;
			}
		//8:DR
		if (p % 8<6 && p<48 && (*this)[p + MP_DR] == co1)
			for (tp = p + MP_DR * 2;;tp += MP_DR) {
				Col cc = (*this)[tp];
				if (cc == col)
				{
					for (tp -= MP_DR;tp != p;tp -= MP_DR) (*this).set(tp, col);
					break;
				}
				else if (cc == C_E) break;
				if (tp % 8 == 7 || tp >= 56) break;
			}
	}

	bool Map::testAll(Col col)
	{
		for (auto p = MP_F;p<MP_E;p++)
			if (testPiece(p, col)) return true;
		return false;
	}

	void Map::countPiece(Bit cnt[3])
	{
		cnt[C_W] = popcount(m[MC_W]);
		cnt[C_B] = popcount(m[MC_B]);
		cnt[C_E] = 64 - cnt[C_W] - cnt[C_B];
	}

	template <typename Type, typename Data>
	class Cache
	{
	public:
		struct Node
		{
			Type s;
			Data data;
		};
	private:
		static const int P = 999983, MAX_TB = 2000000;
		int size;
		Node *tbl;
	public:
		Cache(int _size)
		{
			size = _size;
			assert(size>0);
			if (size>MAX_TB) size = MAX_TB;
			tbl = new Node[size];
		}
		~Cache()
		{
			delete[] tbl;
		}
		Node& find(ull hashcode)
		{
			hashcode %= P;
			return tbl[hashcode];
		}
		void clear()
		{
			memset(tbl, 0, sizeof(Node)*size);
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
			Node()=default;
			/*
			Node(const Node &from)
			{
				val=from.val;
				fa=from.fa;
				p=from.p;
				child.assign(from.child.begin(),from.child.end());
			}*/
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
			Node tnode;
			in>>tnode;
			tree.nodes[0]=tnode;
			for (int i=0;i<size-1;i++)
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
		}
		void nodeinfo()
		{
			cout<<"cur:"<<cur<<' '<<nodes[cur].val<<"  child:"<<nodes[cur].child.size()<<'\n';
			int c=0;
			for (auto &it:nodes[cur].child)
			{
				cout<<setw(2)<<c<<' '<<setw(6)<<it<<' '<<setw(6)<<setprecision(4)<<nodes[it].val<<' '<<setw(2)<<nodes[it].p<<'\n';
				c++;
			}
		}
	};
	
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
	#define EVAL_FILE "reversicoeff.bin"
	void readShort(FILE *stream, short &tar)
	{
		fread(&tar, 2, 1, stream);
	}
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
			cout<<pdata[i].wb<<'\n';
			cout<<pdata[i].k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	int board[128];
	int getMob(Map &map, Col col)
	{
		int ret = 0;
		for (auto i = MP_F;i<MP_E;i++)
			if (map.testPiece(i, col))
				ret++;
		return ret;
	}
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
					board[i*10+j+11]=col_f(map[i*8+j]);
		}
	}
	int Eval_PrTable[61]=
	{-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,
	7,7,7,7,7,8,8,8,8,8,9,9,9,9,9,9,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,11,11,11}; 
	int evalPtn(Map &map, Col col)
	{
		Bit cnt[3];
		map.countPiece(cnt);
		maptoBoard(map, col);
		int eval_pr=Eval_PrTable[cnt[0]];
		
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
}
using namespace bwcore;

Map map;
#define DEBUG_TREE_FILE "debugtreeinfo.txt"

vector<Map> mplist;
vector<int> collist;
Tree tree;

int runCommand()
{
	string s;
	cin>>s;
	if (s=="exit") return -1;
	if (s=="tr")
	{
		int tar;
		cin>>tar;
		if (tar==-1) 
		{
			tree.goback();
			tree.nodeinfo();
			mplist.pop_back();
			collist.pop_back();
		}
		else
		{
			if (tar>tree.nodes[tree.cur].child.size()) return 0;
			tree.cur=tree.nodes[tree.cur].child[tar];
			Map tmap=mplist.back();
			tmap.setPiece(tree.nodes[tree.cur].p,collist.back());
			cout<<tmap.toString()<<"val:"<<evalPtn(tmap, collist.back()%2+1)/256.0f<<'\n';
			mplist.push_back(tmap);
			collist.push_back(collist.back()%2+1);
			tree.nodeinfo();
		}
	}
	return 0;
}

int main()
{
	initPtnData();
	ifstream fin(DEBUG_TREE_FILE);
	ifstream fmap("debugtreerootmap.txt");
	fmap>>map;
	cout<<map.toString();
	mplist.push_back(map);
	collist.push_back(2);
	fin>>tree;
	tree.nodeinfo();
	while (1)
	{
		int ret=runCommand();
		if (ret==-1) break;
	}
	return 0;
}
