/*
ptndatainfo.cpp
This is a helper program to show bwcore1.5 coeff data metainfo
*/
#include <bits/stdc++.h>
using namespace std;
#define inc(i,n) for (int i=0;i<n;i++)

constexpr unsigned short pow3[]={1,3,9,27,81,243,729,2187,6561,19683,59049};

/*
struct CoeffPack{
	short e1[59049], c52[59049], c33[19683],
	 e2[6561], e3[6561], e4[6561], k8[6561], k7[2187], k6[729], k5[243], k4[81],
	  wb, wodd, wmob;
}pdata[12];*/

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
    short plen[20];
    inc(i,type_cnt)  readShort(in, plen[i]), printf("%d ", plen[i]); puts("");
    int datapack_len = 0;
    inc(i,type_cnt) datapack_len += (plen[i]>10?plen[i]:pow3[plen[i]]);
    printf("datapack_len: %d\n", datapack_len);
    unsigned short checksum=0;
    auto rdc=[&](short &x){readShort(in, x); checksum^=x;};
    inc(i,part_cnt){
        inc(k,type_cnt){
            short x; int len;
            if (plen[k]>10) len=plen[k];
            else len=pow3[plen[k]];
            inc(j, len) rdc(x); // load & check
        }
    }
    short file_checksum; readShort(in, file_checksum);
    printf("checksum=%d,",(int)checksum);
    if (checksum!=file_checksum) printf("fail!\n");
    else printf("ok\n");
    
    char desc[200];
    short desc_len; readShort(in, desc_len);
    if (desc_len>200){
        puts("wrong desc_len");
        exit(1);
    }
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

