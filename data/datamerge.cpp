#include <iostream>
#include <fstream>
#include <cstdio>
using namespace std;

#define EVAL_FILE "reversicoeff.bin"

void readShort(FILE *stream, short &tar)
{
	fread(&tar, sizeof(tar), 1, stream);
}

void writeShort(FILE *stream, short tar)
{
	fwrite(&tar, sizeof(tar), 1, stream);
}

void readwriteShort(FILE *in, FILE *out)
{
	short t;
	readShort(in, t);
	writeShort(out, t);
	
}

void resave(FILE *in, FILE *out)
{
	short t;
	//wb, wodd, wcnt
	readShort(in, t);
	writeShort(out, t);
	cout<<t<<'\n';
	
	readwriteShort(in, out);
	readwriteShort(in, out);
	//e1_2x
	for (int i=0;i<59049;i++)
		readwriteShort(in, out);
	//c52
	for (int i=0;i<59049;i++)
		readwriteShort(in, out);
	//c33
	for (int i=0;i<19683;i++)
		readwriteShort(in, out);
	//e2
	for (int i=0;i<6561;i++)
		readwriteShort(in, out);
	//e3
	for (int i=0;i<6561;i++)
		readwriteShort(in, out);
	//e4
	for (int i=0;i<6561;i++)
		readwriteShort(in, out);
	//k8
	for (int i=0;i<6561;i++)
		readwriteShort(in, out);
	//k7
	for (int i=0;i<2187;i++)
		readwriteShort(in, out);
	//k6
	for (int i=0;i<729;i++)
		readwriteShort(in, out);
	//k5
	for (int i=0;i<243;i++)
		readwriteShort(in, out);
	//k4
	for (int i=0;i<80;i++)
		readwriteShort(in, out);
	
	//final test
	readShort(in, t);
	writeShort(out, t);
	cout<<t<<'\n';
}

int main()
{
	FILE* file0=fopen("trained2_4.bin","rb");
	FILE* file1=fopen("trained5_7.bin","rb");
	FILE* file2=fopen("trained8_10.bin","rb");
	FILE* file3=fopen("trained11_13.bin","rb");
	FILE* file4=fopen("trained14_17.bin","rb");
	FILE* file5=fopen("trained18_21.bin","rb");
	FILE* file6=fopen("trained22_26.bin","rb");
	FILE* file7=fopen("trained27_31.bin","rb");
	FILE* file8=fopen("trained32_37.bin","rb");
	FILE* file9=fopen("trained38_43.bin","rb");
	FILE* file10=fopen("trained44_50.bin","rb");
	FILE* fout=fopen(EVAL_FILE, "wb");
	
	//10 part used
	writeShort(fout, (short)11);
	resave(file0, fout);
	resave(file1, fout);
	resave(file2, fout);
	resave(file3, fout);
	resave(file4, fout);
	resave(file5, fout);
	resave(file6, fout);
	resave(file7, fout);
	resave(file8, fout);
	resave(file9, fout);
	resave(file10, fout);
	
	fclose(file0);
	fclose(file1);
	fclose(file2);
	fclose(file3);
	fclose(file4);
	fclose(file5);
	fclose(file6);
	fclose(file7);
	fclose(file8);
	fclose(file9);
	fclose(file10);
	fclose(fout);
	return 0;
}

