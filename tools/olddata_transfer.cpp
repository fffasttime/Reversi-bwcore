/*
transfer bwcore1.4 coeff file into bwcore1.5.0 coeff file
deprecated because bwcore1.5.0 has made new coeff data, bwcore1.5.1 has new evalptn function
*/
#include <bits/stdc++.h>
using namespace std;

struct CoeffPack{
	short e1[59049], c52[59049], c33[19683],
	 e2[6561], e3[6561], e4[6561], k8[6561], k7[2187], k6[729], k5[243], k4[81],
	  wb, wodd, wmob;
}pdata[12];

constexpr unsigned short pow3[]={1,3,9,27,81,243,729,2187,6561,19683,59049};
int new_e1_pos[59049];

char *EVAL_FILE, EVAL_FILE_S[50];

const short format_version=1;
const char *data_desc="from old ptndata_botzone\n\
wb|wodd|wmob|e1|c52|c33|e2|e3|e4|k8|k7|k6|k5|k4\n";
const short plen[]=
{0,0,0,10,10,9,8,8,8,8,7,6,5,4};
short COEFF_PARTCNT;

unsigned short checksum;
void readShort(FILE *f, short &x){
	fread(&x, 2, 1, f);
	checksum^=x;
}
void writeShort(FILE *f, short x){
	fwrite(&x, 2, 1, f);
	checksum^=x;
}

void loadPtnData(){
	FILE *eval_stream=fopen(EVAL_FILE, "rb");
	readShort(eval_stream, COEFF_PARTCNT);
	for (int i=0;i<COEFF_PARTCNT;i++){
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
void savePtnData(){
	printf("save to %s\n", EVAL_FILE_S);
	FILE *eval_stream=fopen(EVAL_FILE_S, "wb");
	writeShort(eval_stream, format_version);
	writeShort(eval_stream, COEFF_PARTCNT);
	writeShort(eval_stream, sizeof(plen)/sizeof(short));
	for (short x:plen) writeShort(eval_stream, x);
	checksum=0;
	for (int i=0;i<COEFF_PARTCNT;i++){
		writeShort(eval_stream,pdata[i].wb);
		writeShort(eval_stream,pdata[i].wodd);
		writeShort(eval_stream,pdata[i].wmob);
		for (int j=0;j<59049;j++)
			writeShort(eval_stream,pdata[i].e1[new_e1_pos[j]]);
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
	}
	writeShort(eval_stream, checksum);
	writeShort(eval_stream, strlen(data_desc)+1);
	fwrite(data_desc, strlen(data_desc)+1, 1, eval_stream);
	
	long long t=time(NULL);
	fwrite(&t, sizeof t, 1, eval_stream);
	printf("timestamp: %lld\n",t);
	
	fclose(eval_stream);
}
void init(){
	int p[10];
	#define FI(ii) for(p[ii]=0;p[ii]<3;p[ii]++)
	FI(0) FI(1) FI(2) FI(3) FI(4) FI(5) FI(6) FI(7) FI(8) FI(9){
		int s=0;
		for(int j=0;j<10;j++) s*=3, s+=p[j];
		int s1=p[0];
		s1=s1*3+p[2];
		s1=s1*3+p[3];
		s1=s1*3+p[4];
		s1=s1*3+p[5];
		s1=s1*3+p[6];
		s1=s1*3+p[7];
		s1=s1*3+p[9];
		s1=s1*3+p[1];
		s1=s1*3+p[8];
		new_e1_pos[s1]=s;
	}
}

int main(int argc, char **argv){
	init();
	if (argc==1) return puts("args: input_filename [output_filename]"),0;
	EVAL_FILE=argv[1];
	loadPtnData();
	if (argc==2){
		strcpy(EVAL_FILE_S, "new_");
		strcat(EVAL_FILE_S, EVAL_FILE);
	}
	else strcpy(EVAL_FILE_S,argv[2]);
	savePtnData();
	return 0;
}

