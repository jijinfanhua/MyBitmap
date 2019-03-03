/*_
 * Copyright (c) 2014-2016 Hirochika Asai <asai@jar.jp>
 * All rights reserved.
 */

#include "poptrie.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef char CHAR;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;

int KK;
int KKlist[] = { 1, 5, 8, 10, 15, 20, 30, 60, 100, 200, 350, 500, 700, 1000,1200, 1400, 1600, 1800, 2000, 3000, 4000, 5000, 7000, 10000, 20000, 50000, 399999999 };
int lookupnum = 0, updatenum = 0;
char * fibfile = "/home/one/eclipse-workspace/data/isc_20172.txt";
char * traceFile = "/home/one/eclipse-workspace/data/five_tuple_trace_equinix_modify.txt";
char * updateFile = "/home/one/eclipse-workspace/data/isc_update_2017.txt";
struct poptrie * pop;

FILE * fytemp;

typedef struct {
	u32 ipnet;
	u32 port;
	ushort length:8;
	ushort type:8;
}RouterEntry;

typedef union _LARGE_INTEGER{
	struct{
		DWORD LowPart;
		DWORD HigfPart;
	};
	struct{
		DWORD LowPart;
		DWORD HighPart;
	}u;
	LONGLONG QuadPart;
}LARGE_INTEGER;

/* Macro for testing */
#define TEST_FUNC(str, func, ret)                \
    do {                                         \
        printf("%s: ", str);                     \
        if ( 0 == func() ) {                     \
            printf("passed");                    \
        } else {                                 \
            printf("failed");                    \
            ret = -1;                            \
        }                                        \
        printf("\n");                            \
    } while ( 0 )

#define TEST_PROGRESS()                              \
    do {                                             \
        printf(".");                                 \
        fflush(stdout);                              \
    } while ( 0 )


/*
 * Initialization test
 */
static int
test_init(void)
{
    struct poptrie *poptrie;

    /* Initialize */
    poptrie = poptrie_init(NULL, 19, 22);
    if ( NULL == poptrie ) {
        return -1;
    }

    TEST_PROGRESS();

    /* Release */
    poptrie_release(poptrie);

    return 0;
}

static int
test_lookup(void)
{
    struct poptrie *poptrie;
    int ret;
    void *nexthop;

    /* Initialize */
    poptrie = poptrie_init(NULL, 19, 22);
    if ( NULL == poptrie ) {
        return -1;
    }

    /* No route must be found */
    if ( NULL != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }

    /* Route add */
    nexthop = (void *)1234;
    ret = poptrie_route_add(poptrie, 0x1c001200, 24, nexthop);
    if ( ret < 0 ) {
        /* Failed to add */
        return -1;
    }
    if ( nexthop != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }
    TEST_PROGRESS();

    /* Route update */
    nexthop = (void *)5678;
    ret = poptrie_route_update(poptrie, 0x1c001200, 24, nexthop);
    if ( ret < 0 ) {
        /* Failed to update */
        return -1;
    }
    if ( nexthop != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }
    TEST_PROGRESS();

    /* Route delete */
    ret = poptrie_route_del(poptrie, 0x1c001200, 24);
    if ( ret < 0 ) {
        /* Failed to update */
        return -1;
    }
    if ( NULL != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }
    TEST_PROGRESS();

    /* Release */
    poptrie_release(poptrie);

    return 0;
}

static int
test_lookup2(void)
{
    struct poptrie *poptrie;
    int ret;
    void *nexthop;

    /* Initialize */
    poptrie = poptrie_init(NULL, 19, 22);
    if ( NULL == poptrie ) {
        return -1;
    }

    /* No route must be found */
    if ( NULL != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }

    /* Route add */
    nexthop = (void *)1234;
    ret = poptrie_route_add(poptrie, 0x1c001203, 32, nexthop);
    if ( ret < 0 ) {
        /* Failed to add */
        return -1;
    }
    if ( nexthop != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }
    TEST_PROGRESS();

    /* Route update */
    nexthop = (void *)5678;
    ret = poptrie_route_update(poptrie, 0x1c001203, 32, nexthop);
    if ( ret < 0 ) {
        /* Failed to update */
        return -1;
    }
    if ( nexthop != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }
    TEST_PROGRESS();

    /* Route delete */
    ret = poptrie_route_del(poptrie, 0x1c001203, 32);
    if ( ret < 0 ) {
        /* Failed to update */
        return -1;
    }
    if ( NULL != poptrie_lookup(poptrie, 0x1c001203) ) {
        return -1;
    }
    TEST_PROGRESS();

    /* Release */
    poptrie_release(poptrie);

    return 0;
}

static int
test_lookup_linx(void)
{
    struct poptrie *poptrie;
    FILE *fp;
    char buf[4096];
    int prefix[4];
    int prefixlen;
    int nexthop[4];
    int ret;
    u32 addr1;
    u32 addr2;
    u64 i;

    char * tempFile = "poptrie_lookup_result.txt";
    int result_pointer = 0;

    unsigned long long time_start, time_end;
    unsigned long long sum = 0;
    double average_cycle;
    uint hop = 0;
    uint count = 0;

    FILE * temp = fopen(tempFile, "w");

    /* Load from the linx file */
    //fp = fopen("tests/isc.2016.out", "r");
    fp = fopen("org_2016.txt", "r");
    if ( NULL == fp ) {
        return -1;
    }

    /* Initialize */
    printf("111\n");
    //getchar();
    //printf("222\n");
    poptrie = poptrie_init(NULL, 18, 22);
    if ( NULL == poptrie ) {
        return -1;
    }

    /* Load the full route */
    i = 0;
    while ( !feof(fp) ) {
        if ( !fgets(buf, sizeof(buf), fp) ) {
            continue;
        }
        ret = sscanf(buf, "%d.%d.%d.%d/%d %d", &prefix[0], &prefix[1],
                     &prefix[2], &prefix[3], &prefixlen, &addr2);
        if ( ret < 0 ) {
            return -1;
        }

        /* Convert to u32 */
        addr1 = ((u32)prefix[0] << 24) + ((u32)prefix[1] << 16)
            + ((u32)prefix[2] << 8) + (u32)prefix[3];
        //addr2 = ((u32)nexthop[0] << 24) + ((u32)nexthop[1] << 16) + ((u32)nexthop[2] << 8) + (u32)nexthop[3];

        /* Add an entry */
        ret = poptrie_route_add(poptrie, addr1, prefixlen, (void *)(u64)addr2);
        if ( ret < 0 ) {
            return -1;
        }
        //if ( 0 == i % 10000 ) {
        //    TEST_PROGRESS();
       // }
        i++;
    }
    printf("333\n");
    //getchar();
    /*our lookup statistic*/
    char * tracePath= "five_tuple_trace_equinix_modify.txt";
    FILE *fpTrace = fopen(tracePath, "r");

	uint *list = (uint *)malloc(sizeof(uint)* (1e8));
	printf("\n");
	while (!feof(fpTrace) && count < 1e7)
	{
		//printf("here\n");
		fscanf(fpTrace, "%u", &list[count++]);
		if (count % 1000000 == 0) printf("%d M\r", (count / 1000000));
	}


    unsigned int LPMPort = 0;

	//2825683
    struct timeval start_time, end_time;
    double time_use, lookup_speed;
    printf("here we go! count:%u\n",count);
    gettimeofday(&start_time, NULL);
	register int j;
    for (j = 0; j < count; j++)
    {
        {
            LPMPort = poptrie_lookup(poptrie, list[j]);
        }
    }
    gettimeofday(&end_time, NULL);

    time_use = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;

    lookup_speed = count * 1.0 / (time_use * 1.0 / 1000000) / 1000000;

    FILE * myfp = fopen("poptrie.real.lookup.txt","at");
    fprintf(myfp,"eqix\t%lf\n",lookup_speed);
    fclose(myfp);

    fprintf(fytemp,"%lf\t",lookup_speed);
    printf("LPMPort: %u\t time_use: %lf\t count: %u\t lookup_speed: %lf\n", LPMPort, time_use, count, lookup_speed);
    printf("\ndown!\n");
    //average_cycle = sum*1.0 / 0xffffffe;

    /* Release */
    poptrie_release(poptrie);

    /* Close */
    fclose(temp);
    fclose(fp);

    return 0;
}

static int
test_lookup_linx_update(void)
{
    struct poptrie *poptrie;
    FILE *fp;
    char buf[4096];
    int prefix[4];
    int prefixlen;
    int nexthop[4];
    int ret;
    u32 addr1;
    u32 addr2;
    u64 i;
    int tm;
    char type;

    /* Initialize */
    poptrie = poptrie_init(NULL, 19, 22);
    if ( NULL == poptrie ) {
        return -1;
    }

    /* Load from the linx file */
    fp = fopen("eqix_20172.txt", "r");
    if ( NULL == fp ) {
        return -1;
    }

    /* Load the full route */
    i = 0;
    while ( !feof(fp) ) {
        if ( !fgets(buf, sizeof(buf), fp) ) {
            continue;
        }
        ret = sscanf(buf, "%d.%d.%d.%d/%d %d", &prefix[0], &prefix[1],
                     &prefix[2], &prefix[3], &prefixlen, &addr2);
        if ( ret < 0 ) {
            return -1;
        }

        /* Convert to u32 */
        addr1 = ((u32)prefix[0] << 24) + ((u32)prefix[1] << 16)
            + ((u32)prefix[2] << 8) + (u32)prefix[3];
        //addr2 = ((u32)nexthop[0] << 24) + ((u32)nexthop[1] << 16)
        //    + ((u32)nexthop[2] << 8) + (u32)nexthop[3];

        /* Add an entry */
        ret = poptrie_route_add(poptrie, addr1, prefixlen, (void *)(u64)addr2);
        if ( ret < 0 ) {
            return -1;
        }
        if ( 0 == i % 10000 ) {
            TEST_PROGRESS();
        }
        i++;
    }

    /* Close */
    fclose(fp);

    /* Load from the update file */
    fp = fopen("eqix_update_2017.txt", "r");
    if ( NULL == fp ) {
        return -1;
    }

    struct timeval start_time, end_time;
    double sum = 0.0;
    double speed = 0.0;

    /* Load the full route */
    i = 0;
    while ( !feof(fp) && i < 1000000 ) {
        if ( !fgets(buf, sizeof(buf), fp) ) {
            continue;
        }
        //ret = sscanf(buf, "%d %c %d.%d.%d.%d/%d %d.%d.%d.%d", &tm, &type,
        //             &prefix[0], &prefix[1], &prefix[2], &prefix[3], &prefixlen,
        //             &nexthop[0], &nexthop[1], &nexthop[2], &nexthop[3]);
        ret = sscanf(buf, "%c %d.%d.%d.%d/%d %d", &type,
                     &prefix[0], &prefix[1], &prefix[2], &prefix[3], &prefixlen, &addr2);
        if ( ret < 0 ) {
            return -1;
        }

        /* Convert to u32 */
        addr1 = ((u32)prefix[0] << 24) + ((u32)prefix[1] << 16)
            + ((u32)prefix[2] << 8) + (u32)prefix[3];
        //addr2 = ((u32)nexthop[0] << 24) + ((u32)nexthop[1] << 16)
        //    + ((u32)nexthop[2] << 8) + (u32)nexthop[3];
        //printf("type:%c",type);
        if ( 'A' == type ) {
            /* Add an entry (use update) */
        	gettimeofday(&start_time, NULL);
            ret = poptrie_route_update(poptrie, addr1, prefixlen, (void *)(u64)addr2);
            gettimeofday(&end_time, NULL);
            sum = sum + (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000.0;
            if ( ret < 0 ) {
                return -1;
            }
        } else if ( 'W' == type ) {
            /* Delete an entry */
        	gettimeofday(&start_time, NULL);
            ret = poptrie_route_del(poptrie, addr1, prefixlen);
            gettimeofday(&end_time, NULL);
            sum = sum + (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000.0;
            if ( ret < 0 ) {
                /* Ignore any errors */
            }
        }
        //sum += (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000.0;
        //if ( 0 == i % 1000 ) {
        //    TEST_PROGRESS();
        //}
        i++;
    }
    speed = i * 1.0 / (sum * 1.0 / 1000000) / 1000000;

    FILE * myfp = fopen("poptrie.update.txt","at");
    fprintf(myfp,"eqix\t%lf\n",speed);
    fclose(myfp);

    printf("\nsum:%lf\thop:%d\tcount:%d\tupdate_speed:%lf\n", sum, ret, i, speed);
    fprintf(fytemp,"%lf\n",speed);
    /* Close */
    fclose(fp);
    fclose(fytemp);
    /*for ( i = 0; i < 0x100000000ULL; i++ ) {
        if ( 0 == i % 0x10000000ULL ) {
            TEST_PROGRESS();
        }
        if ( poptrie_lookup(poptrie, i) != poptrie_rib_lookup(poptrie, i) ) {
            return -1;
        }
    }*/


    /* Release */
    poptrie_release(poptrie);

    return 0;
}

static int
lookup_multithread(){

	return 0;
}

int inputUpdate(RouterEntry * rentry, char * c, FILE *fp){
	char ch;
	int u1, u2, u3, u4, u5, u6 = 0;
	ushort f8 = 0x00ff;
	rentry->ipnet = 0;
	if(fscanf(fp,"%c", &ch) == 1){
		*c = ch;
		if(ch == 'A'){
			fscanf(fp, "%d.%d.%d.%d/%d\t%d\n", &u1, &u2, &u3, &u4, &u5, &u6);
			printf("%c\t%d.%d.%d.%d/%d\t%d", ch, u1, u2, u3, u4, u5, u6);
			rentry->port = u6;
		}else if(ch == 'W'){
			fscanf(fp, "%d.%d.%d.%d/%d\n", &u1, &u2, &u3, &u4, &u5);
			printf("%c\t%d.%d.%d.%d/%d", ch, u1, u2, u3, u4, u5);
		}else{
			return 1;
		}
		rentry->ipnet = ((u32)u1<<24) + ((u32)u2<<16) + ((u32)u3<<8) + ((u32)u4&f8);
		rentry->length = u5;
		printf("\tipnet:%u\tlength:%d\n", rentry->ipnet, u5);
		return 1;
	}
	return 0;
}

int inputTraceMean(RouterEntry * p, int max){
	FILE * fpLookup, *fpUpdate;
	fpLookup = fopen(traceFile, "r");
	if(fpLookup == NULL){
		printf("open lookup file error\n");
		exit(0);
	}
	fpUpdate = fopen(updateFile, "r");
	if(fpUpdate == NULL){
		printf("open update file error\n");
		exit(0);
	}

	int cur = 0;
	char c;
	uint ipad;
	printf("\nmax:%d\n",max);
	while(cur < max){
		if(cur % KK == 0){
			if(!inputUpdate(p, &c, fpUpdate)) {
				printf("%u\t%u\t%u\t%u\n",p->type,p->ipnet,p->length,p->port);
				printf("\nupdate input wrong\n");
				break;
			}
			if(c == 'A'){
				p->type = 1;
				updatenum++;
			}else if(c == 'W'){
				p->type = 2;
				updatenum++;
			}
		}else{
			if(fscanf(fpLookup, "%u\n", &ipad) == -1) {
				printf("\nlookup input wrong\n");
				break;
			}
			p->type = 0;
			p->ipnet = ipad;
			lookupnum++;
		}
		p++;
		cur++;
	}
	fclose(fpLookup);
	fclose(fpUpdate);
	printf("\ncur:%d\n",cur);
	return cur;
}

int
init_poptrie(){
	FILE *fp;
	char buf[4096];
	int prefix[4];
	int prefixlen;
	int nexthop[4];
	int ret;
	u32 addr1;
	u32 addr2;
	u64 i;

	char * tempFile = "poptrie_lookup_result.txt";
	int result_pointer = 0;

	unsigned long long time_start, time_end;
	unsigned long long sum = 0;
	double average_cycle;
	uint hop = 0;
	uint count = 0;

	FILE * temp = fopen(tempFile, "w");

	fp = fopen(fibfile, "r");
	if ( NULL == fp ) {
	      return -1;
	}

	/* Initialize */
	pop = poptrie_init(NULL, 18, 22);
	if ( NULL == pop ) {
	    return -1;
	}

	/* Load the full route */
	i = 0;
	while ( !feof(fp) ) {
	   if ( !fgets(buf, sizeof(buf), fp) ) {
	        continue;
	    }
	    ret = sscanf(buf, "%d.%d.%d.%d/%d %d", &prefix[0], &prefix[1],
	                 &prefix[2], &prefix[3], &prefixlen, &addr2);
	    if ( ret < 0 ) {
	        return -1;
	    }

	    /* Convert to u32 */
	    addr1 = ((u32)prefix[0] << 24) + ((u32)prefix[1] << 16)
	        + ((u32)prefix[2] << 8) + (u32)prefix[3];
	    //addr2 = ((u32)nexthop[0] << 24) + ((u32)nexthop[1] << 16) + ((u32)nexthop[2] << 8) + (u32)nexthop[3];

	    /* Add an entry */
	    ret = poptrie_route_add(pop, addr1, prefixlen, (void *)(u64)addr2);
	    if ( ret < 0 ) {
	        return -1;
	    }
	    if ( 0 == i % 10000 ) {
	        TEST_PROGRESS();
	    }
	    i++;
	}
	return 0;
}

int updateLookup(struct poptrie * poptrie, RouterEntry * list, int max){
	RouterEntry * p = list;
	int all = 0;
	for(p = list; p < list + max; p++){
		if(p->type == 0){
			poptrie_lookup(poptrie, p->ipnet);
		}else if(p->type == 1){
			poptrie_route_update(poptrie, p->ipnet, p->length, (void *)(u64)(p->port));
		}else if(p->type == 2){
			poptrie_route_del(poptrie, p->ipnet, p->length);
		}
	}
	return all;
}

int
a(int argc, const char * const argv[])
{
	int max = 10000000;
	RouterEntry * list = (RouterEntry*)malloc(sizeof(RouterEntry)*max);
	FILE * fp;
	fp = fopen("poptrie_isc_update_lookup_mix_result.txt","at");

	int i = 0;
	if(argc == 2){
		i = atoi(argv[1]);
	}
	memset(list,0,sizeof(RouterEntry)*max);
	KK = KKlist[i];
	max = inputTraceMean(list,max);

	struct poptrie * poptrie = NULL;
	init_poptrie();
	poptrie = pop;
	struct timeval start_time, end_time;
	double total;
	double lookup_speed, update_speed;
	gettimeofday(&start_time, NULL);
	updateLookup(poptrie, list, max);
	gettimeofday(&end_time, NULL);

	total = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000.0;
	lookup_speed = lookupnum * 1.0 / (total / 1000000.0) / 1000000;
	update_speed = updatenum * 1.0 / (total / 1000000.0) / 1000000;
	fprintf(fp, "%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\n", lookupnum, updatenum, max, KK, total, lookup_speed, update_speed);
	printf("\n%d\t%d\t%d\t%d\t%lf\tlookupspeed:%lf\tupdatespeed:%lf\n", lookupnum, updatenum, max, KK, total, lookup_speed, update_speed);

	free(list);
	fclose(fp);
	poptrie_release(poptrie);
	return 0;
}

/*
 * Main routine for the basic test
 */
int
main(int argc, const char *const argv[])
{
    int ret;

    ret = 0;
    fytemp = fopen("poptrie_org_lookup_result.txt","at");
    /* Run tests */
    //TEST_FUNC("init", test_init, ret);
    //TEST_FUNC("lookup", test_lookup, ret);
    //TEST_FUNC("lookup2", test_lookup2, ret);
    //TEST_FUNC("lookup_fullroute", test_lookup_linx, ret);
    //TEST_FUNC("lookup_fullroute_update", test_lookup_linx_update, ret);
    test_init();
    test_lookup();
    test_lookup_linx();
    //test_lookup_linx_update();

    return ret;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
