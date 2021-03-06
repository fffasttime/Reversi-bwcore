/*
metacode.cpp
generate constant arrays code
- deprecated because BMI2 flip code is deprecated
author: fffasttime
*/
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
using namespace std;

typedef unsigned long long ull;
typedef ull u64;

bool flghex;

#define prtval(name) cout<< "constexpr " << #name << "[" << dec << name.size() << "]=" << name <<";\n";
#define inc(i,n) for (int i=0;i<n;i++)

void showMask(ull x){
	puts("");
	inc(i,8){
		inc(j,8)
			putchar((x>>(i*8+j)&1)?'*':'.');
		puts("");
	}
}

template<typename T>
ostream &operator<<(ostream &o, const vector<T> &vec){
	o<<"{";
	for (size_t i=0;i<vec.size();i++){
		if (flghex){
			//showMask(vec[i]);
			o<<hex<<"0x"<<vec[i]<<",}"[i==vec.size()-1];
		}
		else
			o<<dec<<vec[i]<<",}"[i==vec.size()-1];
	}
	return o;
}

//common mask "-" "|" "\" "/"
vector<ull> h,v,d1,d2;

void genCommonMask(){
	inc(i,8){
		ull hh=0,vv=0;
		inc(j,8) 
			hh|=1ull<<(8*i+j), 
			vv|=1ull<<(8*j+i);
		h.push_back(hh);
		v.push_back(vv);
	}
	inc(i,15){
		ull dd1=0,dd2=0;
		inc(j,8) if (i-j>=0 && i-j<8){
			int y=i-j;
			dd1|=1ull<<((7-j)*8+y);
			dd2|=1ull<<(j*8+y);
		}
		d1.push_back(dd1);
		d2.push_back(dd2);
	}
	reverse(begin(d1),end(d1));
	flghex=1;
	prtval(h); prtval(v);
	prtval(d1); prtval(d2);
	flghex=0;
}

//position mask (4x64)
vector<ull> pm_h,pm_v,pm_d1,pm_d2,p_umask;
//position to line pos after PEXT (4x64)
vector<ull> pl_h,pl_v,pl_d1,pl_d2;

template<class T> constexpr const T& cmin(const T &a,const T &b){return a<b?a:b;}
constexpr u64 fpl_h(int x){return x%8;}
constexpr u64 fpl_v(int x){return x/8;}
constexpr u64 fpl_d1(int x){return cmin(x/8,x%8);}
constexpr u64 fpl_d2(int x){return cmin(x/8,7-x%8);}
constexpr u64 fpm_h(int x){return 0xffull<<((x/8)*8);}
constexpr u64 fpm_v(int x){return 0x101010101010101ull<<(x%8);}
constexpr u64 fpm_d1(int x){return (x/8-x%8>0)?0x8040201008040201<<((x/8-x%8)*8):0x8040201008040201>>((x%8-x/8)*8);}
constexpr u64 fpm_d2(int x){return (x/8+x%8-7>0)?0x0102040810204080<<((x/8+x%8-7)*8):0x0102040810204080>>((7-x%8-x/8)*8);}
constexpr u64 fp_umask(int x){return ~(fpm_h(x)|fpm_v(x)|fpm_d1(x)|fpm_d2(x));}
bool equ(int n, function<u64(int)> f, vector<u64> &v){
	for (int i=0;i<n;i++) if (f(i)!=v[i]) return 0;
	return 1; 
}

void genFlipPos(){
	auto getp=[](ull x, int p){
		return __builtin_popcountll(x&((1ull<<p)-1));
	};
	inc(p,64){
		for (auto hh:h) if (hh>>p&1){
			pm_h.push_back(hh);
			pl_h.push_back(getp(hh,p));
		}
		for (auto vv:v) if (vv>>p&1){
			pm_v.push_back(vv);
			pl_v.push_back(getp(vv,p));
		}
		for (auto dd1:d1) if (dd1>>p&1){
			pm_d1.push_back(dd1);
			pl_d1.push_back(getp(dd1,p));
		}
		for (auto dd2:d2) if (dd2>>p&1){
			pm_d2.push_back(dd2);
			pl_d2.push_back(getp(dd2,p));
		}
		p_umask.push_back(~(pm_h.back()|pm_v.back()|pm_d1.back()|pm_d2.back()));
	}
	prtval(pl_h); prtval(pl_v);
	prtval(pl_d1); prtval(pl_d2);
	
	flghex=1;
	prtval(pm_h); prtval(pm_v);
	prtval(pm_d1); prtval(pm_d2);	
	prtval(p_umask);
	
	#define CHK(ptn) cout<<#ptn<<" "<<equ(64, f##ptn, ptn)<<"\n";
	
	CHK(pl_h); CHK(pl_v);
	CHK(pl_d1); CHK(pl_d2);
	CHK(pm_h); CHK(pm_v);
	CHK(pm_d1); CHK(pm_d2);
	CHK(p_umask);
}

int main(){
	genCommonMask();
	genFlipPos();
	return 0;
}

