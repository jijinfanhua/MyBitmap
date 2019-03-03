#include "Fib.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <math.h>
#include <string.h>
#include <curses.h>
#include <sys/time.h>
#include <stdlib.h>
//#include <conio.h>


#define IP_LEN		32

//char * ribFile					= "rib.txt";				//original Rib file
char * updateFile = "org_update_2017.txt";
char * tracefile = "five_tuple_trace_equinix_modify.txt";
char * fibfile = "org_20172.txt";
//char * updateFile = "D:\\Tsinghua_OBMA\\fengyong_experiment\\data\\eqix_update_2017.txt";/*"E:\\document\\2017infocom\\data\\update\\isc.update";*/ /*"E:\\document\\2017infocom\\data\\update\\isc.update";*/ //"E:\\SVN\\infocom2017code\\data\\update\\eqix_update_2017.txt"; //update file in IP format
//char * tracefile = "D:\\Tsinghua_OBMA\\fengyong_experiment\\data\\five_tuple_trace_equinix_modify.txt";
//char * fibfile = "D:\\Tsinghua_OBMA\\fengyong_experiment\\data\\eqix_20172.txt";


char * oldPortfile1				= "oldport1.txt";
char * oldPortfile_bin1			= "oldport_bin1.txt";
char * oldPortfile2				= "oldport2.txt";
char * oldPortfile_bin2			= "oldport_bin2.txt";
char * oldPortfile3				= "oldport3.txt";
char * oldPortfile_bin3			= "oldport_bin3.txt";

char * newPortfile1				= "newport1.txt";
char * newPortfile_bin1			= "newport_bin1.txt";
char * newPortfile2				= "newport2.txt";
char * newPortfile_bin2			= "newport_bin2.txt";
char * newPortfile3				= "newport3.txt";
char * newPortfile_bin3			= "newport_bin3.txt";

char * trace_path				= "trace(100000).integer";
char * ribfileName				= "rib.txt.port";
char ret[IP_LEN+1];

#define UpdateFileCount		6
#define UPDATE_ALG	_MINI_REDUANDANCY_TWOTRAS

FILE *resultFile;


int KK;
int KKlist[] = {  1, 5, 8, 10, 15, 20, 30, 60, 100, 200, 350, 500, 700, 1000,1200, 1400, 1600, 1800, 2000, 3000, 4000, 5000, 7000, 10000, 20000, 50000, 399999999 };
int lookupnum = 0;
int updatenum = 0;

FILE * fytemp;

int FindFibTrieNextHop(FibTrie * m_trie, char * insert_C)
{
	int nextHop = -1;//init the return value
	FibTrie *insertNode = m_trie;

	if (insertNode->oldPort != 0) {
		nextHop = insertNode->oldPort;
	}

	int len=(int) strlen(insert_C);

	for (int i=0; i < (len + 1);i++)
	{		
		if ('0' == insert_C[i])
		{//if 0, turn left
			if (NULL != insertNode->lchild)	
			{
				insertNode = insertNode->lchild;
			}
			else {
				break;
			}
		}
		else
		{//if 1, turn right
			if (NULL != insertNode->rchild) {
				insertNode = insertNode->rchild;
			}
			else {
				break;
			}
		}

		if (insertNode->newPort != 0)	{
			nextHop = insertNode->newPort;
		}
	}

	return	nextHop;
}

//given a ip in binary---str and its length---len, return the next ip in binary
char * GetStringIP(char *str, int len)
{
	memset(ret,0,sizeof(ret));
	memcpy(ret,str,IP_LEN);
	int i;
	for (i=0;i<len;i++)
	{
		if ('0'==ret[i])
		{
			ret[i]='1';
			break;
		}
		else if ('1'==ret[i])
		{
			ret[i]='0';
		}
	}
	//printf("%s*\n",ret);
	return ret;
}

unsigned int btod(char *bstr)
{
	unsigned int d = 0;
	unsigned int len = (unsigned int)strlen(bstr);
	if (len > 32)
	{
		printf("too long\n");
		return -1; 
	}
	len--;
	for (unsigned int i = 0; i <= len; i++)
	{
		d += (bstr[i] - '0') * (1 << (len - i));
	}
	return d;
}

void sailDetectForFullIp(CFib *tFib) {
	int nonRouteStatic=0;

	int hop1=0;
	int hop2=0;

	char strIP00[IP_LEN + 1];
	memset(strIP00, 0, sizeof(strIP00));
	
	for (int tmp=0; tmp < IP_LEN; tmp++)
	{
		strIP00[tmp]='0';
	}

	int len88 = (int)strlen(strIP00);

	char new_tmp[IP_LEN + 1];
	char old_tmp[IP_LEN + 1];

	memset(new_tmp, 0, sizeof(new_tmp));
	memset(new_tmp, 0, sizeof(new_tmp));
	memcpy(new_tmp, strIP00, IP_LEN);

	double zhishuI = pow((double)2,(double)IP_LEN);

	bool ifhalved = false;
	printf("\t\ttotal\t%.0f\t\n", zhishuI);
	printf("\t\tlength\tcycles\t\tpercent\tnexthop\n");

	for (long long k=0; k < zhishuI; k++)
	{
		memcpy(old_tmp, new_tmp, IP_LEN);
		memcpy(new_tmp, GetStringIP(old_tmp, IP_LEN), IP_LEN);
		
		hop1 = FindFibTrieNextHop(tFib->m_pTrie, new_tmp);
	
		unsigned int IPInteger = btod(new_tmp);
		hop2 = tFib->sailLookup(IPInteger);

		if (hop1== -1 && hop2 != hop1)
		{
			nonRouteStatic++;
			continue;
		}

		double ratio=0;
		
		if (hop2 != hop1)
		{
			printf("%d:%d", hop1, hop2);
			printf("\n\n\t\tNot Equal!!!\n");
			getchar();
		}
		else 
		{
			if (k%100000 == 0)
			{
				ratio=k/(double)(zhishuI/100);
				printf("\r\t\t%d\t%lld\t%.2f%%\t%d             ", IP_LEN, k, ratio, hop1);
			}
		}
	}

	printf("\n\t\tTotal number of garbage roaming route��%d",nonRouteStatic);
	printf("\n\t\tEqual!!!!\n");
}

void help () {
	printf ("#######################################\n");
	printf ("##  *-*-*-*-OH algorithm-*-*-*-*-*   ##\n");
	printf ("#   {para} = [trace name] [rib name]  #\n");
	printf ("##       trace_path   ribfileName    ##\n");
	printf ("#######################################\n");
	//system ("pause");
	printf("Press Enter key to continue...");
	//fgetc(stdin);  //fy    1211
}
// Levelpushing Trie Update
unsigned int BFLevelPushingTrieUpdate(string sFileName,CFib *tFib)
{
	unsigned int		iEntryCount = 0;					//the number of items from file
	char				sPrefix[20];						//prefix from rib file
	unsigned long		lPrefix;							//the value of Prefix
	unsigned int		iPrefixLen;							//the length of PREFIX
	int					iNextHop;							//to store NEXTHOP in RIB file

	char				operate_type_read;
	int 				operate_type;
	int					readlines = 0;
	unsigned long long	updatetimeused = 0;

	long				yearmonthday=0;						//an integer to record year, month, day
	long				hourminsec=0;						//an integer to record hour, minute, second
	long				yearmonthday_old=0;					//an integer to record year, month, day
	long				hourminsec_old=0;					//an integer to record hour, minute, second
	
	long				outputCount=0;
	long				insertNum_old=0;
	long				DelNum_old=0;
	long				readlines_old=0;
	unsigned long long sum = 0;

	struct timeval time_start, time_end;
	double time_use;
	double update_speed;

	FILE * update_result = fopen("eqix_sail_update.txt","w");
	
	ifstream fin(sFileName.c_str());
	if (!fin)
	{
		return 0;
	}
	printf("\n	Parsing %s\n", sFileName.c_str());

	int cmax = 0;
	int cmin = (1 << 30);

	//iNextHop  insert_C  operate_Type  sPrefix
	while (!fin.eof() && readlines < 100000) {		//UPDATEMAX   fy  	// Add by Qiaobin Fu 2014-4-22
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = -9;

		memset(sPrefix, 0, sizeof(sPrefix));

		//read data from rib file, iNextHop attention !!!
		fin >> operate_type_read >> sPrefix;	//>> iNextHop;
		
		if ('W' == operate_type_read) {
			//fin >> iNextHop;
			iNextHop = 1; //to avodi error
			operate_type = _DELETE;
			//printf("%c\t%s\n", operate_type_read, sPrefix);
		}
		else if ('A' == operate_type_read)
		{
			fin >> iNextHop;// there is sPrefix
			operate_type = _NOT_DELETE;
			//printf("%c\t%s\t%d\n", operate_type_read, sPrefix,iNextHop);
		}
		else
		{
			printf("\tFormat of update file Error, quit....\n");
			getchar();
			return 0;
		}
		

		//printf("read ok!");
		int iStart = 0;								//the end point of IP
		int iEnd = 0;								//the end point of IP
		int iFieldIndex = 3;
		int iLen = (int)strlen(sPrefix);			//the length of Prefix

		if (iLen > 0)
		{
			readlines++;
			if (readlines % 1000000 == 0)
				printf("%d M\r", readlines / 100000);

			//����lPrefix��iPrefixLen
			for (int i = 0; i<iLen; i++)
			{
				//extract the first 3 sub-part
				if (sPrefix[i] == '.')
				{
					iEnd = i;
					string strVal(sPrefix + iStart, iEnd - iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
					iFieldIndex--;
					iStart = i + 1;
					i++;
				}

				if (sPrefix[i] == '/') {
					//extract the 4th sub-part
					iEnd = i;
					string strVal(sPrefix + iStart, iEnd - iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i + 1;

					//extract the length of prefix
					i++;
					strVal = string(sPrefix + iStart, iLen - 1);
					iPrefixLen = atoi(strVal.c_str());
				}
			}

			char insert_C[50];
			memset(insert_C, 0, sizeof(insert_C));
			//insert the current node into Trie tree
			for (unsigned int yi = 0; yi < iPrefixLen; yi++)
			{
				//turn right
				if (((lPrefix << yi) & HIGHTBIT) == HIGHTBIT) insert_C[yi] = '1';
				else insert_C[yi] = '0';
			}
			//printf("%s\/%d\t%d\n",insert_C,iPrefixLen,iNextHop);
			//printf("%d\t%s\t%s\n", iNextHop, insert_C, sPrefix);
			if (iPrefixLen < 8) {
				//printf("%d-%d; ", iPrefixLen, iNextHop);
			}
			else
			{
				fprintf(update_result, "%lu\t%u\t", lPrefix, iPrefixLen);
				gettimeofday(&time_start,NULL);
				tFib->Update(iNextHop, insert_C, operate_type, sPrefix);
				gettimeofday(&time_end,NULL);
				fprintf(update_result, "%llu\n", (time_end.tv_sec-time_start.tv_sec)*1000000 + (time_end.tv_usec-time_start.tv_usec));
				double res = (time_end.tv_sec-time_start.tv_sec)*1000000 + (time_end.tv_usec-time_start.tv_usec);
				if (res < cmin)cmin = res;
				if (res > cmax)cmax = res;
				time_use += res;
			}
		}
	}

	update_speed = readlines * 1.0 / (time_use * 1.0 / 1000000) / 1000000;
	fprintf(fytemp,"%lf\n",update_speed);
	printf("all_time:\t%lf\naverage update speed:\t%lf\nmax update time:\t%dus\nmin update time:\t%dus\n", time_use, update_speed, cmax, cmin);

	FILE * myfp = fopen("sail.update.txt","at");
	fprintf(myfp,"org\t%lf\n",update_speed);
	fclose(myfp);

	fclose(fytemp);
	fclose(update_result);
	return readlines;
}

void amination()
{
	//system("color 1D");
	//system("mode con cols=75 lines=40 &color 3f");
	int sleeptime=200;
	printf("\t\t    |-*-*-*-*-*-*-     -*-*-*-*-*-*-*-|\n");
	printf("\t\t    |-*-*-*-                 -*-*-*-*-|\n");
	printf("\t\t    |          S                      |");
	sleep(sleeptime);
	printf("\r\t\t    |          SA                     |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAI                    |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL                  |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL A                |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Al               |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Alg              |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algo             |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algor            |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algori           |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algorit          |");
	sleep(sleeptime);
	printf("\r\t\t    |          SAIL Algoritm         |\n");


	printf("\t\t    |by Tong Yang & Qiaobin Fu        |");
	sleep(sleeptime);
	printf("\r\t\t    |by Tong Yang & Qiaobin Fu        |");
	sleep(sleeptime);
	printf("\r\t\t    | by Tong Yang & Qiaobin Fu       |");
	sleep(sleeptime);
	printf("\r\t\t    |   by Tong Yang & Qiaobin Fu     |");
	sleep(sleeptime);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");

	sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |");
	sleep(sleeptime/2);
	printf("\r\t\t    |                                 |");
	sleep(sleeptime/2);
	printf("\r\t\t    |    by Tong Yang & Qiaobin Fu    |\n");
	sleep(sleeptime/2);
	printf("\r\t\t    |  Directed by Prof. Gaogang Xie  |\n");
	sleep(sleeptime);
	printf("\r\t\t    |         In Sigcomm 2014         |\n");
	sleep(sleeptime);
	printf("\r\t\t    |          ICT,CAS,China          |\n");
	sleep(sleeptime);
	printf("\t\t    |-*-*-*-                 -*-*-*-*-|\n");
	sleep(sleeptime);
	printf("\t\t    |-*-*-*-*-*-*-     -*-*-*-*-*-*-*-|\n");
}

void sailPerformanceTest(char *traffic_file, char* fib_file)
{
	//printf("\t\t\t********************************************\n");
	//printf("\t\t\t*-*-*         sail algorithm        *-*-*-*\n");
	//printf("\t\t\t*-*-*            ICT, CAS            *-*-*-*\n");
	//printf("\t\t\t********************************************\n");

	//amination();
	FILE * lookup_result = fopen("eqix_sail_lookup.txt","w");
	printf("\n\nsail algorithm starts...\n\n");
	CFib tFib = CFib();
	tFib.BuildFibFromFile(fib_file);
	printf("build fib from file succeed!\n");
	unsigned int *traffic=tFib.TrafficRead(traffic_file);

	//register unsigned char LPMPort=0;
	unsigned int LPMPort = 0;

	FILE * temp = fopen("sail_lookup_result.txt","w");
	int result_pointer = 0;

	struct timeval start_time, end_time;
	double time_use, lookup_speed;


	gettimeofday(&start_time, NULL);
	for (int j=0;j<10;j++)
	{
		for (int i=0;i<TRACE_READ;i++)//	//fy
		{
			
			LPMPort = tFib.sailLookup(traffic[i]);
		}
	}
	gettimeofday(&end_time,NULL);
	time_use = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;//ms =10e-6 s
	printf("time_use: %lf\t count: %d\n",time_use,TRACE_READ);

	lookup_speed = 1.0 / (time_use * 1.0 / 1000000 / ( TRACE_READ * 10 )) / 1000000;

//	FILE * myfp = fopen("sail.real.lookup.txt","at");
//	fprintf(myfp,"eqix\t%lf\n",lookup_speed);
//	fclose(myfp);

	fprintf(fytemp,"%lf\t",lookup_speed);
	printf("all_time: %lf \t lookup_speed: %lf\n \t hops:%u\n", time_use, lookup_speed, LPMPort);

	//for (int i = 0x00000000; i <= 0xfffffffe; i++)
	//{
	//	LPMPort = tFib.sailLookup(i);
	//	printf("%u\n", LPMPort);
	//	getchar();
	//}
	//printf("\tLMPport=%d\n\tLookup time=%u\n\tThroughput is:\t %.3f Mpps\n",LPMPort,Lookuptime, 1.0*TRACE_READ/Lookuptime);
	
	double size = (tFib.currentChunkNum24 * 256 * (sizeof(char) + sizeof(short))+ tFib.currentChunkNum32 * 256 * sizeof(char) + sizeof(sailTable_16) + sizeof(tFib.map));
	size = size / 1000000; //MB
	printf("size : %lf MB\n", size);
	fclose(lookup_result);
	fprintf(resultFile, "[Size]\nsize:\t%lf MB\n", size);
	fprintf(resultFile, "[Lookup]\nall_time: %lf \t lookup_speed: %lf\n", time_use, lookup_speed);
	int updateEntryCount = BFLevelPushingTrieUpdate(updateFile, &tFib);
	//system("pause");
	printf("Press Enter key to continue...");
	//  fgetc(stdin);
}


/*void test(int argc, char** argv)
{
	printf("\t\tStage One: The Initial Trie Construction\n");
	//build FibTrie
	CFib tFib = CFib();

	tFib.ytGetNodeCounts();
	printf("\nThe total number of Trie node is :\t%u.\n", tFib.allNodeCount);
	printf("The total number of solid Trie node is :\t%d.\n", tFib.solidNodeCount);

	//tFib.outputPortMapping(portMapFile1);
	//tFib.OutputTrie(tFib.m_pTrie, newPortfile1, oldPortfile1);
	
	printf("\n***********************Trie Correct Test***********************\n");
	
	if (!tFib.isCorrectTrie(tFib.m_pTrie)) {
		printf("The trie structure is incorrect!!!\n");
	}
	else {
		printf("The trie structure is correct!\n");
	}

	tFib.ytTriePortTest(tFib.m_pTrie);
	printf("******************************End******************************\n");

	//system("pause");

	printf("\n\n\t\tStage Two: The First Round Update\n");
	unsigned int iEntryCount = 0;
	unsigned int updateEntryCount = 0;

	iEntryCount = tFib.BuildFibFromFile(fibfile);



	tFib.ytGetNodeCounts();
	printf("\nThe total number of Trie node is :\t%u.\n",tFib.allNodeCount);
	printf("The total number of solid Trie node is :\t%d.\n", tFib.solidNodeCount);
	printf("The total number of routing items in FRib file is :\t%u.\n", iEntryCount);

	//tFib.outputPortMapping(portMapFile2);
	//tFib.OutputTrie(tFib.m_pTrie, newPortfile2, oldPortfile2);

	printf("\n***********************Trie Correct Test***********************\n");
	
	if (!tFib.isCorrectTrie(tFib.m_pTrie)) {
		printf("The trie structure is incorrect!!!\n");
	}
	else {
		printf("The trie structure is correct!\n");
	}

	tFib.ytTriePortTest(tFib.m_pTrie);
	printf("******************************End******************************\n");

	//tFib.checkTable(tFib.m_pTrie, 0);

	//printf("\n************************sail Lookup Correct Test************************\n");
	//sailDetectForFullIp(&tFib);
	//printf("***********************************End***********************************\n");

	//system("pause");

	printf("\n\n\t\tStage Three: The Second Round Update\n");
	//update FibTrie stage
	//updateEntryCount = BFLevelPushingTrieUpdate(updateFile, &tFib);
	tFib.ytGetNodeCounts();
	printf("\nThe total memory access is :\t%llu.\n", tFib.memory_access);
	printf("The total number of Trie node is :\t%d.\n", tFib.allNodeCount);
	printf("The total number of solid Trie node is :\t%u.\n", tFib.solidNodeCount);
	printf("The total number of updated routing items is :\t%u.\n", updateEntryCount);
	
	//tFib.outputPortMapping(portMapFile3);
	//tFib.OutputTrie(tFib.m_pTrie, newPortfile3, oldPortfile3);

	printf("\n************************Trie Correct Test************************\n");
	
	if (!tFib.isCorrectTrie(tFib.m_pTrie)) {
		printf("The trie structure is incorrect!!!\n");
	}
	else {
		printf("The trie structure is correct!\n");
	}

	tFib.ytTriePortTest(tFib.m_pTrie);
	printf("*******************************End*******************************\n");

	printf("\n\n\t\tUpdate Statistics\n");
	printf("\nThe total number of true update items is :\t%u.\n", tFib.trueUpdateNum);
	printf("The total number of invalid update items is :\t%u.\n", tFib.invalid);
	printf("The detailed invalid items:\n\tinvalid0 = %u\tinvalid1 = %u\tinvalid2 = %u\n", tFib.invalid0, tFib.invalid1, tFib.invalid2);

	//system("pause");
	//tFib.checkTable(tFib.m_pTrie, 0);

	printf("\n\n************************sail Lookup Correct Test************************\n");
	sailDetectForFullIp(&tFib);
	printf("***********************************End***********************************\n");

	printf("\nMission Complete, Press any key to continue...\n");
	//system("pause");
}*/

/*
int inputTraceMean(RouterEntry *p, int max) {
	printf("here 1...\n");
	FILE *fpLookup;
	fpLookup = fopen(tracefile, "r");
	if (fpLookup == NULL)
	{
		printf("open lookupfile error\n");
		exit(0);
	}

	int cur = 0;
	char c;
	unsigned int ipad;


	unsigned int		iEntryCount = 0;					//the number of items from file
	char				sPrefix[20];						//prefix from rib file
	unsigned long		lPrefix;							//the value of Prefix
	unsigned int		iPrefixLen;							//the length of PREFIX
	int					iNextHop;							//to store NEXTHOP in RIB file

	char				operate_type_read;
	int 				operate_type;
	int					readlines = 0;
	unsigned long long	updatetimeused = 0;
	char				insert_C[50];

	ifstream fin(updateFile);
	if (!fin)
	{
		return 0;
	}
	//printf("here 2...\n");
	while (cur < max) {
		if (cur % KK == 0) {
			lPrefix = 0;
			iPrefixLen = 0;
			iNextHop = -9;


			memset(sPrefix, 0, sizeof(sPrefix));

			//read data from rib file, iNextHop attention !!!
			fin >> operate_type_read >> sPrefix;	//>> iNextHop;
			//printf("operate_type:%c, sPrefix:%s\n", operate_type_read, sPrefix);
			if ('W' == operate_type_read) {
				//fin >> iNextHop;
				iNextHop = 1; //to avodi error
				operate_type = _DELETE;
				//printf("%c\t%s\n", operate_type_read, sPrefix);
			}
			else if ('A' == operate_type_read)
			{
				fin >> iNextHop;// there is sPrefix
				operate_type = _NOT_DELETE;
				//printf("%c\t%s\t%d\n", operate_type_read, sPrefix,iNextHop);
			}
			else
			{
				printf("\tFormat of update file Error, quit....\n");
				getchar();
				return 0;
			}

			//printf("here 3..\n");
			//printf("read ok!");
			int iStart = 0;								//the end point of IP
			int iEnd = 0;								//the end point of IP
			int iFieldIndex = 3;
			int iLen = (int)strlen(sPrefix);			//the length of Prefix

			if (iLen > 0)
			{
				readlines++;

				for (int i = 0; i < iLen; i++)
				{
					//extract the first 3 sub-part
					if (sPrefix[i] == '.')
					{
						iEnd = i;
						string strVal(sPrefix + iStart, iEnd - iStart);
						lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
						iFieldIndex--;
						iStart = i + 1;
						i++;
					}

					if (sPrefix[i] == '/') {
						//extract the 4th sub-part
						iEnd = i;
						string strVal(sPrefix + iStart, iEnd - iStart);
						lPrefix += atol(strVal.c_str());
						iStart = i + 1;

						//extract the length of prefix
						i++;
						strVal = string(sPrefix + iStart, iLen - 1);
						iPrefixLen = atoi(strVal.c_str());
					}
				}

				
				memset(insert_C, 0, sizeof(insert_C));
				for (unsigned int yi = 0; yi < iPrefixLen; yi++)
				{
					//turn right
					if (((lPrefix << yi) & HIGHTBIT) == HIGHTBIT) insert_C[yi] = '1';
					else insert_C[yi] = '0';
				}
			}
			//printf("here 4...\n");
			if (operate_type == _DELETE) {
				p->oprate_type = 1;
				p->iNextHop = iNextHop;
				p->ipnet = lPrefix;
				memcpy(p->insert_C, insert_C, sizeof(p->insert_C));
				memcpy(p->sPrefix, sPrefix, sizeof(p->sPrefix));
				updatenum++;
			}else if (operate_type == _NOT_DELETE) {
				//printf("here 5...\n");
				p->oprate_type = 0;
				//printf("here 8...\n");
				p->iNextHop = iNextHop;
				p->ipnet = lPrefix;
				//printf("here 6...\n");
				memcpy(p->insert_C, insert_C, sizeof(p->insert_C));
				//printf("here 7...\n");
				memcpy(p->sPrefix, sPrefix, sizeof(p->sPrefix));
				updatenum++;
			}
			
		}else {
			if (fscanf(fpLookup, "%u\n", &ipad) == -1) break;
			p->oprate_type = 2;
			p->ipnet = ipad;
			lookupnum++;
		}
		p++;
		cur++;
	}
	//printf("here 3...\n");
	fclose(fpLookup);
	fin.close();
	printf("hahahhahaha cur=%u, max = %u\n", cur, max);
	return cur;
}

int updateLookup(RouterEntry * list, int max, CFib * tFib) {
	RouterEntry * p = list;
	int all = 0;

	double time_use = 0.0;
	struct timeval start_time, end_time;

	//gettimeofday(&start_time,NULL);
	for (p = list; p < list + max; p++) {
		if (p->oprate_type == 2) {
			//gettimeofday(&start_time,NULL);
			all += tFib->sailLookup(p->ipnet);
			//gettimeofday(&end_time,NULL);
		}else if (p->oprate_type == 0) {
			//gettimeofday(&start_time,NULL);
			tFib->Update(p->iNextHop, p->insert_C, p->oprate_type, p->sPrefix);
			//gettimeofday(&end_time,NULL);
		}
		else if (p->oprate_type == 1) {
			//gettimeofday(&start_time,NULL);
			tFib->Update(p->iNextHop, p->insert_C, p->oprate_type, p->sPrefix);
			//gettimeofday(&end_time,NULL);
		}

	}
	//gettimeofday(&end_time,NULL);
	//time_use = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;
	//printf("The time use is: %lf, and lookupnum is: %d, updatenum is %d\n",time_use, lookupnum, updatenum);

	return 0;
}

int a1(int argc, char** argv) {
	int t = 0;

	struct timeval time_start, time_end;
	double time_use=0, speed;
	double lookup_speed, update_speed;

	char fileName[40];
	int j = 0;
	for (j = strlen(fibfile); fibfile[j] != '\\' && j >= 0; j--);
	strcpy(fileName, fibfile + j + 1);
	printf("%s\n", fileName);
	strcat(fileName, ".sail.result.txt");
	resultFile = fopen(fileName, "w");
	if (resultFile == NULL)
		return 0;

	printf("\n\nsail algorithm starts...\n\n");
	getchar();
	CFib tFib = CFib();
	tFib.BuildFibFromFile(fibfile);

	printf("build fib from file succeed!\n");
	getchar();

	int max = 10000000;
	//RouterEntry list[1<<25];
	RouterEntry *list = (RouterEntry*)malloc(sizeof(struct RouterEntry)*max);//9000000
	printf("size:%d\n",sizeof(struct RouterEntry));
	FILE * fytemp1;
	fytemp1 = fopen("sail_eqix_update_lookup_mix_result.txt", "at");

	printf("11111...\n");

	int i = 26;
	if (argc == 2) {
		i = atoi(argv[1]);
	}
		
	memset(list, 0, sizeof(struct RouterEntry)* max);
	printf("sizeof(RouterEntry): %d\n", sizeof(RouterEntry) * max);
	//system("pause");
	printf("2222...\n");
	KK = KKlist[i];
	lookupnum = 0;
	updatenum = 0;
		
	max = inputTraceMean(list, max);

	gettimeofday(&time_start,NULL);
	int all = updateLookup(list, max, &tFib);
	gettimeofday(&time_end,NULL);

	time_use = (time_end.tv_usec - time_start.tv_usec) + (time_end.tv_sec - time_start.tv_sec) * 1000000.0 ;
	lookup_speed = lookupnum * 1.0 / (time_use * 1.0 / 1000000) / 1000000;
	update_speed = updatenum * 1.0 / (time_use * 1.0 / 1000000) / 1000000;
	//fprintf(fp, "%d\t%d\t%d\t%d\t%lf\n", lookupnum, updatenum, max, KK, time_use);
	printf("%d\t%d\t%d\t%d\t%lf\t%u\n", lookupnum, updatenum, max, KK, time_use, all);
	fprintf(fytemp1, "%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\n", lookupnum, updatenum, max, KK, time_use, lookup_speed, update_speed);
	printf("%d\t%d\t%d\t%d\t%lf\tlookupspeed:%lf\tupdatenum:%lf\n", lookupnum, updatenum, max, KK, time_use, lookup_speed, update_speed);
	free(list);
	fclose(fytemp1);
	printf("all over...\n");
	return 0;
}*/

int main (int argc, char** argv) {

	if (argc == 4)
	{
		char * fibfile = argv[1];
		updateFile = argv[2];
		char * tracefile = argv[3];
		
		char fileName[40];
		int j = 0;
		for (j = strlen(fibfile); fibfile[j] != '\\' && j >= 0; j--);
		strcpy(fileName, fibfile + j + 1);
		printf("%s\n", fileName);
		strcat(fileName, ".sail.result.txt");
		resultFile = fopen(fileName, "w");
		if (resultFile == NULL)
			return 0;
		//sailPerformanceTest(tracefile, fibfile);
	}
	else
	{
		//char * tracefile = "E:\\SVN\\infocom2017code\\SAIL\\SAIL\\five_tuple_trace_equinix_modify.txt";
		//char * tracefile = "five_tuple_trace_equinix_modify.txt";//"E:\\SVN\\infocom2017code\\data\\trace\\five_tuple_trace_equinix_modify.txt";
		//char * fibfile = "eqix_20172.txt";//"E:\\SVN\\infocom2017code\\data\\rib\\eqix_20172.txt"; //"E://SVN//infocom2017//SAIL//SAIL//rrc01-12.1.1.txt";

		fytemp = fopen("sail_eqix_lookup_update_result.txt","at");

		char fileName[40];
		int j = 0;
		for (j = strlen(fibfile); fibfile[j] != '\\' && j >= 0; j--);
		strcpy(fileName, fibfile + j + 1);
		printf("%s\n", fileName);
		strcat(fileName, ".sail.result.txt");
		resultFile = fopen(fileName, "w");
		if (resultFile == NULL)
			return 0;
		
		sailPerformanceTest(tracefile, fibfile);
	}
}
