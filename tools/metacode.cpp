/*
metacode.cpp
generate constant arrays in code
author: fffasttime
*/
#include <iostream>
#include <vector>
using namespace std;

typedef unsigned long long ull;

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
	flghex=1;
	prtval(h); prtval(v);
	prtval(d1); prtval(d2);
	flghex=0;
}

//position mask (4x64)
vector<ull> pm_h,pm_v,pm_d1,pm_d2,p_umask;
//position to line pos after PEXT (4x64)
vector<int> pl_h,pl_v,pl_d1,pl_d2;
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
}

int main(){
	genCommonMask();
	genFlipPos();
	return 0;
}

