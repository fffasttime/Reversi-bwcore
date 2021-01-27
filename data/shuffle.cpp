#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <queue>
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
			s+=((*this)[p]+'0');
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
	void toArr(Col arr[8][8]);
	void resetByArr(Col arr[8][8]);
	
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

void Map::toArr(Col arr[8][8])
{
	for (int i=0;i<8;i++)
		for (int j=0;j<8;j++)
			arr[i][j]=(*this)[Ploc(i,j)];
}
void Map::resetByArr(Col arr[8][8])
{
	for (int i=0;i<8;i++)
		for (int j=0;j<8;j++)
			(*this).set(Ploc(i,j).toMP(),arr[i][j]);
}
}

void readShort(FILE *stream, short &tar)
{
	fread(&tar, sizeof(tar), 1, stream);
}

const int max_size = 200000;

struct Data
{
	ull r1,r2;
	int col, val;
}data[max_size];

int ccnt;

#include <algorithm>
int main()
{
	ifstream fin("reversidata44_50");
	ofstream fout("reversidata_44_50");
	srand(1);
	//int col;
	while (fin>>data[ccnt].col)
	{
		fin>>data[ccnt].r1 >>data[ccnt].r2 >>data[ccnt].val;
		ccnt++;
	}
	cout << ccnt << " board loaded\n" << '\n';
	std::random_shuffle(data,data+ccnt);
	for (int i=0;i<ccnt;i++)
	{
		fout <<data[i].col<<' '<<data[i].r1<<' ' <<data[i].r2<<' ' <<data[i].val<<'\n';
	}
	/*
	float x,maxx=-10000,minx=10000;
	ifstream fin("trained11_13.bin");
	while (fin>>x)
	{
		if (x>maxx) maxx=x;
		if (x<minx) minx=x;
	}
	cout<<maxx<<' '<<minx<<'\n';*/
	
	return 0;
}
