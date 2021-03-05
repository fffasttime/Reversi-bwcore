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
#include <iomanip>
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
				s += ((*this)[p] + '0');
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
		void toArr(Col arr[8][8]);
		void resetByArr(Col arr[8][8]);

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

	void Map::toArr(Col arr[8][8])
	{
		for (int i = 0;i<8;i++)
			for (int j = 0;j<8;j++)
				arr[i][j] = (*this)[Ploc(i, j)];
	}
	void Map::resetByArr(Col arr[8][8])
	{
		for (int i = 0;i<8;i++)
			for (int j = 0;j<8;j++)
				(*this).set(Ploc(i, j).toMP(), arr[i][j]);
	}
}
using namespace bwcore;

int ptnhash(int a[], int l)
{
	int ha = 0;
	for (int i = 0;i<l;i++) ha = ha * 3 + a[i];
	return ha;
}

const int max_size = 200000;
float le = 0.018;

int ccnt;
Bit mm[max_size][8][8];
int mcol[max_size], batch_size = 500;
float mval[max_size];
float we1[59049], w52[59049], w33[19683], we2[6561], we3[6561], we4[6561], wk8[6561], wk7[2187], wk6[729], wk5[243], wk4[81], wb, wib, wmob, wcnt;
short bwe1[59049], bw52[59049], bw33[19683], bwe2[6561], bwe3[6561], bwe4[6561], bwk8[6561], bwk7[2187], bwk6[729], bwk5[243], bwk4[81], bwb, bwib, bwmob, bwcnt;
float rwe1[59049], rw52[59049], rw33[19683], rwe2[6561], rwe3[6561], rwe4[6561], rwk8[6561], rwk7[2187], rwk6[729], rwk5[243], rwk4[81], rwb, rwib, rwmob, rwcnt;

int ptn52[8][10], ptn33[4][9], ptne1[4][10], ptne2[4][8], ptne3[4][8], ptne4[4][8], ptnk8[2][8], ptnk7[4][7], ptnk6[4][6], ptnk5[4][5], ptnk4[4][4];
int ptn33_[4][9], ptne1_[4][10], ptne2_[4][8], ptne3_[4][8], ptne4_[4][8], ptnk8_[2][8], ptnk7_[4][7], ptnk6_[4][6], ptnk5_[4][5], ptnk4_[4][4];

void getptns(int num)
{
	ptne1[0][0] = mm[num][0][0];
	ptne1[0][1] = mm[num][1][1];
	ptne1[0][2] = mm[num][0][1];
	ptne1[0][3] = mm[num][0][2];
	ptne1[0][4] = mm[num][0][3];
	ptne1[0][5] = mm[num][0][4];
	ptne1[0][6] = mm[num][0][5];
	ptne1[0][7] = mm[num][0][6];
	ptne1[0][8] = mm[num][1][6];
	ptne1[0][9] = mm[num][0][7];

	ptne1[1][0] = mm[num][7][0];
	ptne1[1][1] = mm[num][6][1];
	ptne1[1][2] = mm[num][7][1];
	ptne1[1][3] = mm[num][7][2];
	ptne1[1][4] = mm[num][7][3];
	ptne1[1][5] = mm[num][7][4];
	ptne1[1][6] = mm[num][7][5];
	ptne1[1][7] = mm[num][7][6];
	ptne1[1][8] = mm[num][6][6];
	ptne1[1][9] = mm[num][7][7];

	ptne1[2][0] = mm[num][0][0];
	ptne1[2][1] = mm[num][1][1];
	ptne1[2][2] = mm[num][1][0];
	ptne1[2][3] = mm[num][2][0];
	ptne1[2][4] = mm[num][3][0];
	ptne1[2][5] = mm[num][4][0];
	ptne1[2][6] = mm[num][5][0];
	ptne1[2][7] = mm[num][6][0];
	ptne1[2][8] = mm[num][6][1];
	ptne1[2][9] = mm[num][7][0];

	ptne1[3][0] = mm[num][0][7];
	ptne1[3][1] = mm[num][1][6];
	ptne1[3][2] = mm[num][1][7];
	ptne1[3][3] = mm[num][2][7];
	ptne1[3][4] = mm[num][3][7];
	ptne1[3][5] = mm[num][4][7];
	ptne1[3][6] = mm[num][5][7];
	ptne1[3][7] = mm[num][6][7];
	ptne1[3][8] = mm[num][6][6];
	ptne1[3][9] = mm[num][7][7];

	ptne2[0][0] = mm[num][1][0];
	ptne2[0][1] = mm[num][1][1];
	ptne2[0][2] = mm[num][1][2];
	ptne2[0][3] = mm[num][1][3];
	ptne2[0][4] = mm[num][1][4];
	ptne2[0][5] = mm[num][1][5];
	ptne2[0][6] = mm[num][1][6];
	ptne2[0][7] = mm[num][1][7];

	ptne2[1][0] = mm[num][6][0];
	ptne2[1][1] = mm[num][6][1];
	ptne2[1][2] = mm[num][6][2];
	ptne2[1][3] = mm[num][6][3];
	ptne2[1][4] = mm[num][6][4];
	ptne2[1][5] = mm[num][6][5];
	ptne2[1][6] = mm[num][6][6];
	ptne2[1][7] = mm[num][6][7];

	ptne2[2][0] = mm[num][0][1];
	ptne2[2][1] = mm[num][1][1];
	ptne2[2][2] = mm[num][2][1];
	ptne2[2][3] = mm[num][3][1];
	ptne2[2][4] = mm[num][4][1];
	ptne2[2][5] = mm[num][5][1];
	ptne2[2][6] = mm[num][6][1];
	ptne2[2][7] = mm[num][7][1];

	ptne2[3][0] = mm[num][0][6];
	ptne2[3][1] = mm[num][1][6];
	ptne2[3][2] = mm[num][2][6];
	ptne2[3][3] = mm[num][3][6];
	ptne2[3][4] = mm[num][4][6];
	ptne2[3][5] = mm[num][5][6];
	ptne2[3][6] = mm[num][6][6];
	ptne2[3][7] = mm[num][7][6];

	ptne3[0][0] = mm[num][2][0];
	ptne3[0][1] = mm[num][2][1];
	ptne3[0][2] = mm[num][2][2];
	ptne3[0][3] = mm[num][2][3];
	ptne3[0][4] = mm[num][2][4];
	ptne3[0][5] = mm[num][2][5];
	ptne3[0][6] = mm[num][2][6];
	ptne3[0][7] = mm[num][2][7];

	ptne3[1][0] = mm[num][5][0];
	ptne3[1][1] = mm[num][5][1];
	ptne3[1][2] = mm[num][5][2];
	ptne3[1][3] = mm[num][5][3];
	ptne3[1][4] = mm[num][5][4];
	ptne3[1][5] = mm[num][5][5];
	ptne3[1][6] = mm[num][5][6];
	ptne3[1][7] = mm[num][5][7];

	ptne3[2][0] = mm[num][0][2];
	ptne3[2][1] = mm[num][1][2];
	ptne3[2][2] = mm[num][2][2];
	ptne3[2][3] = mm[num][3][2];
	ptne3[2][4] = mm[num][4][2];
	ptne3[2][5] = mm[num][5][2];
	ptne3[2][6] = mm[num][6][2];
	ptne3[2][7] = mm[num][7][2];

	ptne3[3][0] = mm[num][0][5];
	ptne3[3][1] = mm[num][1][5];
	ptne3[3][2] = mm[num][2][5];
	ptne3[3][3] = mm[num][3][5];
	ptne3[3][4] = mm[num][4][5];
	ptne3[3][5] = mm[num][5][5];
	ptne3[3][6] = mm[num][6][5];
	ptne3[3][7] = mm[num][7][5];

	ptne4[0][0] = mm[num][3][0];
	ptne4[0][1] = mm[num][3][1];
	ptne4[0][2] = mm[num][3][2];
	ptne4[0][3] = mm[num][3][3];
	ptne4[0][4] = mm[num][3][4];
	ptne4[0][5] = mm[num][3][5];
	ptne4[0][6] = mm[num][3][6];
	ptne4[0][7] = mm[num][3][7];

	ptne4[1][0] = mm[num][4][0];
	ptne4[1][1] = mm[num][4][1];
	ptne4[1][2] = mm[num][4][2];
	ptne4[1][3] = mm[num][4][3];
	ptne4[1][4] = mm[num][4][4];
	ptne4[1][5] = mm[num][4][5];
	ptne4[1][6] = mm[num][4][6];
	ptne4[1][7] = mm[num][4][7];

	ptne4[2][0] = mm[num][0][3];
	ptne4[2][1] = mm[num][1][3];
	ptne4[2][2] = mm[num][2][3];
	ptne4[2][3] = mm[num][3][3];
	ptne4[2][4] = mm[num][4][3];
	ptne4[2][5] = mm[num][5][3];
	ptne4[2][6] = mm[num][6][3];
	ptne4[2][7] = mm[num][7][3];

	ptne4[3][0] = mm[num][0][4];
	ptne4[3][1] = mm[num][1][4];
	ptne4[3][2] = mm[num][2][4];
	ptne4[3][3] = mm[num][3][4];
	ptne4[3][4] = mm[num][4][4];
	ptne4[3][5] = mm[num][5][4];
	ptne4[3][6] = mm[num][6][4];
	ptne4[3][7] = mm[num][7][4];

	ptnk8[0][0] = mm[num][0][0];
	ptnk8[0][1] = mm[num][1][1];
	ptnk8[0][2] = mm[num][2][2];
	ptnk8[0][3] = mm[num][3][3];
	ptnk8[0][4] = mm[num][4][4];
	ptnk8[0][5] = mm[num][5][5];
	ptnk8[0][6] = mm[num][6][6];
	ptnk8[0][7] = mm[num][7][7];

	ptnk8[1][0] = mm[num][0][7];
	ptnk8[1][1] = mm[num][1][6];
	ptnk8[1][2] = mm[num][2][5];
	ptnk8[1][3] = mm[num][3][4];
	ptnk8[1][4] = mm[num][4][3];
	ptnk8[1][5] = mm[num][5][2];
	ptnk8[1][6] = mm[num][6][1];
	ptnk8[1][7] = mm[num][7][0];

	ptnk7[0][0] = mm[num][0][1];
	ptnk7[0][1] = mm[num][1][2];
	ptnk7[0][2] = mm[num][2][3];
	ptnk7[0][3] = mm[num][3][4];
	ptnk7[0][4] = mm[num][4][5];
	ptnk7[0][5] = mm[num][5][6];
	ptnk7[0][6] = mm[num][6][7];

	ptnk7[1][0] = mm[num][1][0];
	ptnk7[1][1] = mm[num][2][1];
	ptnk7[1][2] = mm[num][3][2];
	ptnk7[1][3] = mm[num][4][3];
	ptnk7[1][4] = mm[num][5][4];
	ptnk7[1][5] = mm[num][6][5];
	ptnk7[1][6] = mm[num][7][6];

	ptnk7[2][0] = mm[num][0][6];
	ptnk7[2][1] = mm[num][1][5];
	ptnk7[2][2] = mm[num][2][4];
	ptnk7[2][3] = mm[num][3][3];
	ptnk7[2][4] = mm[num][4][2];
	ptnk7[2][5] = mm[num][5][1];
	ptnk7[2][6] = mm[num][6][0];

	ptnk7[3][0] = mm[num][1][7];
	ptnk7[3][1] = mm[num][2][6];
	ptnk7[3][2] = mm[num][3][5];
	ptnk7[3][3] = mm[num][4][4];
	ptnk7[3][4] = mm[num][5][3];
	ptnk7[3][5] = mm[num][6][2];
	ptnk7[3][6] = mm[num][7][1];

	ptnk6[0][0] = mm[num][0][2];
	ptnk6[0][1] = mm[num][1][3];
	ptnk6[0][2] = mm[num][2][4];
	ptnk6[0][3] = mm[num][3][5];
	ptnk6[0][4] = mm[num][4][6];
	ptnk6[0][5] = mm[num][5][7];

	ptnk6[1][0] = mm[num][2][0];
	ptnk6[1][1] = mm[num][3][1];
	ptnk6[1][2] = mm[num][4][2];
	ptnk6[1][3] = mm[num][5][3];
	ptnk6[1][4] = mm[num][6][4];
	ptnk6[1][5] = mm[num][7][5];

	ptnk6[2][0] = mm[num][0][5];
	ptnk6[2][1] = mm[num][1][4];
	ptnk6[2][2] = mm[num][2][3];
	ptnk6[2][3] = mm[num][3][2];
	ptnk6[2][4] = mm[num][4][1];
	ptnk6[2][5] = mm[num][5][0];

	ptnk6[3][0] = mm[num][2][7];
	ptnk6[3][1] = mm[num][3][6];
	ptnk6[3][2] = mm[num][4][5];
	ptnk6[3][3] = mm[num][5][4];
	ptnk6[3][4] = mm[num][6][3];
	ptnk6[3][5] = mm[num][7][2];

	ptnk5[0][0] = mm[num][0][3];
	ptnk5[0][1] = mm[num][1][4];
	ptnk5[0][2] = mm[num][2][5];
	ptnk5[0][3] = mm[num][3][6];
	ptnk5[0][4] = mm[num][4][7];

	ptnk5[1][0] = mm[num][3][0];
	ptnk5[1][1] = mm[num][4][1];
	ptnk5[1][2] = mm[num][5][2];
	ptnk5[1][3] = mm[num][6][3];
	ptnk5[1][4] = mm[num][7][4];

	ptnk5[2][0] = mm[num][0][4];
	ptnk5[2][1] = mm[num][1][3];
	ptnk5[2][2] = mm[num][2][2];
	ptnk5[2][3] = mm[num][3][1];
	ptnk5[2][4] = mm[num][4][0];

	ptnk5[3][0] = mm[num][3][7];
	ptnk5[3][1] = mm[num][4][6];
	ptnk5[3][2] = mm[num][5][5];
	ptnk5[3][3] = mm[num][6][4];
	ptnk5[3][4] = mm[num][7][3];

	ptnk4[0][0] = mm[num][0][4];
	ptnk4[0][1] = mm[num][1][5];
	ptnk4[0][2] = mm[num][2][6];
	ptnk4[0][3] = mm[num][3][7];

	ptnk4[1][0] = mm[num][4][0];
	ptnk4[1][1] = mm[num][5][1];
	ptnk4[1][2] = mm[num][6][2];
	ptnk4[1][3] = mm[num][7][3];

	ptnk4[2][0] = mm[num][0][3];
	ptnk4[2][1] = mm[num][1][2];
	ptnk4[2][2] = mm[num][2][1];
	ptnk4[2][3] = mm[num][3][0];

	ptnk4[3][0] = mm[num][4][7];
	ptnk4[3][1] = mm[num][5][6];
	ptnk4[3][2] = mm[num][6][5];
	ptnk4[3][3] = mm[num][7][4];

	ptn33[0][0] = mm[num][0][0];
	ptn33[0][1] = mm[num][0][1];
	ptn33[0][2] = mm[num][0][2];
	ptn33[0][3] = mm[num][1][0];
	ptn33[0][4] = mm[num][1][1];
	ptn33[0][5] = mm[num][1][2];
	ptn33[0][6] = mm[num][2][0];
	ptn33[0][7] = mm[num][2][1];
	ptn33[0][8] = mm[num][2][2];

	ptn33[1][0] = mm[num][0][7];
	ptn33[1][1] = mm[num][0][6];
	ptn33[1][2] = mm[num][0][5];
	ptn33[1][3] = mm[num][1][7];
	ptn33[1][4] = mm[num][1][6];
	ptn33[1][5] = mm[num][1][5];
	ptn33[1][6] = mm[num][2][7];
	ptn33[1][7] = mm[num][2][6];
	ptn33[1][8] = mm[num][2][5];

	ptn33[2][0] = mm[num][7][0];
	ptn33[2][1] = mm[num][7][1];
	ptn33[2][2] = mm[num][7][2];
	ptn33[2][3] = mm[num][6][0];
	ptn33[2][4] = mm[num][6][1];
	ptn33[2][5] = mm[num][6][2];
	ptn33[2][6] = mm[num][5][0];
	ptn33[2][7] = mm[num][5][1];
	ptn33[2][8] = mm[num][5][2];

	ptn33[3][0] = mm[num][7][7];
	ptn33[3][1] = mm[num][7][6];
	ptn33[3][2] = mm[num][7][5];
	ptn33[3][3] = mm[num][6][7];
	ptn33[3][4] = mm[num][6][6];
	ptn33[3][5] = mm[num][6][5];
	ptn33[3][6] = mm[num][5][7];
	ptn33[3][7] = mm[num][5][6];
	ptn33[3][8] = mm[num][5][5];

	ptn52[0][0] = mm[num][0][0];
	ptn52[0][1] = mm[num][0][1];
	ptn52[0][2] = mm[num][0][2];
	ptn52[0][3] = mm[num][0][3];
	ptn52[0][4] = mm[num][0][4];
	ptn52[0][5] = mm[num][1][0];
	ptn52[0][6] = mm[num][1][1];
	ptn52[0][7] = mm[num][1][2];
	ptn52[0][8] = mm[num][1][3];
	ptn52[0][9] = mm[num][1][4];

	ptn52[1][0] = mm[num][0][7];
	ptn52[1][1] = mm[num][0][6];
	ptn52[1][2] = mm[num][0][5];
	ptn52[1][3] = mm[num][0][4];
	ptn52[1][4] = mm[num][0][3];
	ptn52[1][5] = mm[num][1][7];
	ptn52[1][6] = mm[num][1][6];
	ptn52[1][7] = mm[num][1][5];
	ptn52[1][8] = mm[num][1][4];
	ptn52[1][9] = mm[num][1][3];

	ptn52[2][0] = mm[num][7][0];
	ptn52[2][1] = mm[num][7][1];
	ptn52[2][2] = mm[num][7][2];
	ptn52[2][3] = mm[num][7][3];
	ptn52[2][4] = mm[num][7][4];
	ptn52[2][5] = mm[num][6][0];
	ptn52[2][6] = mm[num][6][1];
	ptn52[2][7] = mm[num][6][2];
	ptn52[2][8] = mm[num][6][3];
	ptn52[2][9] = mm[num][6][4];

	ptn52[3][0] = mm[num][7][7];
	ptn52[3][1] = mm[num][7][6];
	ptn52[3][2] = mm[num][7][5];
	ptn52[3][3] = mm[num][7][4];
	ptn52[3][4] = mm[num][7][3];
	ptn52[3][5] = mm[num][6][7];
	ptn52[3][6] = mm[num][6][6];
	ptn52[3][7] = mm[num][6][5];
	ptn52[3][8] = mm[num][6][4];
	ptn52[3][9] = mm[num][6][3];

	ptn52[4][0] = mm[num][0][0];
	ptn52[4][1] = mm[num][1][0];
	ptn52[4][2] = mm[num][2][0];
	ptn52[4][3] = mm[num][3][0];
	ptn52[4][4] = mm[num][4][0];
	ptn52[4][5] = mm[num][0][1];
	ptn52[4][6] = mm[num][1][1];
	ptn52[4][7] = mm[num][2][1];
	ptn52[4][8] = mm[num][3][1];
	ptn52[4][9] = mm[num][4][1];

	ptn52[5][0] = mm[num][7][0];
	ptn52[5][1] = mm[num][6][0];
	ptn52[5][2] = mm[num][5][0];
	ptn52[5][3] = mm[num][4][0];
	ptn52[5][4] = mm[num][3][0];
	ptn52[5][5] = mm[num][7][1];
	ptn52[5][6] = mm[num][6][1];
	ptn52[5][7] = mm[num][5][1];
	ptn52[5][8] = mm[num][4][1];
	ptn52[5][9] = mm[num][3][1];

	ptn52[6][0] = mm[num][0][7];
	ptn52[6][1] = mm[num][1][7];
	ptn52[6][2] = mm[num][2][7];
	ptn52[6][3] = mm[num][3][7];
	ptn52[6][4] = mm[num][4][7];
	ptn52[6][5] = mm[num][0][6];
	ptn52[6][6] = mm[num][1][6];
	ptn52[6][7] = mm[num][2][6];
	ptn52[6][8] = mm[num][3][6];
	ptn52[6][9] = mm[num][4][6];

	ptn52[7][0] = mm[num][7][7];
	ptn52[7][1] = mm[num][6][7];
	ptn52[7][2] = mm[num][5][7];
	ptn52[7][3] = mm[num][4][7];
	ptn52[7][4] = mm[num][3][7];
	ptn52[7][5] = mm[num][7][6];
	ptn52[7][6] = mm[num][6][6];
	ptn52[7][7] = mm[num][5][6];
	ptn52[7][8] = mm[num][4][6];
	ptn52[7][9] = mm[num][3][6];
}

void getpartner()
{
	for (int i = 0;i<4;i++)
	{
		for (int j = 0;j < 10;j++)
			ptne1_[i][j] = ptne1[i][9 - j];
		for (int j = 0;j < 8;j++)
		{
			ptne2_[i][j] = ptne2[i][7 - j];
			ptne3_[i][j] = ptne3[i][7 - j];
			ptne4_[i][j] = ptne4[i][7 - j];
		}
		for (int j = 0;j < 7;j++) ptnk7_[i][j] = ptnk7[i][6 - j];
		for (int j = 0;j < 6;j++) ptnk6_[i][j] = ptnk6[i][5 - j];
		for (int j = 0;j < 5;j++) ptnk5_[i][j] = ptnk5[i][4 - j];
		for (int j = 0;j < 4;j++) ptnk4_[i][j] = ptnk4[i][3 - j];
		ptn33_[i][0] = ptn33[i][0];
		ptn33_[i][4] = ptn33[i][4];
		ptn33_[i][8] = ptn33[i][8];
		ptn33_[i][1] = ptn33[i][3];
		ptn33_[i][3] = ptn33[i][1];
		ptn33_[i][2] = ptn33[i][6];
		ptn33_[i][6] = ptn33[i][2];
		ptn33_[i][5] = ptn33[i][7];
		ptn33_[i][7] = ptn33[i][5];

	}
	for (int i = 0;i<2;i++)
		for (int j = 0;j < 8;j++)
			ptnk8_[i][j] = ptnk8[i][7 - j];
}

void printb(int num)
{
	for (int i = 0;i<7;i++, cout << '\n')
		for (int j = 0;j<7;j++)
			cout << mm[num][i][j] << ' ';
}

float ssig = 0, sse = 0;
Map mmap;
Bit mcnt[3];

int getMob(int num)
{
	int ret = 0;
	for (auto i = MP_F;i<MP_E;i++)
		if (mmap.testPiece(i, 2))
			ret++;
	return ret;
}

int getOdd(int num)
{
	return mcnt[0] % 2;
}

void validf(int num)
{
	mmap.resetByArr(mm[num]); mmap.countPiece(mcnt);
	//cout<<mmap.toString(); 
	getptns(num);
	int pe1[4], pe2[4], pe3[4], pe4[4], pk8[2], pk7[4], pk6[4], pk5[4], pk4[4], p52[8], p33[4];
	for (int i = 0;i < 8;i++)
		p52[i] = ptnhash(ptn52[i], 10);
	for (int i = 0;i < 4;i++)
	{
		pe1[i] = ptnhash(ptne1[i], 10);
		pe2[i] = ptnhash(ptne2[i], 8);
		pe3[i] = ptnhash(ptne3[i], 8);
		pe4[i] = ptnhash(ptne4[i], 8);
		pk7[i] = ptnhash(ptnk7[i], 7);
		pk6[i] = ptnhash(ptnk6[i], 6);
		pk5[i] = ptnhash(ptnk5[i], 5);
		pk4[i] = ptnhash(ptnk4[i], 4);
		p33[i] = ptnhash(ptn33[i], 9);
	}
	for (int i = 0;i < 2;i++)
		pk8[i] = ptnhash(ptnk8[i], 8);
	float sigma = wb;
	for (int i = 0; i < 8; i++)
		sigma += w52[p52[i]];
	for (int i = 0;i < 4;i++)
	{
		sigma += we1[pe1[i]];
		sigma += we2[pe2[i]];
		sigma += we3[pe3[i]];
		sigma += we4[pe4[i]];
		sigma += wk7[pk7[i]];
		sigma += wk6[pk6[i]];
		sigma += wk5[pk5[i]];
		sigma += wk4[pk4[i]];
		sigma += w33[p33[i]];
	}
	for (int i = 0;i < 2;i++)
		sigma += wk8[pk8[i]];
	int cmob = getMob(num);
	int codd = getOdd(num);
	int cwcnt = mcnt[2] - mcnt[1];
	sigma += wmob*cmob;
	sigma += wib*codd;
	sigma += wcnt*cwcnt;

	float mse = (sigma - mval[num])*(sigma - mval[num]) / 2, delta = mval[num] - sigma;
	sse += fabs(delta);
}

void valid(int num)
{
	mmap.resetByArr(mm[num]); mmap.countPiece(mcnt);
	//cout<<mmap.toString(); 
	getptns(num); //getpartner();
	int pe1[4], pe2[4], pe3[4], pe4[4], pk8[2], pk7[4], pk6[4], pk5[4], pk4[4], p52[8], p33[4];
	for (int i = 0;i < 8;i++)
		p52[i] = ptnhash(ptn52[i], 10);
	for (int i = 0;i < 4;i++)
	{
		pe1[i] = ptnhash(ptne1[i], 10);
		pe2[i] = ptnhash(ptne2[i], 8);
		pe3[i] = ptnhash(ptne3[i], 8);
		pe4[i] = ptnhash(ptne4[i], 8);
		pk7[i] = ptnhash(ptnk7[i], 7);
		pk6[i] = ptnhash(ptnk6[i], 6);
		pk5[i] = ptnhash(ptnk5[i], 5);
		pk4[i] = ptnhash(ptnk4[i], 4);
		p33[i] = ptnhash(ptn33[i], 9);
	}
	for (int i = 0;i < 2;i++)
		pk8[i] = ptnhash(ptnk8[i], 8);
	float sigma = bwb;
	int cmob = getMob(num);
	int codd = getOdd(num);
	int cwcnt = mcnt[2] - mcnt[1];
	sigma += bwmob*cmob;
	sigma += bwib*codd;
	sigma += bwcnt*cwcnt;
	for (int i = 0; i < 8; i++) sigma += bw52[p52[i]];
	for (int i = 0;i < 4;i++)
	{
		sigma += bwe1[pe1[i]];
		sigma += bwe2[pe2[i]];
		sigma += bwe3[pe3[i]];
		sigma += bwe4[pe4[i]];
		sigma += bwk7[pk7[i]];
		sigma += bwk6[pk6[i]];
		sigma += bwk5[pk5[i]];
		sigma += bwk4[pk4[i]];
		sigma += bw33[p33[i]];
	}
	for (int i = 0;i < 2;i++) sigma += bwk8[pk8[i]];
	sigma/=256.0;

	float mse = (sigma - mval[num])*(sigma - mval[num]) / 2, delta = mval[num] - sigma;
	sse += fabs(delta);
}

void updateArg()
{
	float ee = le / batch_size;

	for (int i = 0;i < 59049;i++)
	{
		we1[i] += rwe1[i] * ee; 
		w52[i] += rw52[i] * ee;
	}
	for (int i = 0;i < 19683;i++)
		w33[i] += rw33[i] * ee;
	for (int i = 0;i < 6561;i++)
	{
		we2[i] += rwe2[i] * ee;
		we3[i] += rwe3[i] * ee;
		we4[i] += rwe4[i] * ee;
		wk8[i] += rwk8[i] * ee;
	}
	for (int i = 0;i<2187;i++)
		wk7[i] += rwk7[i] * ee;
	for (int i = 0;i<729;i++)
		wk6[i] += rwk6[i] * ee;
	for (int i = 0;i<243;i++)
		wk5[i] += rwk5[i] * ee;
	for (int i = 0;i<81;i++)
		wk4[i] += rwk4[i] * ee;
	wmob += rwmob*ee;
	wib += rwib*ee;
	wb += rwb*ee;
	wcnt += rwcnt*ee;

	for (int i = 0;i<59049;i++)
		rwe1[i] = rw52[i]= 0;
	for (int i = 0;i < 19683;i++)
		rw33[i] = 0;
	for (int i = 0;i<6561;i++)
		rwe2[i] = rwe3[i] = rwe4[i] = rwk8[i] = 0;
	for (int i = 0;i<2187;i++)
		rwk7[i] = 0;
	for (int i = 0;i<729;i++)
		rwk6[i] = 0;
	for (int i = 0;i<243;i++)
		rwk5[i] = 0;
	for (int i = 0;i<81;i++)
		rwk4[i] = 0;
	rwmob = rwib = rwb = rwcnt = 0;
}

void accuGrad(int num)
{
	mmap.resetByArr(mm[num]); mmap.countPiece(mcnt);
	//cout<<mmap.toString(); 
	getptns(num);
	getpartner();
	int pe1[4], pe2[4], pe3[4], pe4[4], pk8[2], pk7[4], pk6[4], pk5[4], pk4[4], p52[8], p33[4];
	int pe1_[4], pe2_[4], pe3_[4], pe4_[4], pk8_[2], pk7_[4], pk6_[4], pk5_[4], pk4_[4], p33_[4];
	for (int i = 0;i < 8;i++)
		p52[i] = ptnhash(ptn52[i], 10);
	for (int i = 0;i<4;i++)
	{
		pe1[i] = ptnhash(ptne1[i], 10);
		pe2[i] = ptnhash(ptne2[i], 8);
		pe3[i] = ptnhash(ptne3[i], 8);
		pe4[i] = ptnhash(ptne4[i], 8);
		pk7[i] = ptnhash(ptnk7[i], 7);
		pk6[i] = ptnhash(ptnk6[i], 6);
		pk5[i] = ptnhash(ptnk5[i], 5);
		pk4[i] = ptnhash(ptnk4[i], 4);
		p33[i] = ptnhash(ptn33[i], 9);

		pe1_[i] = ptnhash(ptne1_[i], 10);
		pe2_[i] = ptnhash(ptne2_[i], 8);
		pe3_[i] = ptnhash(ptne3_[i], 8);
		pe4_[i] = ptnhash(ptne4_[i], 8);
		pk7_[i] = ptnhash(ptnk7_[i], 7);
		pk6_[i] = ptnhash(ptnk6_[i], 6);
		pk5_[i] = ptnhash(ptnk5_[i], 5);
		pk4_[i] = ptnhash(ptnk4_[i], 4);
		p33_[i] = ptnhash(ptn33_[i], 9);
	}
	for (int i = 0;i < 2;i++)
	{
		pk8[i] = ptnhash(ptnk8[i], 8);
		pk8_[i] = ptnhash(ptnk8_[i], 8);
	}
	float sigma = wb;
	for (int i = 0;i < 8;i++)
		sigma += w52[p52[i]];

	for (int i = 0;i < 4;i++)
	{
		sigma += we1[pe1[i]];
		sigma += we2[pe2[i]];
		sigma += we3[pe3[i]];
		sigma += we4[pe4[i]];
		sigma += wk7[pk7[i]];
		sigma += wk6[pk6[i]];
		sigma += wk5[pk5[i]];
		sigma += wk4[pk4[i]];
		sigma += w33[p33[i]];
	}
	for (int i = 0;i < 2;i++)
		sigma += wk8[pk8[i]];
	int cmob = getMob(num);
	int codd = getOdd(num);
	int cwcnt = mcnt[2] - mcnt[1];
	sigma += wmob*cmob;
	sigma += wib*codd;
	sigma += wcnt*cwcnt;
	
	float mse = (sigma - mval[num])*(sigma - mval[num]) / 2, delta = mval[num] - sigma;
	sse += fabs(delta);

	for (int i = 0;i < 8;i++)
		rw52[p52[i]] += delta;

	for (int i = 0;i<4;i++)
	{
		rwe1[pe1[i]] += delta;
		rwe2[pe2[i]] += delta;
		rwe3[pe3[i]] += delta;
		rwe4[pe4[i]] += delta;
		rwk7[pk7[i]] += delta;
		rwk6[pk6[i]] += delta;
		rwk5[pk5[i]] += delta;
		rwk4[pk4[i]] += delta;
		rw33[p33[i]] += delta;

		rwe1[pe1_[i]] += delta;
		rwe2[pe2_[i]] += delta;
		rwe3[pe3_[i]] += delta;
		rwe4[pe4_[i]] += delta;
		rwk7[pk7_[i]] += delta;
		rwk6[pk6_[i]] += delta;
		rwk5[pk5_[i]] += delta;
		rwk4[pk4_[i]] += delta;
		rw33[p33_[i]] += delta;
	}
	for (int i = 0;i < 2;i++)
	{
		rwk8[pk8[i]] += delta;
		rwk8[pk8_[i]] += delta;
	}
	rwb += delta;
	rwmob += delta*cmob/2;
	rwib += delta*codd;
	//rwcnt += delta*cwcnt/16;
}

#define EVAL_FILE "trained44_50.bin"

void readShort(FILE *stream, short &tar)
{
	fread(&tar, sizeof(tar), 1, stream);
}

void writeShort(FILE *stream, short tar)
{
	fwrite(&tar, sizeof(tar), 1, stream);
}

void saveArgs()
{
#if 1 
	FILE *eval_stream=fopen(EVAL_FILE, "wb");
	writeShort(eval_stream, bwb);
	writeShort(eval_stream, bwib);
	writeShort(eval_stream, bwmob);
	fflush(eval_stream);
	for (int i = 0;i<59049;i++)
		writeShort(eval_stream, bwe1[i]);
	for (int i = 0;i<59049;i++)
		writeShort(eval_stream, bw52[i]);
	for (int i = 0;i<19683;i++)
		writeShort(eval_stream, bw33[i]);
	for (int i = 0;i<6561;i++)
		writeShort(eval_stream, bwe2[i]);
	for (int i = 0;i<6561;i++)
		writeShort(eval_stream, bwe3[i]);
	for (int i = 0;i<6561;i++)
		writeShort(eval_stream, bwe4[i]);
	for (int i = 0;i<6561;i++)
		writeShort(eval_stream, bwk8[i]);
	for (int i = 0;i<2187;i++)
		writeShort(eval_stream, bwk7[i]);
	for (int i = 0;i<729;i++)
		writeShort(eval_stream, bwk6[i]);
	for (int i = 0;i<243;i++)
		writeShort(eval_stream, bwk5[i]);
	for (int i = 0;i<81;i++)
		writeShort(eval_stream, bwk4[i]);
	fclose(eval_stream);
	cout<<"final saved arg:"<< bwk4[80]<<'\n';
#else
	ofstream foutp(EVAL_FILE);
	foutp << wb << ' ' << wib << ' ' << wmob << '\n';
	for (int i = 0;i<59049;i++)
		foutp << we1[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<59049;i++)
		foutp << w52[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<19683;i++)
		foutp << w33[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<6561;i++)
		foutp << we2[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<6561;i++)
		foutp << we3[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<6561;i++)
		foutp << we4[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<6561;i++)
		foutp << wk8[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<2187;i++)
		foutp << wk7[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<729;i++)
		foutp << wk6[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<243;i++)
		foutp << wk5[i] << ' ';
	foutp << '\n';
	for (int i = 0;i<81;i++)
		foutp << wk4[i] << ' ';
	foutp << '\n';
#endif
}

void loadArgs()
{
	FILE *eval_stream=fopen(EVAL_FILE, "rb");
	readShort(eval_stream, bwb);
	readShort(eval_stream, bwib);
	readShort(eval_stream, bwmob);
	for (int i = 0;i<59049;i++)
		readShort(eval_stream, bwe1[i]);
	for (int i = 0;i<59049;i++)
		readShort(eval_stream, bw52[i]);
	for (int i = 0;i<19683;i++)
		readShort(eval_stream, bw33[i]);
	for (int i = 0;i<6561;i++)
		readShort(eval_stream, bwe2[i]);
	for (int i = 0;i<6561;i++)
		readShort(eval_stream, bwe3[i]);
	for (int i = 0;i<6561;i++)
		readShort(eval_stream, bwe4[i]);
	for (int i = 0;i<6561;i++)
		readShort(eval_stream, bwk8[i]);
	for (int i = 0;i<2187;i++)
		readShort(eval_stream, bwk7[i]);
	for (int i = 0;i<729;i++)
		readShort(eval_stream, bwk6[i]);
	for (int i = 0;i<243;i++)
		readShort(eval_stream, bwk5[i]);
	for (int i = 0;i<81;i++)
		readShort(eval_stream, bwk4[i]);
	fclose(eval_stream);
	
	cout<<bwb<<'\n';
	cout<<bwk4[80]<<'\n';
}

void coeff_to_short()
{
	float ee=256;
	for (int i = 0;i < 59049;i++)
	{
		bwe1[i] = we1[i] * ee + 0.5f; 
		bw52[i] = w52[i] * ee + 0.5f;
	}
	for (int i = 0;i < 19683;i++)
		bw33[i] = w33[i] * ee + 0.5f;
	for (int i = 0;i < 6561;i++)
	{
		bwe2[i] = we2[i] * ee + 0.5f;
		bwe3[i] = we3[i] * ee + 0.5f;
		bwe4[i] = we4[i] * ee + 0.5f;
		bwk8[i] = wk8[i] * ee + 0.5f;
	}
	for (int i = 0;i<2187;i++)
		bwk7[i] = wk7[i] * ee + 0.5f;
	for (int i = 0;i<729;i++)
		bwk6[i] = wk6[i] * ee + 0.5f;
	for (int i = 0;i<243;i++)
		bwk5[i] = wk5[i] * ee + 0.5f;
	for (int i = 0;i<81;i++)
		bwk4[i] = wk4[i] * ee + 0.5f;
	bwmob = wmob*ee + 0.5f;
	bwib = wib*ee + 0.5f;
	bwb = wb*ee + 0.5f;
}

void train()
{
	float vy_ = 0;
	for (int i = 0;i<25000;i++)
	{
		for (int j = 0;j<batch_size;j++)
			accuGrad((i*batch_size + j) % ccnt);
		if (i % 200 == 199)
		{
			cout << i << ' ' << sse / (200 * batch_size) << '\n';
			sse = 0;
		}
		updateArg();
		if (i % 1000 == 999)
		{
			for (int j = 60000;j<61000;j++)
				valid(j);
			cout << "valid:   " << sse / 1000 << '\n';
		}
	}
	le/=4;
	for (int i = 25000;i<35000;i++)
	{
		for (int j = 0;j<batch_size;j++)
			accuGrad((i*batch_size + j) % ccnt);
		if (i % 200 == 199)
		{
			cout << i << ' ' << sse / (200 * batch_size) << '\n';
			sse = 0;
		}
		updateArg();/* 
		if (i % 1000 == 999)
		{
			for (int j = 60000;j<61000;j++)
				valid(j);
			cout << "valid:   " << sse / 1000 << '\n';
		}*/ 
	}
	printf("wb:%f\nwodd:%f\nwmob:%f\nwcnt:%f\n", wb, wib, wmob, wcnt);
	sse=0;
	for (int i = ccnt;i<ccnt+5000;i++)
	{
		validf(i);
		if (i % 1000 == 999)
		{
			cout << i << ' ' << sse / 1000 << '\n';
			sse = 0;
		}
	}
	cout << '\n';
	coeff_to_short();
}

int main()
{
	ifstream fin("data/rawdata1/data5_7.txt");
	//ofstream cout("linregdata.txt");
	int col, vy_=0;
	ull r1, r2;
	while (fin >>hex>> r1 >> r2)
	{
		mmap.m[1] = r1, mmap.m[0] = r2;
		mmap.toArr(mm[ccnt]);
		fin >> mval[ccnt];
		vy_ += mval[ccnt];
		ccnt++;
	}
	cout << ccnt << " board loaded\n" << vy_ / ccnt << '\n';
	ccnt=94000;
	#define TRAIN
	#ifdef TRAIN
	train();
	#else
	loadArgs();
	#endif
	sse=0;
	#if 0
	valid(60001);
	cout<<sse<<'\n';
	#else
	float ssse=0;
	for (int i = ccnt;i<ccnt+5000;i++)
	{
		valid(i);
		if (i % 1000 == 999)
		{
			cout << i << ' ' << sse / 1000 << '\n';
			ssse+=sse;
			sse = 0;
		}
	}
	cout<<ssse/5000<<'\n';
	#endif
	#ifdef TRAIN
	//saveArgs();
	#endif
	return 0;
}
  
