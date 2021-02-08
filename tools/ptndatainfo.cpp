#include <bits/stdc++.h>
using namespace std;
#define inc(i,n) for (int i=0;i<n;i++)

constexpr unsigned short pow3[]={1,3,9,27,81,243,729,2187,6561,19683,59049};

struct CoeffPack{
	short e1[59049], c52[59049], c33[19683],
	 e2[6561], e3[6561], e4[6561], k8[6561], k7[2187], k6[729], k5[243], k4[81],
	  wb, wodd, wmob;
}pdata[12];

char *EVAL_FILE;

void readShort(FILE *f, short &x){
	fread(&x, 2, 1, f);
}

void loadPtnData(){
    FILE *in=fopen(EVAL_FILE, "rb");
    short format_version; readShort(in, format_version);
    printf("version: %d\n", format_version); fflush(stdout);
    assert(format_version==1);
    short part_cnt; readShort(in, part_cnt);
    printf("partcnt: %d\n", part_cnt);
    short type_cnt; readShort(in, type_cnt);
    printf("typecnt: %d\n", type_cnt);
    short _; inc(i,type_cnt)  readShort(in, _), printf("%d ", _); puts("");
    unsigned short checksum=0;
    auto rdc=[&](short &x){readShort(in, x); checksum^=x;};
    inc(i,part_cnt){
        auto &p=pdata[i];
        rdc(p.wb);
        rdc(p.wodd);
        rdc(p.wmob);
        inc(j,pow3[10]) rdc(p.e1[j]);
        inc(j,pow3[10]) rdc(p.c52[j]);
        inc(j,pow3[9])  rdc(p.c33[j]);
        inc(j,pow3[8])  rdc(p.e2[j]);
        inc(j,pow3[8])  rdc(p.e3[j]);
        inc(j,pow3[8])  rdc(p.e4[j]);
        inc(j,pow3[8])  rdc(p.k8[j]);
        inc(j,pow3[7])  rdc(p.k7[j]);
        inc(j,pow3[6])  rdc(p.k6[j]);
        inc(j,pow3[5])  rdc(p.k5[j]);
        inc(j,pow3[4])  rdc(p.k4[j]);
		cout<<pdata[i].wb<<'\n';
		cout<<pdata[i].k4[80]<<'\n';
    }
    short file_checksum; readShort(in, file_checksum);
    printf("checksum=%d,",(int)checksum);
    if (checksum!=file_checksum) printf("fail!\n");
    else printf("ok\n");
    
    char desc[200];
    short desc_len; readShort(in, desc_len);
    fread(desc, desc_len, 1, in);
    puts("description:");
	puts(desc);
    
    long long tm;
    fread(&tm, sizeof tm, 1, in);
    printf("gen timestamp: %lld , %s",tm, ctime(&tm));
    
    fclose(in);
}

int main(int argc, char **argv){
	if (argc==1) return puts("args: input_filename"),0;
	EVAL_FILE=argv[1];
	loadPtnData();
}

