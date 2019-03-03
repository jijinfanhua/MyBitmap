#include <stdio.h>
#include <fstream>
#include <math.h>
#include <sys/time.h>
//#include <windows.h>
#include "bitType.h"
#include "rib.h"
#include "global.h"

/*
void traverse() {
	uint i;
	uint hop1, hop2;
	for (i = 0x00000000; i <= 0xfffffffe; i++) {
		hop2 = lookupNode(i);
		hop1 = lookup(i);
		if (hop1 != hop2) {
			printf("ip:%x\t\t\tlulea:%u\t\t\ttrie:%u\n", i, hop1, hop2);
			getchar();
		}
		if ((i << 12) == 0) {
			printf("ratio:%f\t\t\thop:%10u\r", (((i >> 20) + 0.0) / (0xfff)), hop1);
		}
	}
}
*/


/*
void traverse_lookup(char * file) {
	uint i;
	int j;
	uint hop;
	unsigned long long time_start, time_end;
	unsigned long long sum = 0;
	double average_cycle;
	char fileName[100];
	printf("%s\n", file);
	for (j = strlen(file); file[j] != '\\' && j >= 0; j--);
	printf("%d\n", j);
	strcpy(fileName, file + j + 1);
	printf("%s\n", fileName);
	strcat(fileName, ".lookup.lulea");
	FILE *fp = fopen(fileName, "w");
	for (i = 0x00000000; i <= 0xfffffffe; i++)
	{
		time_start = GetCycleCount();
		hop = lookup(i);
		time_end = GetCycleCount();
		sum += time_end - time_start;
		//fprintf(fp, "%llu\n", time_end - time_start);
		if ((i << 12) == 0)
		{
			printf("ratio:%f\t\t\thop:%10u\r", (((i >> 20) + 0.0) / (0xfff)), hop);
		}
	}
	average_cycle = sum* 1.0 / 0xfffffffe;
	printf("all cycles: %llu \t average lookup cycles: %lf\n", sum, average_cycle);
	fprintf(fp, "all cycles: %llu \t average lookup cycles: %lf\n", sum, average_cycle);
	fclose(fp);
}*/

void trace_lookup(char *file)
{
	ushort u1, u2, u3, u4;
	uint ip = 0;
	ushort f8 = 0x00ff;
	FILE *fp = fopen(file, "r");
	int count = 0;
	double average_cycle;
	uint hop = 0;
	uint *list = (uint *)malloc(sizeof(uint)* (1e8));
	uint * h = list;
	while (!feof(fp) && count < 1e8)
	{
		fscanf(fp, "%u", &list[count++]);
		if (count % 1000000 == 0) printf("%d M\r", (count / 1000000));
	}

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	for (h = list; h < list + count; h++)
	{
		//fscanf(fp, "%*d.%*d,%hu.%hu.%hu.%hu,", &u1, &u2, &u3, &u4);
		//fscanf(fp, "%hu.%hu.%hu.%hu,%*[^\n]", &u1, &u2, &u3, &u4);
		//ip = ((u1 & f8) << 24) + ((u2 & f8) << 16) + ((u3 & f8) << 8) + ((u4 & f8));
		//printf("ip address: %hu.%hu.%hu.%hu\n", u1, u2, u3, u4);
		//fscanf(fp, "%hu,", &u1);
		//fscanf(fp, "%hu,", &u1);
		//fscanf(fp, "%hu,", &u1);
		ip = *h;
		hop += lookup(ip);
	}
	gettimeofday(&end_time, NULL);



	double sum = (end_time.tv_sec-start_time.tv_sec)*1000000.0 + (end_time.tv_usec-start_time.tv_usec)*1.0;
	average_cycle = count * 1.0 / (sum / 1000000) / 1000000;


	FILE * myfp = fopen("lulea.random.lookup.txt","at");
	fprintf(myfp,"eqix\t%lf\n",average_cycle);
	fclose(myfp);

	printf("all cycles: %lf \t average lookup cycles: %lf\n \t hops:%u\n", sum, average_cycle, hop);
}

int main(int argc, char * argv[])
{
	char * file = "eqix_2017.txt";
	char * updateFile = "eqix_update_2017.txt";
	char * traceFile = "five_tuple_trace_equinix_modify.txt";
	bool traceFlag = 1;
	printf("argc == %d\n", argc);
	for (int i = 0; i < argc; i++) printf("%s\n", argv[i]);
	if (argc == 3)
	{
		file = argv[1];
		updateFile = argv[2];
	}
	if (argc == 4)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFile = argv[3];
		traceFlag = 1;
	}
	/*else
	{
	printf("input: name, lookup_file, update_file\n");
	system("pause");
	return 0;
	}
	*/
	start(file);
	printf("construction over...\n");

	/*lookup*/
	printf("trace flag = %d\n", traceFlag);
	traceFlag = 1;
	if (traceFlag)
	{
		trace_lookup(traceFile);
	}
	else
	{
		//traverse_lookup(file);
	}
	printf("lookup over...\n");

	/*update */
	//update(updateFile);
	printf("lulea update over...\n");

	/*for varifying*/
	//makeWholeTrie(file);
	//modifyWholeTrie(updateFile);
	//printf("tire update over...\n");
	//traverse();
	freeMem();
	//freeRoot();
	printf("all over...\n");
	//system("pause");
}
