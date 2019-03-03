#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "lulea.h"
#include "rib.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/unistd.h>
#endif

FILE *resultFile;

int KK;
int KKlist[] = {  1, 5, 8, 10, 15, 20, 30, 60, 100, 200, 350, 500, 700, 1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000, 3000, 4000, 5000, 7000, 10000, 20000, 50000, 399999999 };
char * file = "eqix_20172.txt";//"D:\\Tsinghua_OBMA\\fengyong_experiment\\data\\eqix.2016.out";
char * updateFile = "eqix_update_2017.txt";//"D:\\Tsinghua_OBMA\\fengyong_experiment\\data\\eqix.update";
char * traceFile = "five_tuple_trace_equinix_modify.txt";//"random.txt";//five_tuple_trace_equinix_modify.txt
char * chunkFile = "eqix_chunk.txt";
int lookupnum = 0;
int updatenum = 0;

int traverse_local(RouterEntry rentry)
{
	uint i,j;
	uint hop1, hop2;
	int usame = 0;
	for (i = rentry.ipnet; i < rentry.ipnet + (1 << (32 - rentry.length)); i++)
	{
		hop2 = lookupNode(i);
		hop1 = lookup(i);
		if (hop1 != hop2)
		{
			printf("ip:%x\t\t\tlulea:%u\t\t\ttrie:%u\n", i, hop1, hop2);
			return 0;
			getchar();
		}
	}
	return 1;
}

void traverse() {
	uint i;
	uint hop1, hop2;
	int usame = 0;
	makeWholeTrie(file);
	for (i = 0x00000000; i <= 0xfffffffe; i++) {
		hop2 = lookupNode(i);
		hop1 = lookup(i);
		if (hop1 != hop2) 
		{
			printf("ip:%x\t\t\tlulea:%u\t\t\ttrie:%u\n", i, hop1, hop2);
			getchar();
			usame++;
		}
		if ((i << 12) == 0) 
		{
			printf("ratio:%f\t\t\thop:%10u\r", (((i >> 20) + 0.0) / (0xfff)), hop1);
			printf("wrong number:%d\n", usame);
		}		
	}
	printf("wrong number:%d\n", usame);
	freeRoot();
}

void verify_update(char *file, char* fileUpdate)
{
	RouterEntry rentry;
	FILE * fp;
	FILE * fpUpdate;
	char c;
	int num = 0;
	fp = fopen(file, "r");
	if (fp == NULL) 
	{
		perror("Open file");
		//printf("err no: %d\n", errno);
		exit(1);
	}
	fpUpdate = fopen(fileUpdate, "r");
	if (fp == NULL) 
	{
		perror("Open file");
		//printf("err no: %d\n", errno);
		exit(1);
	}
	makeWholeTrie(file);
	while (inputUpdateRib(&rentry, &c, fpUpdate))
	{

		if (num == 1978824)//1997889
		{
			if (c == 'A') {
				announceNode(rentry);
				rentry.port = findMap(rentry.port);
				announce(rentry);
			}
			else if (c == 'W') {
				withdrawNode(rentry);
				//rentry.port = findMap(rentry.port);
				withdraw(rentry);
			}
			if (traverse_local(rentry) == 0)
			{
				printf("num [%d] error\n", num);
				getchar();
			}
			num++;
		}
		else
		{
			if (c == 'A') {
				announceNode(rentry);
				rentry.port = findMap(rentry.port);
				announce(rentry);
			}
			else if (c == 'W') {
				withdrawNode(rentry);
				//rentry.port = findMap(rentry.port);
				withdraw(rentry);
			}
			/*if (traverse_local(rentry) == 0)
			{
				printf("num [%d] error\n", num);
				getchar();
			}
			uint ipp = 0x55967600;
			uint p1 = lookupNode(ipp);
			uint p2 = lookup(ipp);
			if (p1 != p2)
			{
				printf("num [%d] \t lulea: %u \t trie:%u\n", num, p2, p1);
				getchar();
			}*/
			num++;
		}
	}
	printf("num [%d] correct\n", num);
	int usame = 0;
	for (uint i = 0x00000000; i <= 0xfffffffe; i++) {
		uint hop2 = lookupNode(i);
		uint hop1 = lookup(i);
		if (hop1 != hop2)
		{
			printf("ip:%x\t\t\tlulea:%u\t\t\ttrie:%u\n", i, hop1, hop2);
			getchar();
			usame++;
		}
		if ((i << 12) == 0)
		{
			printf("ratio:%f\t\t\thop:%10u\r", (((i >> 20) + 0.0) / (0xfff)), hop1);
			printf("wrong number:%d\n", usame);
		}
	}
	printf("wrong number:%d\n", usame);
	fclose(fp);
	fclose(fpUpdate);
	freeRoot();
}

void traverse_lookup(char * file) {
	uint i;
	int j;
	uint hop;
	unsigned long long time_start, time_end;
	unsigned long long sum = 0;
	double average_cycle;
	char fileName[100];
	printf("%s\n", file);
	for (j = strlen(file); file[j] != '\\' && j>= 0; j--);
	printf("%d\n", j);
	strcpy(fileName, file + j + 1);
	printf("%s\n", fileName);
	strcat(fileName, ".lookup.loop_opt");
	FILE *fp = fopen(fileName, "w");
	for (i = 0x00000000; i <= 0xfffffffe; i++) 
	{
		//time_start = GetCycleCount();
		hop = lookup(i);
		//time_end = GetCycleCount();
		sum += time_end - time_start;
		if ((i << 12) == 0) 
		{
			printf("ratio:%f\t\t\thop:%10u\r", (((i >> 20) + 0.0) / (0xfff)), hop);
		}
	}
	average_cycle = sum* 1.0 / 0xfffffffe;
	fprintf(fp, "all cycles: %llu \t average lookup cycles: %lf\n", sum, average_cycle);
	printf("all cycles: %llu \t average lookup cycles: %lf\n", sum, average_cycle);
	fclose(fp);
}
void trace_lookup(char *file)
{
	ushort u1, u2, u3, u4;
	uint ip = 0;
	ushort f8 = 0x00ff;
	FILE *fp = fopen(file, "r");

	FILE *temp = fopen("mybitmap_eqix_lookup_update_result.txt","at");
	int result_pointer = 0;

	int max = 10000000;
	int cur = 0;
	uint ipad;

	int count = 0;
	uint hop = 0;
	uint *list = (uint *)malloc(sizeof(uint)* (1e8));
	//RouterEntry * list = (RouterEntry *)malloc(sizeof(RouterEntry)*max);
	//uint * h = list;

	struct timeval start_time;
	struct timeval end_time;
	double time_use = 0;
	double lookup_speed = 0;

	while (!feof(fp) && count < TRACE_READ)
	{
		fscanf(fp, "%u", &list[count++]);
		if (count % 1000000 == 0) printf("%d M\r", (count / 1000000));
	}
	uint all = 0;
	gettimeofday(&start_time,NULL);
	for (int j = 0; j < 10; j++)
	{
		for (int i = 0; i < count; i++)
		{
			ip = list[i];
			hop = lookup(ip);
		}
	}
	gettimeofday(&end_time,NULL);
	printf("3333\n");
	time_use = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;//ms =10e-6 s

	printf("time_use: %lf\t count: %d \t all:%u\n",time_use,count,all);

	//lookup_speed = 1.0 / (time_use * 1.0 / 1000000 / ( count * 10 )) / 1000000;
	lookup_speed = 1.0 / (time_use * 1.0 / 1000000 / ( 10 * count )) / 1000000;

	FILE * myfp = fopen("obma.real.lookup.txt","at");
	//fprintf(myfp, "org\t%lf\n",lookup_speed);
	fclose(myfp);

	printf("all_time: %lf \t lookup_speed: %lf\n \t hops:%u\n", time_use, lookup_speed, hop);
	fprintf(temp,"%lf\t",lookup_speed);
	//fprintf(resultFile, "[Lookup]\nall_time: %lf \t lookup_speed: %lf\n", time_use, lookup_speed);
	fclose(temp);
	free(list);
}

int inputTraceMean(RouterEntry *p, int max)
{
	FILE * fpLookup, *fpUpdate;
	fpLookup = fopen(traceFile, "r");
	if (fpLookup == NULL)
	{
		printf("open lookupfile error\n");
		exit(0);
	}
	fpUpdate = fopen(updateFile, "r");
	if (fpUpdate == NULL)
	{
		printf("open updatefile error\n");
		exit(0);
	}

	printf("here..1 and max is %u\n",max);

	int cur = 0;
	char c;//��ʶW����A
	uint ipad;//ip address
	while (cur < max) {
		if (cur % KK == 0) {
			if (!inputUpdate2(p, &c, fpUpdate)) break;
			if (c == 'A') {
				p->type = 1;
				updatenum++;
			}else if (c == 'W') {
				p->type = 2;
				updatenum++;
			}
		}else {
			if (fscanf(fpLookup, "%u\n", &ipad) == -1) break;
			p->type = 0;
			p->ipnet = ipad;
			lookupnum++;
		}
		p++;
		cur++;
	}
	fclose(fpLookup);
	fclose(fpUpdate);
	printf("hahahhahaha cur=%u, max = %u\n", cur, max);
	return cur;
}

int a2(int argc, char * argv[])
{
	//int max = (1 << 27);
	int max = 3000000;
	RouterEntry * list = (RouterEntry *)malloc(sizeof(RouterEntry)*max);
	FILE *fp;
	fp = fopen("mybitmap_isc_update_lookup_mix_result.txt", "at");

	int i = 31;
	if (argc == 2) {
		i = atoi(argv[1]);
	}

	//memset(list, 0, sizeof(RouterEntry)*(1 << 27));
	//printf("the size of memset is: %llu\n", sizeof(RouterEntry)*(1 << 27));
	memset(list, 0, sizeof(RouterEntry)*max);
	printf("the size of memset is: %llu\n", sizeof(RouterEntry)*max);
	KK = KKlist[i];
	lookupnum = 0;
	updatenum = 0;
	max = inputTraceMean(list, max);

	/*open result file*/
	char fileName[40];
	int j = 0;
	for (j = strlen(file); file[j] != '\\' && j >= 0; j--);
	strcpy(fileName, file + j + 1);
	printf("%s\n", fileName);
	strcat(fileName, ".mybitmap.result.txt");
	resultFile = fopen(fileName, "w");
	if (resultFile == NULL)
		return 0;

	start(file);
	printf("construct over...\n");

	overhead();
	modifyChunk(chunkFile);
	printf("chunk MODIFIED over...\n");
	overhead();

	struct timeval start_time, end_time;
	double total;
	double lookup_speed, update_speed;
	gettimeofday(&start_time, NULL);
	int all = updateLookup(list, max);
	gettimeofday(&end_time, NULL);
	total = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000.0;
	lookup_speed = lookupnum * 1.0 / (total / 1000000.0) / 1000000;
	update_speed = updatenum * 1.0 / (total / 1000000.0) / 1000000;
	fprintf(fp, "%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\n", lookupnum, updatenum, max, KK, total, lookup_speed, update_speed);
	printf("%d\t%d\t%d\t%d\t%lf\tlookupspeed:%lf\tupdatespeed:%lf\n", lookupnum, updatenum, max, KK, total, lookup_speed, update_speed);
	freeMemory();

	free(list);
	fclose(fp);
	printf("all over...\n");
	return 0;

}

int inputTrace(RouterEntry *p, int max){
	FILE * fpLookup;
	fpLookup = fopen(traceFile, "r");
	if (fpLookup == NULL)
	{
		printf("open lookupfile error\n");
		exit(0);
	}
	printf("here..1 and max is %u\n", max);

	int cur = 0;
	char c;//��ʶW����A
	uint ipad;//ip address
	while (cur < max) {
		if (fscanf(fpLookup, "%u\n", &ipad) == -1) break;
		p->type = 0;
		p->ipnet = ipad;
		lookupnum++;
		p++;
		cur++;
	}
	fclose(fpLookup);
	printf("hahahhahaha cur=%u, max = %u\n", cur, max);
	return cur;
}

int ai(int argc, char * argv[]){
	bool traceFlag = 1;
	printf("argc == %d\n", argc);
	for (int i = 0; i < argc; i++) printf("%s\n", argv[i]);
	if (argc == 3)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFlag = 0;
	}
	if (argc == 4)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFile = argv[3];
		traceFlag = 1;
	}
	if (argc == 5)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFile = argv[3];
		chunkFile = argv[4];
		traceFlag = 1;
	}

	/*open result file*/
	char fileName[40];
	int j = 0;
	for (j = strlen(file); file[j] != '\\' && j >= 0; j--);
	strcpy(fileName, file + j + 1);
	printf("%s\n", fileName);
	strcat(fileName, ".mybitmap.result.txt");
	resultFile = fopen(fileName, "w");
	if (resultFile == NULL)
		return 0;

	int max = 10000000;
	RouterEntry * list = (RouterEntry *)malloc(sizeof(RouterEntry)*max);
	max = inputTrace(list, max);
	/* construct structure*/
	start(file);
	printf("construction over...\n");

	/*modifychunk*/
	overhead();
	//modifyChunk(chunkFile);
	//printf("chunk MODIFIED over...\n");
	overhead();
	/*lookup*/
	printf("trace flag = %d\n", traceFlag);


	struct timeval start_time;
	struct timeval end_time;
	double time_use = 0;
	double lookup_speed = 0;

	uint all = 0;
	RouterEntry * p = list;
	gettimeofday(&start_time,NULL);
	for(p = list; p < list + max; p++){
		if(p->type == 0){
			all  = lookup(p->ipnet);
			//printf("all:%u\n",all);
		}
	}
	gettimeofday(&end_time,NULL);

	time_use = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;//ms =10e-6 s

	printf("time_use: %lf\t all:%u\n",time_use,all);

	//lookup_speed = 1.0 / (time_use * 1.0 / 1000000 / ( count * 10 )) / 1000000;
	lookup_speed = 1.0 / (time_use * 1.0 / 1000000 / ( max )) / 1000000;

	printf("all_time: %lf \t lookup_speed: %lf\n \t hops:%u\n", time_use, lookup_speed, all);
	//trace_lookup(traceFile);
	printf("lookup over...\n");

	//traverse();

	/*update verify*/
	//printf("verifying update\n");
	//verify_update(file, updateFile);

	/*update */
	//update(updateFile);
	//printf("update over...\n");

	freeMemory();

	printf("all over...\n");
	fclose(resultFile);
	//system("pause");
}

int main(int argc, char * argv [])
{
	//char * file = "D:\\Tsinghua_OBMA\\data\\rib\\isc_20172.txt";
	//char * updateFile = "D:\\Tsinghua_OBMA\\data\\update\\isc_update_2017.txt";
	//char * traceFile = "D:\\Tsinghua_OBMA\\data\\trace\\isc_good_trace.txt";
	/*char * file = "E:\\SVN\\infocom2017\\data\\adaptive_test\\isc_rib.txt";
	char * updateFile = "E:\\SVN\\infocom2017\\data\\adaptive_test\\isc_20170404_after_filter.txt";
	char * traceFile = "E:\\document\\2017infocom\\data\\trace\\five_tuple_trace_equinix_modify.txt";*/
	//char * chunkFile = "D:\\Tsinghua_OBMA\\data\\chunk\\isc.txt";//"E:\\SVN\\infocom2017\\data\\adaptive_test\\isc_chunk.txt";
	bool traceFlag = 1;
	printf("argc == %d\n", argc);
	for (int i = 0; i < argc; i++) printf("%s\n", argv[i]);
	if (argc == 3)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFlag = 0;
	}
	if (argc == 4)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFile = argv[3];
		traceFlag = 1;
	}
	if (argc == 5)
	{
		file = argv[1];
		updateFile = argv[2];
		traceFile = argv[3];
		chunkFile = argv[4];
		traceFlag = 1;
	}

	/*else
	{
	printf("input: name, lookup_file, update_file\n");
	system("pause");
	return 0;
	}
	*/

	/*open result file*/
	char fileName[40];
	int j = 0;
	for (j = strlen(file); file[j] != '\\' && j >= 0; j--);
	strcpy(fileName, file + j + 1);
	printf("%s\n", fileName);
	strcat(fileName, ".mybitmap.result.txt");
	resultFile = fopen(fileName, "w");
	if (resultFile == NULL)
		return 0;

	/* construct structure*/
	start(file);
	printf("construction over...\n");
	
	/*modifychunk*/
	overhead();
	modifyChunk(chunkFile);
	printf("chunk MODIFIED over...\n");
	overhead();
	/*lookup*/
	printf("trace flag = %d\n", traceFlag);
	trace_lookup(traceFile);
	printf("lookup over...\n");
	
	//traverse();

	/*update verify*/
	//printf("verifying update\n");
	//verify_update(file, updateFile);

	/*update */
	update(updateFile);
	printf("update over...\n");

	freeMemory();

	printf("all over...\n");
	fclose(resultFile);
	//system("pause");
}

