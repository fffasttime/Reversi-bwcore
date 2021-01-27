#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
using namespace std;

#define EVAL_FILE_B "reversicoeff_temp.bin"
#define EVAL_FILE_F "reversicoeff_temp.txt"

void readShort(FILE *stream, short &tar)
{
	fread(&tar, 2, 1, stream);
}

void writeShort(FILE *stream, short tar)
{
	fwrite(&tar, 2, 1, stream);

}
struct CoeffPackB
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

struct CoeffPackF
{
	float e1[59049], c52[59049], c33[19683],
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

#define COEFF_PARTCNT 11
struct GameCoeffB
{
	
	CoeffPackB dat[COEFF_PARTCNT];
	
	void initPtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE_B, "rb");
		short part_cnt; readShort(eval_stream, part_cnt);
		assert(part_cnt == COEFF_PARTCNT);
		for (int i=0;i<part_cnt;i++)
		{
			auto &pdata=dat[i];
			readShort(eval_stream,pdata.wb);
			readShort(eval_stream,pdata.wodd);
			readShort(eval_stream,pdata.wmob);
			for (int j=0;j<59049;j++)
				readShort(eval_stream,pdata.e1[j]);
			for (int j=0;j<59049;j++)
				readShort(eval_stream,pdata.c52[j]);
			for (int j=0;j<19683;j++)
				readShort(eval_stream,pdata.c33[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata.e2[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata.e3[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata.e4[j]);
			for (int j=0;j<6561;j++)
				readShort(eval_stream,pdata.k8[j]);
			for (int j=0;j<2187;j++)
				readShort(eval_stream,pdata.k7[j]);
			for (int j=0;j<729;j++)
				readShort(eval_stream,pdata.k6[j]);
			for (int j=0;j<243;j++)
				readShort(eval_stream,pdata.k5[j]);
			for (int j=0;j<81;j++)
				readShort(eval_stream,pdata.k4[j]);
			cout<<pdata.wb<<'\n';
			cout<<pdata.k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	void savePtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE_B, "wb");
		short part_cnt=COEFF_PARTCNT; writeShort(eval_stream, part_cnt);
		for (int i=0;i<part_cnt;i++)
		{
			auto &pdata=dat[i];
			writeShort(eval_stream,pdata.wb);
			writeShort(eval_stream,pdata.wodd);
			writeShort(eval_stream,pdata.wmob);
			for (int j=0;j<59049;j++)
				writeShort(eval_stream,pdata.e1[j]);
			for (int j=0;j<59049;j++)
				writeShort(eval_stream,pdata.c52[j]);
			for (int j=0;j<19683;j++)
				writeShort(eval_stream,pdata.c33[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata.e2[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata.e3[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata.e4[j]);
			for (int j=0;j<6561;j++)
				writeShort(eval_stream,pdata.k8[j]);
			for (int j=0;j<2187;j++)
				writeShort(eval_stream,pdata.k7[j]);
			for (int j=0;j<729;j++)
				writeShort(eval_stream,pdata.k6[j]);
			for (int j=0;j<243;j++)
				writeShort(eval_stream,pdata.k5[j]);
			for (int j=0;j<81;j++)
				writeShort(eval_stream,pdata.k4[j]);
			cout<<pdata.wb<<'\n';
			cout<<pdata.k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	void clear()
	{
		for (int i=0;i<COEFF_PARTCNT;i++)
			dat[i].clear();
	}
	
}coeff_datab;

struct GameCoeffF
{
	CoeffPackF dat[COEFF_PARTCNT];
	
	void initPtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE_F, "r");
		short part_cnt; fscanf(eval_stream,"%hd",&part_cnt);
		assert(part_cnt == COEFF_PARTCNT);
		for (int i=0;i<part_cnt;i++)
		{
			auto &pdata=dat[i];
			fscanf(eval_stream,"%f",&pdata.wb);
			fscanf(eval_stream,"%f",&pdata.wodd);
			fscanf(eval_stream,"%f",&pdata.wmob);
			for (int j=0;j<59049;j++)
				fscanf(eval_stream,"%f",&pdata.e1[j]);
			for (int j=0;j<59049;j++)
				fscanf(eval_stream,"%f",&pdata.c52[j]);
			for (int j=0;j<19683;j++)
				fscanf(eval_stream,"%f",&pdata.c33[j]);
			for (int j=0;j<6561;j++)
				fscanf(eval_stream,"%f",&pdata.e2[j]);
			for (int j=0;j<6561;j++)
				fscanf(eval_stream,"%f",&pdata.e3[j]);
			for (int j=0;j<6561;j++)
				fscanf(eval_stream,"%f",&pdata.e4[j]);
			for (int j=0;j<6561;j++)
				fscanf(eval_stream,"%f",&pdata.k8[j]);
			for (int j=0;j<2187;j++)
				fscanf(eval_stream,"%f",&pdata.k7[j]);
			for (int j=0;j<729;j++)
				fscanf(eval_stream,"%f",&pdata.k6[j]);
			for (int j=0;j<243;j++)
				fscanf(eval_stream,"%f",&pdata.k5[j]);
			for (int j=0;j<81;j++)
				fscanf(eval_stream,"%f",&pdata.k4[j]);
			cout<<pdata.wb<<'\n';
			cout<<pdata.k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	void savePtnData()
	{
		FILE *eval_stream=fopen(EVAL_FILE_F, "w");
		short part_cnt=COEFF_PARTCNT; fprintf(eval_stream,"%d ",part_cnt);
		for (int i=0;i<part_cnt;i++)
		{
			auto &pdata=dat[i];
			fprintf(eval_stream,"%f ",pdata.wb);
			fprintf(eval_stream,"%f ",pdata.wodd);
			fprintf(eval_stream,"%f ",pdata.wmob);
			for (int j=0;j<59049;j++)
				fprintf(eval_stream,"%f ",pdata.e1[j]);
			for (int j=0;j<59049;j++)
				fprintf(eval_stream,"%f ",pdata.c52[j]);
			for (int j=0;j<19683;j++)
				fprintf(eval_stream,"%f ",pdata.c33[j]);
			for (int j=0;j<6561;j++)
				fprintf(eval_stream,"%f ",pdata.e2[j]);
			for (int j=0;j<6561;j++)
				fprintf(eval_stream,"%f ",pdata.e3[j]);
			for (int j=0;j<6561;j++)
				fprintf(eval_stream,"%f ",pdata.e4[j]);
			for (int j=0;j<6561;j++)
				fprintf(eval_stream,"%f ",pdata.k8[j]);
			for (int j=0;j<2187;j++)
				fprintf(eval_stream,"%f ",pdata.k7[j]);
			for (int j=0;j<729;j++)
				fprintf(eval_stream,"%f ",pdata.k6[j]);
			for (int j=0;j<243;j++)
				fprintf(eval_stream,"%f ",pdata.k5[j]);
			for (int j=0;j<81;j++)
				fprintf(eval_stream,"%f ",pdata.k4[j]);
			cout<<pdata.wb<<'\n';
			cout<<pdata.k4[80]<<'\n';
		}
		fclose(eval_stream);
	}
	
	void clear()
	{
		for (int i=0;i<COEFF_PARTCNT;i++)
			dat[i].clear();
	}
	
}coeff_dataf;

float bf(short b)
{
	return (b-0.5f)/256.0f;
}

void btof()
{
	coeff_datab.initPtnData();
	for (int i=0;i<COEFF_PARTCNT;i++)
	{
		auto &pdatab=coeff_datab.dat[i];
		auto &pdataf=coeff_dataf.dat[i];
		pdataf.wb=bf(pdatab.wb);
		pdataf.wodd=bf(pdatab.wodd);
		pdataf.wmob=bf(pdatab.wmob);
		for (int j=0;j<59049;j++)
			pdataf.e1[j]=bf(pdatab.e1[j]);
		for (int j=0;j<59049;j++)
			pdataf.c52[j]=bf(pdatab.c52[j]);
		for (int j=0;j<19683;j++)
			pdataf.c33[j]=bf(pdatab.c33[j]);
		for (int j=0;j<6561;j++)
			pdataf.e2[j]=bf(pdatab.e2[j]);
		for (int j=0;j<6561;j++)
			pdataf.e3[j]=bf(pdatab.e3[j]);
		for (int j=0;j<6561;j++)
			pdataf.e4[j]=bf(pdatab.e4[j]);
		for (int j=0;j<6561;j++)
			pdataf.k8[j]=bf(pdatab.k8[j]);
		for (int j=0;j<2187;j++)
			pdataf.k7[j]=bf(pdatab.k7[j]);
		for (int j=0;j<729;j++)
			pdataf.k6[j]=bf(pdatab.k6[j]);
		for (int j=0;j<243;j++)
			pdataf.k5[j]=bf(pdatab.k5[j]);
		for (int j=0;j<81;j++)
			pdataf.k4[j]=bf(pdatab.k4[j]);
	}
	coeff_dataf.savePtnData();
}

void mystat()
{
	coeff_dataf.initPtnData();
	cout<<'\n';
	for (int i=0;i<11;i++)
	{
		auto &pdataf=coeff_dataf.dat[i];
		cout<<pdataf.wmob<<' ';
		float cc=0;
		for (int j=0;j<59049;j++)
			cc+=fabs(pdataf.e1[j]);
		cout<<cc<<' ';
		cout<<'\n';
	}
}

int main()
{
#if 0
	btof();
#endif
	mystat();
	system("pause");
}
