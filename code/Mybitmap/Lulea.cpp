/*
 ============================================================================
 Name        : Lulea.c
 Author      : Mzaort
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#ifdef WIN32
#include <windows.h>
#else
#include <sys/unistd.h>
#include <sys/time.h>
#endif

int nm = 0;
#include "lulea.h"

static LookupTable ltb;
static TrieNode ports16[1<<LAYER1];

static ushort ltbcnt = 0;
static int ltb_update[MAXCHUNKNUM];
static int ltb_layer[MAXCHUNKNUM];

static PortMap map;
static ushort mapcur = 1;

int freq = 32;
int freqshift = 2;

unsigned long long diff_value;

UpdateNode head;

/*for statistic*/
int pointer_number = 0;
int element_number = 0;
unsigned long long bytes = 0;
int group_size = 0;

int now_one_number = 0;

static ushort bits8[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3,
		2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4,
		3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3,
		2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5,
		4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5,
		4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4,
		3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4,
		3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6,
		5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5,
		4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6,
		5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8 };

static struct {
	TrieNode * v16;
	TrieNode ** v8ptr;
} trie;

#ifdef WIN32
inline unsigned long long GetCycleCount()
{
	__asm("RDTSC");
}
#endif


void freeMemory() {
	TrieNode * ptr;
	TrieNode * ptr2;
	TrieNode * arr;
	TrieNode * v16 = trie.v16;
	TrieNode ** v8ptr = trie.v8ptr;
	int i, j;

	
	for (ptr = v16 + (1 << LAYER1); ptr < v16 + (1 << (LAYER1+1)); ptr++)
	{
		if (ptr->iptr != 0 && ptr->t != PORTTYPE)
		{
			arr = v8ptr[ptr->iptr];
			for (ptr2 = arr + (1 << LAYER2); ptr2 < arr + (1 << (LAYER2 + 1)); ptr2++)
			{
				if (ptr2->iptr != 0 && ptr2->t != PORTTYPE)
				{
					free(v8ptr[ptr2->iptr]);
					v8ptr[ptr2->iptr] = 0;
				}
			}
			free(arr);
			arr = 0;
		}
	}
	free(v16);
	v16 = 0;
	free(v8ptr);
	v8ptr = 0;

	
	ushort ** base;
	Sparse * spe;
	Chunk *chk;
	for (i = 1; i < ltbcnt; i++) 
	{
		spe = ltb[i].spe;
		if (!spe)
		{
			continue;
		}
		else if (spe->type == SPRTYPE) 
		{
			free(spe);
			spe = 0;
		} 
		else 
		{
			chk = ltb[i].chk;
			int gw = chk->groupWidth;
			base = chk->base;
			for (j = 0; j < (1 << gw); j++) 
			{
				free(base[j]);
				base[j] = 0;
			}
			free(base);
			base = 0;
			free(chk);
			chk = 0;
		}
	}
}

void overhead()
{
	int storageAll = 0;
	int ltbSize = sizeof(LookupEntry) * ltbcnt;
	int mapSize = sizeof(uint) * mapcur; //sizeof(map);
	int bit8Size = sizeof(bits8);
	int portsSize = sizeof(ports16);
	int i, j;
	ushort ** base;
	Sparse * spe;

	for (i = 0; i < ltbcnt; i++) 
	{
		spe = ltb[i].spe;
		if (!spe)
		{
			continue;
		}
		else if (spe->type == SPRTYPE) 
		{
			ltbSize += sizeof(Sparse);
		} 
		else 
		{
			ltbSize += sizeof(Chunk);
			Chunk* chkptr = ltb[i].chk;
			base = chkptr->base;
			ltbSize += sizeof(ushort*) * (1 << chkptr->groupWidth);
//			for (j = 0; j < (1 << chkptr->groupWidth); j++)
//			{
//				ltbSize += sizeof(base[j]);//_msize(base[j]);//fengyong linux modify
//			}
		}
	}
	ltbSize += group_size;
	storageAll = ltbSize + mapSize + bit8Size + portsSize;
	printf("all overhead is %d\nltbsize is %d\nmapsize is %d\nbit8size is %d\nportssize is %d\n", storageAll, ltbSize, mapSize, bit8Size, portsSize);
	printf("The whole number of 1: %d\n", now_one_number);
	fprintf(resultFile, "[Size]\nthe size of whole bitmap:\t%lf MB, number of 1:\t%d\n", storageAll*1.0 / 1000000, now_one_number);
	return;
}

//ushort triveMap(ushort mask, ushort lim) {
//	ushort n = mask >> ((1 << CLUSTER) - lim - 1);
//	ushort cnt = 0;
//	while (n) {
//		cnt += n & 0x1;
//		n >>= 1;
//	}
//	return cnt;
//}

void makeltblow3(TrieNode ** v8ptr, ushort current)
{
	int level;
	int i, j;
	int start, end;
	ushort original = 0xff;
	ushort flag = 1;

	TrieNode * ptr;

	TrieNode * v8 = v8ptr[current];
	ushort * bitmask = (ushort *)malloc(sizeof(ushort) * (1 << 8));
	TrieNode * ports = (TrieNode *)malloc(sizeof(TrieNode) * (1 << 8));
	memset(bitmask, 0, sizeof(bitmask));

	Sparse * spa = (Sparse *)malloc(sizeof(Sparse));
	memset(spa, 0, sizeof(Sparse));


	if (v8[1].t == PORTTYPE)
	{
		ptr = &v8[1];
	}
	else
	{
		ptr = &v8[0];
	}
	spa->type = SPRTYPE;
	spa->portfix = ptr->p;
	int spacur = SPARSE - 1;

	for (i = 0; i < (1 << 8); i++)
		ports[i] = *ptr;

	for (level = 1; level < 8; level++)
	{
		i = (1 << level);
		j = i << 1;
		ushort mask = (original >> (8 - level)) << (8 - level);
		for (; i < j; i++)
		{
			if (v8[i].t == PORTTYPE)
			{
				if (flag)
				{
					if (spacur >= 0)
					{
						ushort bits = (i - (1 << level)) << (8 - level);
						spa->special[spacur].bits = bits;
						spa->special[spacur].mask = mask;
						spa->lookup[spacur] = v8[i];
						spacur--;
					}
					else
					{
						flag = 0;
						free(spa);
					}
				}
				start = (i << (8 - level)) - (1 << 8);
				end = ((i + 1) << (8 - level)) - (1 << 8);
				for (; start < end; start++)
				{
					ports[start] = v8[i];
				}
			}
		}
	}

	for (i = 0, ptr = v8 + (1 << 8); i < (1 << 8); i++, ptr++)
	{
		if (ptr->iptr == 0)
		{
		}
		else if (ptr->t == PORTTYPE)
		{
			ports[i] = *ptr;
			if (flag)
			{
				if (spacur >= 0)
				{
					spa->special[spacur].bits = (ushort)i;
					spa->special[spacur].mask = original;
					spa->lookup[spacur] = *ptr;
					spacur--;
				}
				else
				{
					flag = 0;
					free(spa);
				}
			}
		}
		else
		{
			v8ptr[ptr->iptr][0] = ports[i];
			ports[i] = *ptr;
			if (flag)
			{
				if (spacur >= 0)
				{
					spa->special[spacur].bits = (ushort)i;
					spa->special[spacur].mask = original;
					spa->lookup[spacur] = *ptr;
					spacur--;
				}
				else
				{
					flag = 0;
					free(spa);
				}
			}
		}
	}

	TrieNode previous;
	previous.iptr = 0;

	for (i = 0; i < (1 << 8); i++)
	{
		if (previous.iptr == ports[i].iptr)
		{
			bitmask[i] = 0;
		}
		else
		{
			bitmask[i] = 1;
			previous.iptr = ports[i].iptr;
			if (previous.t != PORTTYPE)
			{
				makeltblow3(v8ptr, previous.iptr);
			}
		}
	}

	if (flag)
	{
		if (spacur > -1)
		{
			int spades = 0;
			int spasrc = spacur + 1;
			for (; spasrc < SPARSE; spasrc++, spades++)
			{
				spa->special[spades] = spa->special[spasrc];
				spa->lookup[spades] = spa->lookup[spasrc];
			}
			for (; spades < SPARSE; spades++)
			{
				spa->special[spades].mask = 0;
			}
		}
		ltb[current].spe = spa;
	}
	else
	{
		int group, cluster, bit;
		Chunk *chkptr = (Chunk *)malloc(sizeof(Chunk));
		chkptr->groupWidth = GROUP;
		chkptr->type = BITTYPE;
		//chk->mask
		ushort **base = chkptr->base = (ushort **)malloc(sizeof(ushort *) * (1 << (chkptr->groupWidth)));
		for (i = 0, group = 0; group < (1 << GROUP); i += (1 << (CLUSTER + BIT)), group++)
		{
			bitmask[i] = 1;
			int cnt = 0;
			for (start = i; start < i + (1 << (CLUSTER + BIT)); start++)
				if (bitmask[start] == 1)
					cnt++;
			base[group] = (ushort *)malloc(sizeof(ushort) * ((1 << CLUSTER) + cnt));

			group_size += sizeof(ushort) * ((1 << CLUSTER) + cnt); //added by zcw
			now_one_number += cnt;

			CodeWord *cwd = (CodeWord *)(base[group]);
			TrieNode *lookup = (TrieNode *)(base[group] + (1 << CLUSTER));
			int num8;
			int preOne = 0;
			int curOne = 0;
			for (cluster = 0; cluster < (1 << CLUSTER); cluster++)
			{
				num8 = 0;
				curOne = 0;
				for (bit = i + (cluster << BIT); bit < i + ((cluster + 1) << BIT); bit++)
				{
					num8 <<= 1;
					if (bitmask[bit] == 1)
					{
						num8++;
						curOne++;
						*lookup = ports[bit];
						lookup++;
					}
				}
				cwd->before = preOne;
				cwd->bitmask = num8;
				cwd++;
				preOne += curOne;
			}
		}
		ltb[current].chk = chkptr;
	}
	free(bitmask);
	free(ports);
}

void makeltblow2(TrieNode ** v8ptr, ushort current) 
{
	int level;
	int i, j;
	int start, end;
	ushort original = 0x3f;
	ushort flag = 1;

	TrieNode * ptr;

	TrieNode * v8 = v8ptr[current];
	ushort * bitmask = (ushort *) malloc(sizeof(ushort) * (1 << LAYER2));
	TrieNode * ports = (TrieNode *) malloc(sizeof(TrieNode) * (1 << LAYER2));
	memset(bitmask, 0, sizeof(bitmask));

	Sparse * spa = (Sparse *) malloc(sizeof(Sparse));
	memset(spa, 0, sizeof(Sparse)); 


	if (v8[1].t == PORTTYPE) 
	{
		ptr = &v8[1];
	} 
	else 
	{
		ptr = &v8[0];
	}
	spa->type = SPRTYPE;
	spa->portfix = ptr->p;
	int spacur = SPARSE - 1;

	for (i = 0; i < (1 << LAYER2); i++)
		ports[i] = *ptr;

	for (level = 1; level < LAYER2; level++) 
	{
		i = (1 << level);
		j = i << 1;
		ushort mask = (original >> (LAYER2 - level)) << (LAYER2 - level);
		for (; i < j; i++) 
		{
			if (v8[i].t == PORTTYPE) 
			{
				if (flag) 
				{
					if (spacur >= 0) 
					{
						ushort bits = (i - (1 << level)) << (LAYER2 - level);
						spa->special[spacur].bits = bits;
						spa->special[spacur].mask = mask;
						spa->lookup[spacur] = v8[i];
						spacur--;
					}
					else 
					{
						flag = 0;
						free(spa);
					}
				}
				start = (i << (LAYER2 - level)) - (1 << LAYER2);
				end = ((i + 1) << (LAYER2 - level)) - (1 << LAYER2);
				for (; start < end; start++) 
				{
					ports[start] = v8[i];
				}
			}
		}
	}

	for (i = 0, ptr = v8 + (1 << LAYER2); i < (1 << LAYER2); i++, ptr++)
	{
		if (ptr->iptr == 0) 
		{
		} 
		else if (ptr->t == PORTTYPE) 
		{
			ports[i] = *ptr;
			if (flag) 
			{
				if (spacur >= 0) 
				{
					spa->special[spacur].bits = (ushort) i;
					spa->special[spacur].mask = original;
					spa->lookup[spacur] = *ptr;
					spacur--;
				} 
				else 
				{
					flag = 0;
					free(spa);
				}
			}
		} 
		else 
		{
			v8ptr[ptr->iptr][0] = ports[i];
			ports[i] = *ptr;
			if (flag) 
			{
				if (spacur >= 0) 
				{
					spa->special[spacur].bits = (ushort) i;
					spa->special[spacur].mask = original;
					spa->lookup[spacur] = *ptr;
					spacur--;
				} 
				else 
				{
					flag = 0;
					free(spa);
				}
			}
		}
	}

	TrieNode previous;
	previous.iptr = 0;

	for (i = 0; i < (1 << LAYER2); i++)
	{
		if (previous.iptr == ports[i].iptr) 
		{
			bitmask[i] = 0;
		} 
		else 
		{
			bitmask[i] = 1;
			previous.iptr = ports[i].iptr;
			if (previous.t != PORTTYPE) 
			{
				makeltblow3(v8ptr, previous.iptr);
			}
		}
	}

	if (flag) 
	{
		if (spacur > -1) 
		{
			int spades = 0;
			int spasrc = spacur + 1;
			for (; spasrc < SPARSE; spasrc++, spades++) 
			{
				spa->special[spades] = spa->special[spasrc];
				spa->lookup[spades] = spa->lookup[spasrc];
			}
			for (; spades < SPARSE; spades++) 
			{
				spa->special[spades].mask = 0;
			}
		}
		ltb[current].spe = spa;
	}
	else
	{
		int group, cluster, bit;
		Chunk *chkptr = (Chunk *)malloc(sizeof(Chunk));
		chkptr->groupWidth = GROUP2;
		chkptr->type = BITTYPE;
		//chk->mask
		ushort **base = chkptr->base = (ushort **)malloc(sizeof(ushort *) * (1 << (chkptr->groupWidth)));
		for (i = 0, group = 0; group < (1 << GROUP2); i += (1 << (CLUSTER2 + BIT)), group++)
		{
			bitmask[i] = 1;
			int cnt = 0;
			for (start = i; start < i + (1 << (CLUSTER2 + BIT)); start++)
				if (bitmask[start] == 1)
					cnt++;
			base[group] = (ushort *)malloc(sizeof(ushort) * ((1 << CLUSTER2) + cnt));

			group_size += sizeof(ushort) * ((1 << CLUSTER2) + cnt); //added by zcw
			now_one_number += cnt;

			CodeWord *cwd = (CodeWord *)(base[group]);
			TrieNode *lookup = (TrieNode *)(base[group] + (1 << CLUSTER2));
			int num8;
			int preOne = 0;
			int curOne = 0;
			for (cluster = 0; cluster < (1 << CLUSTER2); cluster++)
			{
				num8 = 0;
				curOne = 0;
				for (bit = i + (cluster << BIT); bit < i + ((cluster + 1) << BIT); bit++)
				{
					num8 <<= 1;
					if (bitmask[bit] == 1)
					{
						num8++;
						curOne++;
						*lookup = ports[bit];
						lookup++;
					}
				}
				cwd->before = preOne;
				cwd->bitmask = num8;
				cwd++;
				preOne += curOne;
			}
		}
		ltb[current].chk = chkptr;
	}
	free(bitmask);
	free(ports);
}

void makeltb(TrieNode ** v8ptr) {
	int level;
	int i, j;
	int start, end;
	TrieNode * ptr;
	TrieNode * v16 = v8ptr[0];


	if (v16[1].t == PORTTYPE)
	{
		ptr = &v16[1];
	}
	else
	{
		ptr = &v16[0];
	}

	for (i = 0; i < (1 << LAYER1); i++)
		ports16[i] = *ptr;

	for (level = 1; level < LAYER1; level++)
	{
		i = (1 << level);
		j = i << 1;
		for (; i < j; i++)
		{
			if (v16[i].t == PORTTYPE)
			{
				start = (i << (LAYER1 - level)) - (1 << LAYER1);
				end = ((i + 1) << (LAYER1 - level)) - (1 << LAYER1);
				for (; start < end; start++)
				{
					ports16[start] = v16[i];
				}
			}
		}
	}
	for (i = 0, ptr = v16 + (1 << LAYER1); i < (1 << LAYER1); i++, ptr++)
	{
		if (ptr->iptr == 0)
		{
		}
		else if (ptr->t == PORTTYPE)
		{
			ports16[i] = *ptr;
		}
		else
		{
			v8ptr[ptr->iptr][0] = ports16[i];
			ports16[i] = *ptr;
			makeltblow2(v8ptr, ptr->iptr);
		}
	}
}

TrieNode findDefault(TrieNode *v8, uint temp) {
	while (v8[temp].t != PORTTYPE)
		temp >>= 1;
	return v8[temp];
}

void addEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v8ptr) {
	//printf("%X/%d\t%d\n", rentry.ipnet, rentry.length, rentry.port);
	uint temp, temp2, temp3;
	TrieNode *ptr, *ptr2, *ptr3;

	if (rentry.length < LAYER1)
	{
		temp = (1 << rentry.length) + (rentry.ipnet >> (32 - rentry.length));
		v16[temp].t = PORTTYPE;
		v16[temp].p = rentry.port;
	}
	else if (rentry.length == LAYER1)
	{
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			ptr->t = PORTTYPE;
			ptr->p = rentry.port;
		} 
		else if (ptr->t == PORTTYPE) 
		{
			ptr->p = rentry.port;
		} 
		else 
		{
			v8ptr[ptr->iptr][1].t = PORTTYPE;
			v8ptr[ptr->iptr][1].p = rentry.port;
		}
	} 
	else if (rentry.length < 24) 
	{
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << (rentry.length - LAYER1)) + ((rentry.ipnet << LAYER1) >> (32 - rentry.length + LAYER1));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;
		} 
		else if (ptr->t == PORTTYPE) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			v8ptr[ltbcnt][1].t = PORTTYPE;
			v8ptr[ltbcnt][1].p = ptr->p;
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;
		}
	} 
	else if (rentry.length == 24) 
	{
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << LAYER2) + ((rentry.ipnet << LAYER1) >> (32 - LAYER2));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;
		} 
		else if (ptr->t == PORTTYPE) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			v8ptr[ltbcnt][1].t = PORTTYPE;
			v8ptr[ltbcnt][1].p = ptr->p;
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->iptr == 0) 
			{
				ptr2->t = PORTTYPE;
				ptr2->p = rentry.port;
			} 
			else if (ptr2->t == PORTTYPE) 
			{
				ptr2->p = rentry.port;
			} 
			else 
			{
				v8ptr[ptr2->iptr][1].t = PORTTYPE;
				v8ptr[ptr2->iptr][1].p = rentry.port;
			}
		}
	} 
	else 
	{
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << LAYER2) + ((rentry.ipnet << LAYER1) >> (32 - LAYER2));
		//temp = (1 << 16) + ((rentry.ipnet) >> 16);
		//temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp3 = (1 << (rentry.length - 24)) + ((rentry.ipnet & 0xff) >> (32
				- rentry.length));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			ptr->iptr = ltbcnt;
			ltbcnt++;

			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->iptr = ltbcnt;
			ltbcnt++;

			ptr3 = v8ptr[ptr2->iptr] + temp3;
			ptr3->t = PORTTYPE;
			ptr3->p = rentry.port;

		} 
		else if (ptr->t == PORTTYPE) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			v8ptr[ltbcnt][1].t = PORTTYPE;
			v8ptr[ltbcnt][1].p = ptr->p;
			ptr->iptr = ltbcnt;
			ltbcnt++;

			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->iptr = ltbcnt;
			ltbcnt++;

			ptr3 = v8ptr[ptr2->iptr] + temp3;
			ptr3->t = PORTTYPE;
			ptr3->p = rentry.port;

		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->iptr == 0) 
			{
				v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
				memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
				ptr2->iptr = ltbcnt;
				ltbcnt++;
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				ptr3->t = PORTTYPE;
				ptr3->p = rentry.port;

			} 
			else if (ptr2->t == PORTTYPE) 
			{
				v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
				memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
				v8ptr[ltbcnt][1].t = PORTTYPE;
				v8ptr[ltbcnt][1].p = ptr2->p;
				ptr2->iptr = ltbcnt;
				ltbcnt++;
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				ptr3->t = PORTTYPE;
				ptr3->p = rentry.port;
			} 
			else 
			{
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				ptr3->t = PORTTYPE;
				ptr3->p = rentry.port;
			}
		}
	}
}

ushort findMap(uint port) {
	/*if(port == 10)
	{
		printf("error port\n");
	}*/
	int i = 1;
	for (; i < mapcur && map[i] != port; i++)
		;
	if (i < mapcur && map[i] == port)
		return i;
	if (mapcur == MAXPORT) {
		for(i = 1; i <= MAXPORT; i++)
			printf("port %d: %d\n", i, map[i]);
		printf("sorry not support\n");
		getchar();
		exit(0);
	}
	map[mapcur] = port;
	mapcur++;
	return mapcur - 1;
}

int input(RouterEntry * rentry, FILE * fp) {
	uint u1, u2, u3, u4, u5, u6;
	ushort f8 = 0x00ff;

	rentry->ipnet = 0;
	if ((fscanf(fp, "%d.%d.%d.%d/%d\t%d", &u1, &u2, &u3, &u4, &u5, &u6)) == 6) {
		rentry->ipnet = ((u1 & f8) << 24) + ((u2 & f8) << 16)
				+ ((u3 & f8) << 8) + ((u4 & f8));
		rentry->length = u5 & f8;
		rentry->port = findMap(u6);
		return 1;
	}
	return 0;
}

int inputUpdate(RouterEntry * rentry, char * c, FILE *fp) {
	uint t1, t2;
	char ch;
	uint u1, u2, u3, u4, u5, u6 = 0;
	ushort f8 = 0x00ff;
	rentry->ipnet = 0;
	if (fscanf(fp, "%d\t%d\t%c", &t1, &t2, &ch) == 3) {
		*c = ch;
		if (ch == 'A') {
			fscanf(fp, "%d.%d.%d.%d/%d\t%d", &u1, &u2, &u3, &u4, &u5, &u6);
			rentry->port = findMap(u6);
		} else if (ch == 'W') {
			fscanf(fp, "%d.%d.%d.%d/%d", &u1, &u2, &u3, &u4, &u5);
		}
		rentry->ipnet = ((u1 & f8) << 24) + ((u2 & f8) << 16)
				+ ((u3 & f8) << 8) + ((u4 & f8));
		rentry->length = u5 & f8;
		return 1;
	}
	return 0;
}

int inputUpdate2(RouterEntry * rentry, char * c, FILE *fp) {
//	uint t1, t2;
	char ch;
	uint u1, u2, u3, u4, u5, u6 = 0;
	ushort f8 = 0x00ff;
	rentry->ipnet = 0;
	if (fscanf(fp, "%c", &ch) == 1) {
		*c = ch;
		if (ch == 'A') {
			fscanf(fp, "%d.%d.%d.%d/%d\t%d\n", &u1, &u2, &u3, &u4, &u5, &u6);
			rentry->port = findMap(u6);
		} else if (ch == 'W') {
			fscanf(fp, "%d.%d.%d.%d/%d\n", &u1, &u2, &u3, &u4, &u5);
		}else{
			return 1;
		}
		rentry->ipnet = ((u1 & f8) << 24) + ((u2 & f8) << 16)
				+ ((u3 & f8) << 8) + ((u4 & f8));
		rentry->length = u5 & f8;
		return 1;
	}
	return 0;
}

void construct(TrieNode * v16, TrieNode ** v8ptr, char * file) {
	RouterEntry rentry;// ipnet  port  length  type
	FILE *fp;
	fp = fopen(file, "r");//����rib�ļ�
	if (fp == NULL) 
	{
		perror("Open file...");
		//printf("err no: %d\n", errno);
		exit(1);
	}
	memset(v16, 0, sizeof(TrieNode)*(1 << (LAYER1)));//�����ָ��2^17������   ��Щ��chunk
	memset(v8ptr, 0, sizeof(TrieNode*) * MAXCHUNKNUM);//
	v16[0].t = PORTTYPE;
	v16[0].p = DEFAULT;
	ltbcnt = 0;
	v8ptr[ltbcnt] = v16;
	ltbcnt++;

	while (input(&rentry, fp)) 
	{
		addEntry(rentry, v16, v8ptr);
	}
	fclose(fp);
}


void start(char * file) {
	TrieNode * v16 = trie.v16 = (TrieNode *) malloc(sizeof(TrieNode) * (1 << (LAYER1 + 1))); //level 16
	TrieNode ** v8ptr = trie.v8ptr = (TrieNode **) malloc(sizeof(TrieNode *)* MAXCHUNKNUM);
	printf("%d\t%d\t%d\t%d\t%d\n", sizeof(TrieNode), sizeof(ushort),
			sizeof(CodeWord), sizeof(Special), sizeof(RouterEntry));
	//generate();
	construct(v16, v8ptr, file);
	printf("Trie has been created....%u\n", mapcur);

	makeltb(v8ptr);
	printf("ltb has been created : %d....\n", ltbcnt);
}

uint lookup(uint ip) {
	ushort cur = 0;
	ushort group, cluster, bit;
	ushort * base;
	TrieNode *ptr, *node;
	CodeWord *cwd;
	Sparse * spe;
	Chunk * chk;
	int i;
	uint speip;


	/*layer 1*/	
	node = ports16 + (ip >> (32 - LAYER1));
	if (node->t == PORTTYPE)
		return map[node->p];
	cur = node->iptr;

	/*layer 2*/
	spe = ltb[cur].spe;
	if (spe->type == SPRTYPE) 
	{
		speip = (ip << LAYER1) >> (32 - LAYER2);  //2018.1.25
		Special *sptr = spe->special;
		for (i = 0; i < SPARSE; i++, sptr++) 
		{
			if (sptr->mask == 0u) 
			{
				return map[spe->portfix];
			} 
			else if ((speip & sptr->mask) == sptr->bits) 
			{
				if (spe->lookup[i].t == PORTTYPE)
					return map[spe->lookup[i].p];
				cur = spe->lookup[i].iptr;
				goto next;
			}
		}
		return map[spe->portfix];
	} 
	else 
	{
		chk = ltb[cur].chk;
		ushort GW = chk->groupWidth;
		ushort CW = LAYER2 - BIT - GW;
		uint mip = (ip << LAYER1) >> (32 - LAYER2); //2018.1.25
		group = mip >> (CW + BIT);
		cluster = (mip << (32 - CW - BIT)) >> (32 - CW);
		bit = mip & 0x7;
		base = chk->base[group];
		cwd = (CodeWord *) base;
		ptr = (TrieNode *) (base + (1 << CW));
		node = ptr + cwd[cluster].before + bits8[cwd[cluster].bitmask >> (OFFSET
				- bit)] - 1;
		//node = ptr + cwd[cluster].before + popcnt(cwd[cluster].bitmask >> (OFFSET
		//				- bit)) - 1;
		if (node->t == PORTTYPE)
			return map[node->p];
		cur = node->iptr;
	}
	/*layer 2*/
	next: spe = ltb[cur].spe;
	if (spe->type == SPRTYPE) 
	{
		speip = (ip << 24) >> 24;
		Special *sptr = spe->special;
		for (i = 0; i < SPARSE; i++, sptr++) 
		{
			if (sptr->mask == 0u) 
			{
				return map[spe->portfix];
			} 
			else if ((speip & sptr->mask) == sptr->bits) 
			{
				return map[spe->lookup[i].p];
			}
		}
		return map[spe->portfix];
	} 
	else 
	{
		chk = ltb[cur].chk;
		ushort GW = chk->groupWidth;
		ushort CW = 8 - BIT - GW;
		uint eip = (ip << 24) >> 24;
		group = eip >> (CW + BIT);
		cluster = (eip << (32 - CW - BIT)) >> (32 - CW);
		bit = eip & 0x7;
		base = chk->base[group];
		cwd = (CodeWord *) base;
		ptr = (TrieNode *) (base + (1 << CW));
		node = ptr + cwd[cluster].before + bits8[cwd[cluster].bitmask >> (OFFSET - bit)] - 1;
		//node = ptr + cwd[cluster].before + popcnt(cwd[cluster].bitmask >> (OFFSET	- bit)) - 1;
		return map[node->p];
	}
}

void update(char * file) {
	RouterEntry rentry;
	FILE * fp, *fpout = resultFile;
	FILE *temp = fopen("mybitmap_eqix_lookup_update_result.txt","at");
	struct timeval time_start, time_end;
	double sum = 0;
	double speed = 0;
	char c;
	int j;

	fp = fopen(file, "r");
	if (fp == NULL) {
		perror("Open file");
		exit(1);
	}


	TrieNode ** v8ptr = trie.v8ptr;
	TrieNode * v16 = trie.v16;
	initHead(&head);
	j = 0;
	//int cmax = 0;
	//int cmin = 1 << 30;


	RouterEntry * list = (RouterEntry*)malloc(sizeof(RouterEntry)*UPDATEMAX);
	for(int i = 0; i < UPDATEMAX; i++){
		inputUpdate2(&rentry, &c, fp);
		list[i] = rentry;
		list[i].c = c;
	}

	int i = 0;
	int k = 0;
	gettimeofday(&time_start, NULL);
	for(i = 0; i < UPDATEMAX; i++){
		//printf("%c\n",list[i].c);
		if(list[i].c=='A'){
			//k++;
			announceEntry(list[i], v16, v8ptr);
			disposeUpdate(v16, v8ptr);
		}else if(list[i].c=='W'){
			//k++;
			withdrawEntry(list[i], v16, v8ptr);
			disposeUpdate(v16, v8ptr);
		}
	}
	gettimeofday(&time_end, NULL);
	sum = (time_end.tv_usec - time_start.tv_usec)*1.0 + (time_end.tv_sec - time_start.tv_sec) * 1000000.0;
	speed = i * 1.0 / (sum * 1.0 / 1000000) / 1000000;

/*
	while (inputUpdate2(&rentry, &c, fp) && j++ < UPDATEMAX) {
		if (c == 'A') {
			gettimeofday(&time_start, NULL);//time_start = GetCycleCount();
			announceEntry(rentry, v16, v8ptr);
			disposeUpdate(v16, v8ptr);
			gettimeofday(&time_end, NULL);
			double res = (time_end.tv_usec - time_start.tv_usec) + (time_end.tv_sec - time_start.tv_sec) * 1000000.0;
			sum += res;
		} else if (c == 'W') {
			gettimeofday(&time_start, NULL);
			withdrawEntry(rentry, v16, v8ptr);
			disposeUpdate(v16, v8ptr);
			gettimeofday(&time_end, NULL);
			double res = (time_end.tv_usec - time_start.tv_usec) + (time_end.tv_sec - time_start.tv_sec) * 1000000.0;
			sum += res;
			//fprintf(fpout, "%d\n", res);
		}
		if (j % 1000000 == 0)
			printf("%d\r", j);
	}
	speed = j * 1.0 / (sum * 1.0 / 1000000) / 1000000;
*/


	FILE * myfp = fopen("obma.update.txt","at");
	fprintf(myfp,"zi+te\t%lf\n",speed);
	fclose(myfp);
	printf("pointer_number:%d\nelemetn_number:%d\nbytes:%d\n",pointer_number, element_number, bytes);
	printf("average change pointer number: %lf\naverage change element number:%lf\naverage extra storage:%lf\n", 1.0 * pointer_number / i, 1.0 * element_number / i, 1.0 * bytes / i);

	printf("update speed: %lf\tj:%d\n", speed, j);
	fprintf(temp,"%lf\n",speed);
	//for chunk file/******/
//	FILE * ch_file = fopen("eqix_chunk.txt","w");
//	for (int i = 1; i < ltbcnt; i++)
//		fprintf(ch_file, "%d\t/%d\t%d\n", i, ltb_layer[i], ltb_update[i]);
//	fclose(ch_file);

	fclose(temp);
	fclose(fp);
}

/*******************************************/
UpdateNode *  makeNode(ushort iptr, ushort type, ushort level, uint index, ushort layer) {
	UpdateNode * node = (UpdateNode *)malloc(sizeof(UpdateNode));
	node->layer = layer;
	node->index = index;
	node->iptr = iptr;
	node->level = level;
	node->next = 0;
	node->type = type;
	return node;
}

void initHead(UpdateNode * head) {
	head->iptr = 0;
	head->index = 0;
	head->level = 0;
	head->next = 0;
}
void insertNode(UpdateNode * head, UpdateNode * e) {
	UpdateNode *p;
	p = head;
	while (p->next && p->next->iptr < e->iptr)
		p = p->next;
	if (!p->next) {
		p->next = e;
	} else {		
		if(e->level == ZLEVEL || e->level == 0){
			e->next = p->next;
			p->next = e;
		}else{
		while(p->next && p->next->iptr == e->iptr && p->next->level >= e->level)
			p = p->next;
		e->next = p->next;
		p->next = e;
		}
	}
}

int cover(uint index1, uint index2){
	while(index2 > index1) index2 >>= 1;
	if(index2 == index1) return 1;
	return 0;
}

void removeCover(UpdateNode * head){

}

void withdrawEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v8ptr) {
	uint temp, temp2, temp3;
	int start, end;
	int i, j;
	TrieNode *ptr, *ptr2, *ptr3;
	if (rentry.length < LAYER1)
	{
		temp = (1 << rentry.length) + (rentry.ipnet >> (32 - rentry.length));
		ptr = v16 + temp;
		if (ptr->t == PORTTYPE)
		{
			ptr->iptr = 0;
			insertNode(&head, makeNode(0, DELETE, rentry.length, temp, 1));
		}
	} 
	else if (rentry.length == LAYER1) 
	{
		temp = (1 << LAYER1) + (rentry.ipnet >> (32 - LAYER1));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
		} 
		else if (ptr->t == PORTTYPE) 
		{
			ptr->iptr = 0;
			insertNode(&head, makeNode(0, DELETE, rentry.length, temp, 1));
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + 1;
			if (ptr2->t == PORTTYPE) 
			{
				ptr2->iptr = 0;
				insertNode(&head, makeNode(ptr->iptr, DELETE, 0, 1, 2));
			}
		}
	} 
	else if (rentry.length < 24) 
	{
		//temp = (1 << 16) + (rentry.ipnet >> 16);
		//temp2 = (1 << (rentry.length - 16)) + ((rentry.ipnet & 0xffff) >> (32 - rentry.length));
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << (rentry.length - LAYER1)) + ((rentry.ipnet << LAYER1) >> (32 - rentry.length + LAYER1));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
		} 
		else if (ptr->t == PORTTYPE) 
		{
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->t == PORTTYPE) 
			{
				ptr2->iptr = 0;
				insertNode(&head, makeNode(ptr->iptr, DELETE, rentry.length - LAYER1, temp2, 2));
			}
		}
	} 
	else if (rentry.length == 24) 
	{
		//temp = (1 << 16) + (rentry.ipnet >> 16);
		//temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << LAYER2) + ((rentry.ipnet << LAYER1) >> (32 - LAYER2));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
		} 
		else if (ptr->t == PORTTYPE) 
		{
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->iptr == 0) {
				;
			} else if (ptr2->t == PORTTYPE) {
				//if (ptr2->t == PORTTYPE) {  //seems extra
					ptr2->iptr = 0;
					insertNode(&head, makeNode(ptr->iptr, DELETE, LAYER2, temp2, 2));
				//}
			} else {
				ptr3 = v8ptr[ptr2->iptr] + 1;
				if (ptr3->t == PORTTYPE) {
					ptr3->iptr = 0;
					insertNode(&head, makeNode(ptr2->iptr, DELETE, 0, 1, 3));
				}
			}
		}
	} 
	else 
	{
		//temp = (1 << 16) + (rentry.ipnet >> 16);
		//temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << LAYER2) + ((rentry.ipnet << LAYER1) >> (32 - LAYER2));
		temp3 = (1 << (rentry.length - 24)) + ((rentry.ipnet & 0xff) >> (32
			- rentry.length));

		ptr = v16 + temp;

		if (ptr->iptr == 0) 
		{
		} 
		else if (ptr->t == PORTTYPE) 
		{
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->iptr == 0) 
			{
			} 
			else if (ptr2->t == PORTTYPE) 
			{
			} 
			else 
			{
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				if (ptr3->t == PORTTYPE) 
				{
					ptr3->iptr = 0;
					insertNode(&head, makeNode(ptr2->iptr, DELETE, rentry.length - 24, temp3, 3));
				}
			}
		}
	}

}

void announceEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v8ptr) {
	uint temp, temp2, temp3;
	int start, end;
	int i, j;
	TrieNode *ptr, *ptr2, *ptr3;
	if (rentry.length < LAYER1)
	{
		//update v8
		temp = (1 << rentry.length) + (rentry.ipnet >> (32 - rentry.length));
		ptr = v16 + temp;
		if (ptr->t == PORTTYPE && ptr->p == rentry.port)
		{
		}
		else
		{
			ptr->t = PORTTYPE;
			ptr->p = rentry.port;
			insertNode(&head, makeNode(0, INSERT, rentry.length, temp, 1));
		}
	}

	else if (rentry.length == LAYER1) 
	{
		temp = (1 << LAYER1) + (rentry.ipnet >> (32 - LAYER1));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			ptr->t = PORTTYPE;
			ptr->p = rentry.port;
			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
		} 
		else if (ptr->t == PORTTYPE) 
		{
			if (ptr->p == rentry.port) 
			{
			} 
			else {
				ptr->p = rentry.port;
				insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			}
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + 1;
			if (ptr2->t == PORTTYPE && ptr2->p == rentry.port) 
			{
			} 
			else 
			{
				ptr2->t = PORTTYPE;
				ptr2->p = rentry.port;
				insertNode(&head, makeNode(ptr->iptr, INSERT, 0, 1, 2));
			}
		}
	} 
	else if (rentry.length < 24) 
	{
		//temp = (1 << 16) + (rentry.ipnet >> 16);
		//temp2 = (1 << (rentry.length - 16)) + ((rentry.ipnet & 0xffff) >> (32- rentry.length));
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << (rentry.length - LAYER1)) + ((rentry.ipnet << LAYER1) >> (32 - rentry.length + LAYER1));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;

			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			insertNode(&head, makeNode(ptr->iptr, INSERT, rentry.length - LAYER1, temp2, 2));
		} 
		else if (ptr->t == PORTTYPE) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			v8ptr[ltbcnt][1].t = PORTTYPE;
			v8ptr[ltbcnt][1].p = ptr->p;
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;

			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			insertNode(&head, makeNode(ptr->iptr, INSERT, 0, 1, 2));
			insertNode(&head, makeNode(ptr->iptr, INSERT, rentry.length - LAYER1, temp2, 2));
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->t == PORTTYPE && ptr2->p == rentry.port) 
			{
			} 
			else
			{
				ptr2->t = PORTTYPE;
				ptr2->p = rentry.port;
				insertNode(&head, makeNode(ptr->iptr, INSERT, rentry.length - LAYER1, temp2, 2));
			}
		}
	} 
	else if (rentry.length == 24) 
	{
		//temp = (1 << 16) + (rentry.ipnet >> 16);
		//temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << LAYER2) + ((rentry.ipnet << LAYER1) >> (32 - LAYER2));
		ptr = v16 + temp;
		if (ptr->iptr == 0) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;

			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			insertNode(&head, makeNode(ptr->iptr, INSERT, rentry.length - LAYER1, temp2, 2));
		} 
		else if (ptr->t == PORTTYPE) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			v8ptr[ltbcnt][1].t = PORTTYPE;
			v8ptr[ltbcnt][1].p = ptr->p;
			ptr->iptr = ltbcnt;
			ltbcnt++;
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->t = PORTTYPE;
			ptr2->p = rentry.port;
			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			insertNode(&head, makeNode(ptr->iptr, INSERT, 0, 1, 2));
			insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
		} 
		else 
		{
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->iptr == 0) 
			{
				ptr2->t = PORTTYPE;
				ptr2->p = rentry.port;
				insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
			} 
			else if (ptr2->t == PORTTYPE) 
			{
				if (ptr2->p == rentry.port) 
				{
				} 
				else 
				{
					ptr2->p = rentry.port;
					insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
				}
			} 
			else 
			{
				ptr3 = v8ptr[ptr2->iptr] + 1;
				if (ptr3->t == PORTTYPE && ptr3->p == rentry.port) 
				{
				} 
				else 
				{
					ptr3->t = PORTTYPE;
					ptr3->p = rentry.port;
					insertNode(&head, makeNode(ptr2->iptr, INSERT, 0, 1, 3));
				}
			}
		}
	} 
	else 
	{
		//temp = (1 << 16) + (rentry.ipnet >> 16);
		//temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp = (1 << LAYER1) + ((rentry.ipnet) >> (32 - LAYER1));
		temp2 = (1 << LAYER2) + ((rentry.ipnet << LAYER1) >> (32 - LAYER2));
		temp3 = (1 << (rentry.length - 24)) + ((rentry.ipnet & 0xff) >> (32
			- rentry.length));
		ptr = v16 + temp;

		if (ptr->iptr == 0) 
		{
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			ptr->iptr = ltbcnt;
			ltbcnt++;

			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->iptr = ltbcnt;
			ltbcnt++;

			ptr3 = v8ptr[ptr2->iptr] + temp3;
			ptr3->t = PORTTYPE;
			ptr3->p = rentry.port;

			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
			insertNode(&head, makeNode(ptr2->iptr, INSERT, rentry.length - 24, temp3, 3));
		} 
		else if (ptr->t == PORTTYPE) {
			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk6));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk6));
			v8ptr[ltbcnt][1].t = PORTTYPE;
			v8ptr[ltbcnt][1].p = ptr->p;
			ptr->iptr = ltbcnt;
			ltbcnt++;

			v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
			memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
			ptr2 = v8ptr[ptr->iptr] + temp2;
			ptr2->iptr = ltbcnt;
			ltbcnt++;

			ptr3 = v8ptr[ptr2->iptr] + temp3;
			ptr3->t = PORTTYPE;
			ptr3->p = rentry.port;

			insertNode(&head, makeNode(0, INSERT, LAYER1, temp, 1));
			insertNode(&head, makeNode(ptr->iptr, INSERT, 0, 1, 2));
			insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
			insertNode(&head, makeNode(ptr2->iptr, INSERT, rentry.length - 24, temp3, 3));
		} 
		else {
			ptr2 = v8ptr[ptr->iptr] + temp2;
			if (ptr2->iptr == 0) 
			{
				v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
				memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
				ptr2->iptr = ltbcnt;
				ltbcnt++;
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				ptr3->t = PORTTYPE;
				ptr3->p = rentry.port;

				insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
				insertNode(&head, makeNode(ptr2->iptr, INSERT, rentry.length - 24, temp3, 3));
			} 
			else if (ptr2->t == PORTTYPE) {
				v8ptr[ltbcnt] = (TrieNode *) malloc(sizeof(Chunk8));
				memset(v8ptr[ltbcnt], 0, sizeof(Chunk8));
				v8ptr[ltbcnt][1].t = PORTTYPE;
				v8ptr[ltbcnt][1].p = ptr2->p;
				ptr2->iptr = ltbcnt;
				ltbcnt++;
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				ptr3->t = PORTTYPE;
				ptr3->p = rentry.port;

				insertNode(&head, makeNode(ptr->iptr, INSERT, LAYER2, temp2, 2));
				insertNode(&head, makeNode(ptr2->iptr, INSERT, 0, 1, 3));
				insertNode(&head, makeNode(ptr2->iptr, INSERT, rentry.length - 24, temp3, 3));
			} 
			else 
			{
				ptr3 = v8ptr[ptr2->iptr] + temp3;
				if (ptr3->t == PORTTYPE && ptr3->p == rentry.port) 
				{
				} 
				else {
					ptr3->t = PORTTYPE;
					ptr3->p = rentry.port;
					insertNode(&head, makeNode(ptr2->iptr, INSERT, rentry.length
							- 24, temp3, 3));
				}
			}
		}
	}
}

void disposeUpdate(TrieNode * v16, TrieNode ** v8ptr)
{
	UpdateNode *curloc, *p, *q;
	curloc = head.next;
	nm = 0;
	if (curloc && curloc->iptr == 0) {
		head.next = curloc->next;
		curloc->next = 0;
		modifyPorts16(v16, v8ptr, curloc);
		freeUpdate(curloc);
	}
	while(head.next)
	{
		curloc = head.next;
		ushort iptr = curloc->iptr;
		head.next = curloc->next;
		curloc->next = 0;

		//for chunk update sta/*****/
//		ltb_update[iptr]++;
//		ltb_layer[iptr] = curloc->layer;

		modifyltb(v8ptr, iptr, curloc);
		freeUpdate(curloc);
	}
	//if (nm == 1)nm1++;
	//else if (nm == 2)nm2++;
	//else nm3++;
}

void freeUpdate(UpdateNode *p){
	UpdateNode *q;
	while(p){
		q = p;
		p = p->next;
		free(q);
	}
}

/*void modifyltb(TrieNode ** v8ptr, ushort current, UpdateNode * p)
{
	UpdateNode *q, *r;
	TrieNode *v8 = v8ptr[current];
	ushort GW = ltb[current].chk->groupWidth;
	if (p->level > GW) 
	{
		p->index = (p->index) >> (p->level - GW);
		p->level = GW;
	}
	uint cr = p->index;
	while(v8[cr].t != PORTTYPE){
		cr >>= 1;
	}
	modifyltb_node(v8ptr, current, p, v8[cr]);
	
}*/

void modifyPorts16(TrieNode * v16, TrieNode ** v8ptr, UpdateNode * p)
{
	int i, j, start, end;
	int masklen = 1 << (LAYER1 - p->level);
	int offsetTrie = ((p->index) << (LAYER1 - p->level));
	int offsetPort = offsetTrie - (1 << LAYER1);
	uint cr = p->index;
	TrieNode * ptr;
	
	while (v16[cr].t != PORTTYPE)
	{
		cr >>= 1;
	}
	TrieNode node = v16[cr];

	for (i = offsetPort; i < offsetPort + masklen; i++)
		ports16[i] = node;

	for (int level = p->level + 1; level < LAYER1; level++)
	{
		i = (p->index << (level - p->level));
		j = ((p->index + 1) << (level - p->level));
		for (; i < j; i++)
		{
			if (v16[i].t == PORTTYPE)
			{
				start = (i << (LAYER1 - level)) - (1 << LAYER1);
				end = ((i + 1) << (LAYER1 - level)) - ( 1 << LAYER1);
				for (; start < end; start++)
				{
					ports16[start] = v16[i];
				}
			}
		}
	}
	for (i = offsetPort, ptr = v16 + offsetTrie; i < offsetPort + masklen; i++, ptr++)
	{
		if (ptr->iptr == 0)
		{
		}
		else if (ptr->t == PORTTYPE)
		{
			ports16[i] = *ptr;
		}
		else
		{
			if (v8ptr[ptr->iptr][0].iptr != ports16[i].iptr)
			{
				v8ptr[ptr->iptr][0] = ports16[i];
				if (v8ptr[ptr->iptr][1].t != PORTTYPE)
				{
					insertNode(&head, makeNode(ptr->iptr, DELETE, ZLEVEL, 0, 2));
				}
			}
			ports16[i] = *ptr;
		}
	}

	/*for statistic*/
	//pointer_number++;
	bytes += masklen * 2;
	element_number += masklen;
}

void modifyltb(TrieNode ** v8ptr, ushort current, UpdateNode * p)
{

	/*for statistic*/
	//pointer_number++;

	TrieNode * v8 = v8ptr[current];
	Sparse * spe = ltb[current].spe;
	if(!spe)
	{
		/*build sparse*/
		modifyltb_new(v8ptr, current, p);
		return;
	}
	if(spe->type == SPRTYPE)
	{
		/*update sparse chunk*/
		int len = 1;
		modifyltb_special(v8ptr, current, p, len);
	}
	else
	{
		/*update bitmap chunk    if cut it, no specific*/
		if (p->layer == 2 && p->level == LAYER2 && p->type == INSERT && v8ptr[current][p->index].t == PORTTYPE)
		{
			// use conditional optimization  
			modifyltb_optimal(v8ptr, current, p);
		}
		else
		{
			if (p->level != 0 && p->level != ZLEVEL)
			{
				/*nomal update*/
				ushort GW = ltb[current].chk->groupWidth;
				//printf("p->level:%u,p->index:%u\n", p->level, p->index);
				if (p->level > GW)
				{
					p->index = (p->index) >> (p->level - GW);
					p->level = GW;
				}
				uint cr = p->index;
				while (v8[cr].t != PORTTYPE)
				{
					cr >>= 1;
				}
				modifyltb_node(v8ptr, current, p, v8[cr]);
			}
			else
			{
				/*refreh the bitmap chunk*/
				modifyltb_refresh(v8ptr, current, p->layer);
			}
		}
			
	}
	return;
}

void modifyltb_new(TrieNode ** v8ptr, ushort current, UpdateNode * p)
{
	Sparse * spa = (Sparse *) malloc(sizeof(Sparse));
	memset(spa, 0, sizeof(Sparse));
	int layer = p->layer == 2 ? LAYER2 : LAYER3;
	ushort original = p->layer == 2 ? 0x3f : 0xff;
	spa->type = SPRTYPE;
	spa->portfix = v8ptr[p->iptr][1].iptr == 0 ? v8ptr[p->iptr][0].p : v8ptr[p->iptr][1].p;
	int i = 0;
	if(p->level != ZLEVEL && p->level != 0)
	{
		spa->special[i].bits = ((p->index) - (1 << p->level)) << (layer - p->level);   //2018.2.1
		spa->special[i].mask = (original >> (layer - p->level)) << ((layer - p->level));
		spa->lookup[i] = v8ptr[current][p->index];
		i++;
	}
	UpdateNode * q = head.next;
	while(q && q->iptr == p->iptr)
	{
		head.next = q->next;
		if(q->level != ZLEVEL && q->level != 0){
		spa->special[i].bits = ((q->index) - (1 << q->level)) << (layer - q ->level);
		spa->special[i].mask = (original >> (layer - q->level)) << ((layer - q->level));
		spa->lookup[i] = v8ptr[current][q->index];
		i++;
		}
		q = head.next;
	}
	if (p->layer == 2)
	{
		for (i = 0; i < SPARSE; i++)
		{
			if (spa->special[i].mask == original && spa->lookup[i].t != PORTTYPE)
			{
				uint cr = (1 << layer) + spa->special[i].bits;
				while (v8ptr[current][cr].t != PORTTYPE)
				{
					cr >>= 1;
				}
				TrieNode * ptr3 = v8ptr[spa->lookup[i].iptr];
				if (ptr3[0].iptr != v8ptr[current][cr].iptr)
				{
					ptr3[0] = v8ptr[current][cr];
					if (ptr3[1].t != PORTTYPE)
					{
						insertNode(&head, makeNode(spa->lookup[i].iptr, INSERT, ZLEVEL, 0, 3));
					}
				}
			}
		}
	}


	ltb[current].spe = spa;

	/*for statistic*/
	bytes += sizeof(spa);
	pointer_number += 1;
}

void modifyltb_special(TrieNode ** v8ptr, ushort current, UpdateNode * p, int len)
{
	/*for statistic*/
	pointer_number += 1;

	TrieNode * v8 = v8ptr[current];
	Sparse * spe = ltb[current].spe;

	int cnt = 0;
	int i;

	int layer = p->layer == 2 ? LAYER2 : LAYER3;
	ushort original = p->layer == 2 ? 0x3f : 0xff;
	UpdateNode *q;
	ushort bits, mask;
	q = p;


	if (q->level == 0 || q->level == ZLEVEL) 
	{
		if (v8ptr[current][1].t == PORTTYPE)
		{
			spe->portfix = v8ptr[current][1].p;
		}
		else
		{
			spe->portfix = v8ptr[current][0].p;
		}
		for (i = 0; i < SPARSE; i++)
		{
			if (spe->special[i].mask == original && spe->lookup[i].t != PORTTYPE)
			{
				uint cr = (1 << layer) + spe->special[i].bits;
				while (v8[cr].t != PORTTYPE)
				{
					cr >>= 1;
				}
				TrieNode * ptr3 = v8ptr[spe->lookup[i].iptr];
				if (ptr3[0].iptr != v8[cr].iptr)
				{
					ptr3[0] = v8[cr];
					if (ptr3[1].t != PORTTYPE)
					{
						insertNode(&head, makeNode(spe->lookup[i].iptr, INSERT, ZLEVEL, 0, layer+1));
					}
				}
			}
		}
	}
	else
	{
		Special * special =(Special *)malloc(sizeof(Special) * (len + SPARSE));
		TrieNode * lookup = (TrieNode *)malloc(sizeof(TrieNode) * (len + SPARSE));
		memset(special, 0, sizeof(Special) * (len + SPARSE));
		memset(lookup, 0, sizeof(TrieNode) * (len + SPARSE));
		while(cnt < SPARSE && spe->special[cnt].mask != 0)
		{
			special[cnt] = spe->special[cnt];
			lookup[cnt] = spe->lookup[cnt];
			cnt ++;
		}

		bits = ((q->index) << (layer - q->level)) - (1 << layer);
		mask = (original >> (layer - q->level)) << (layer - q->level);
		if(q->type == INSERT)
		{
			int flag = 0;
			for(i = cnt-1; i >= 0; i--)
			{
				if(special[i].bits == bits && special[i].mask == mask)
				{
					flag = 1;
					break;
				}
			}
			if(flag)
			{
				lookup[i] = v8ptr[current][q->index];
			} 
			else 
			{
				for (i = cnt - 1; i >= 0; i--) 
				{
					if (special[i].mask < mask) 
					{
						special[i + 1] = special[i];
						lookup[i + 1] = lookup[i];
					} 
					else 
					{
						break;
					}
				}
				special[i + 1].bits = bits;
				special[i + 1].mask = mask;
				lookup[i + 1] = v8ptr[current][q->index];
				cnt ++;
			}
		}
		else if(q->type == DELETE)
		{
			for (i = cnt - 1; i >= 0; i--) 
			{
				if (special[i].bits == bits && special[i].mask == mask) {
					for(; i < cnt - 1; i++)
					{
						special[i] = special[i+1];
						lookup[i]  = lookup[i+1];
					}
					cnt --;
					break;
				}
			}
		}
		if(cnt <= SPARSE)
		{
			for(i = 0; i < cnt; i++)
			{
				spe->special[i] = special[i];
				spe->lookup[i] = lookup[i];
				if(spe->special[i].mask == original && spe->lookup[i].t != PORTTYPE)
				{
					uint cr = (1 << layer) + spe->special[i].bits;
					while(v8[cr].t != PORTTYPE)
					{
						cr >>= 1;
					}
					TrieNode * ptr3 = v8ptr[spe->lookup[i].iptr];
					if(ptr3[0].iptr != v8[cr].iptr)
					{
						ptr3[0] = v8[cr];
						if(ptr3[1].t != PORTTYPE)
						{
							insertNode(&head, makeNode(spe->lookup[i].iptr, INSERT, ZLEVEL, 0, layer+1));
						}
					}
				}
			}
			for(i = cnt; i < SPARSE; i++)
			{
				spe->special[i].mask = 0;
				spe->special[i].bits = 0;
			}
			/*for statistic*/
			bytes += sizeof(Sparse);
			nm++;
		}
		else
		{
			modifyltb_spa2bit(v8ptr, current, spe->portfix, cnt, special, lookup, p->layer);
			free(spe);
		}
		free(special);
		free(lookup);

	}	
}

void modifyltb_spa2bit(TrieNode ** v8ptr, ushort current, ushort portfix, int cnt, Special * special, TrieNode * lookup, int layer_n)
{

	int level;
	int i, j;
	int start, end;

	TrieNode * ptr;
	int layer = layer_n == 2 ? LAYER2 : LAYER3;

	TrieNode * v8 = v8ptr[current];
	ushort * bitmask = (ushort *) malloc(sizeof(ushort) * (1 << layer));
	TrieNode * ports = (TrieNode *) malloc(sizeof(TrieNode) * (1 << layer));
	memset(bitmask, 0, sizeof(bitmask));

	for (i = 0; i < (1 << layer); i++){
		ports[i].t = PORTTYPE;
		ports[i].p = portfix;
	}

	for(j = cnt - 1; j >= 0; j--)
	{
		int masklen = 0;
		ushort mask = special[j].mask;
		while(!(mask & 0x1))
		{
			masklen++;
			mask >>= 1;
		}
		if(masklen == 0)
		{
			if(lookup[j].t == PORTTYPE)
			{
				ports[special[j].bits] = lookup[j];
			}
			else
			{
				TrieNode * ptr = v8 + special[j].bits + (1 << layer);
				if(v8ptr[ptr->iptr][0].iptr != ports[special[j].bits].iptr )
				{
					v8ptr[ptr->iptr][0] = ports[special[j].bits];
					if (v8ptr[ptr->iptr][1].t != PORTTYPE)
						insertNode(&head, makeNode(ptr->iptr, DELETE, ZLEVEL, 0, layer + 1));
				}
				ports[special[j].bits] = lookup[j];
			}
		}
		else
		{
			start = special[j].bits;
			end = start + (1 << masklen);
			for (; start < end; start++) {
				ports[start] = lookup[j];
			}
		}

	}

	TrieNode previous;
	previous.iptr = 0;

	for (i = 0; i < (1 << layer); i++) {
		if (previous.iptr == ports[i].iptr) {
			bitmask[i] = 0;
		} else {
			bitmask[i] = 1;
			previous.iptr = ports[i].iptr;
		}
	}

	int group, cluster, bit;
	int GROUP_ = layer_n == 2 ? GROUP2 : GROUP;
	int CLUSTER_ = layer - GROUP_ - BIT;
	Chunk *chkptr = (Chunk *)malloc(sizeof(Chunk));
	chkptr->groupWidth = GROUP_;
	chkptr->type = BITTYPE;
	//chk->mask
	ushort **base = chkptr->base = (ushort **)malloc(sizeof(ushort *) * (1 << (chkptr->groupWidth)));

	/*for statistic*/
	bytes += sizeof(ushort *) * (1 << (chkptr->groupWidth));
	/**************/

	for (i = 0, group = 0; group < (1 << GROUP_); i += (1 << (CLUSTER_ + BIT)), group++)
	{
		bitmask[i] = 1;
		int cnt = 0;
		for (start = i; start < i + (1 << (CLUSTER_ + BIT)); start++)
			if (bitmask[start] == 1)
				cnt++;
		base[group] = (ushort *)malloc(sizeof(ushort) * ((1 << CLUSTER_) + cnt));

		/*for statistic*/
		bytes += sizeof(ushort) * ((1 << CLUSTER) + cnt);
		/**************/

		CodeWord *cwd = (CodeWord *)(base[group]);
		TrieNode *lookup = (TrieNode *)(base[group] + (1 << CLUSTER_));
		int num8;
		int preOne = 0;
		int curOne = 0;
		for (cluster = 0; cluster < (1 << CLUSTER_); cluster++)
		{
			num8 = 0;
			curOne = 0;
			for (bit = i + (cluster << BIT); bit < i + ((cluster + 1) << BIT); bit++)
			{
				num8 <<= 1;
				if (bitmask[bit] == 1)
				{
					num8++;
					curOne++;
					*lookup = ports[bit];
					lookup++;
				}
			}
			cwd->before = preOne;
			cwd->bitmask = num8;
			cwd++;
			preOne += curOne;
		}
	}
	ltb[current].chk = chkptr;
	free(bitmask);
	free(ports);
	nm++; 
}

void modifyltb_optimal(TrieNode ** v8ptr, ushort current, UpdateNode * p)
{
	/* for statistic */
	pointer_number += 1;

	uint bits, group, cluster, bit;
	uint GROUP_ = ltb[current].chk->groupWidth;
	uint CLUSTER_ = LAYER2 - GROUP_ - BIT;
	uint clusterMax = (1 << CLUSTER_) - 1;
	uint bitMax = (1 << BIT) - 1;
	bits = p->index - (1 << p->level);
	group = bits >> (CLUSTER_ + BIT);
	if (CLUSTER_ == 0)
		cluster = 0;
	else
		cluster  = (bits << (32 - CLUSTER_ - BIT)) >> (32 - CLUSTER_);
	bit = (bits << (32 - BIT)) >> (32 - BIT);

	TrieNode node = v8ptr[current][p->index];
	CodeWord * cwdbase = (CodeWord *)ltb[current].chk->base[group];
	TrieNode * lkpbase = (TrieNode *)(cwdbase + (1 << CLUSTER_));
	int numOfLookup = cwdbase[cluster].before + bits8[cwdbase[cluster].bitmask >> (OFFSET - bit)];
	int totalLookup = cwdbase[(1 << CLUSTER_) - 1].before + bits8[cwdbase[(1 << CLUSTER_) - 1].bitmask];
	//int numOfLookup = cwdbase[cluster].before + popcnt(cwdbase[cluster].bitmask >> (OFFSET - bit));
	//int totalLookup = cwdbase[(1 << CLUSTER_) - 1].before + popcnt(cwdbase[(1 << CLUSTER_) - 1].bitmask);
	//if (numOfLookup > totalLookup)
	//{
	//	printf("aaa\n");
	//	for(int i = 0; i < (1 << CLUSTER_); i++)
	//		printf("%x\t%u\n", cwdbase[i].bitmask, cwdbase[i].before);
	//	printf("aaa\n");
	//}

	if (cluster == 0 && bit == 0)
	{
		/*the first bit of cluser 1, which must be 1; */
		uint x1;
		uint off = OFFSET - 1;
		x1 = uint(cwdbase[0].bitmask) << (31 - off) >> 31;
		switch (x1)
		{
			case 0:
			{
				/*10 -> 11*/
				ushort * base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort));

				/*for statistic*/
				bytes += ((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort);
				/**************/

				CodeWord *cwdend = (CodeWord*)base;
				TrieNode *lkpend = (TrieNode*)(cwdend + (1 << CLUSTER_));
				for(int i = 0; i < (1 << CLUSTER_); i++)
					cwdend[i] = cwdbase[i];
				cwdend[0].bitmask |= (1 << (OFFSET - 1));
				for(int i = 1; i < (1 << CLUSTER_); i++)
					cwdend[i].before++;
				*lkpend = node;
				lkpend++;
				for(int i = 0; i < totalLookup; i++, lkpend++, lkpbase++){
					*lkpend = *lkpbase;
				}
				free((ushort *)cwdbase);
				break;
			}
			case 1:
			{
				/* 11 -> 11*/
				*lkpbase = node;
				break;
			}
		}
	}
	else if (cluster == clusterMax  && bit == bitMax)
	{
		/*the last bit of the last cluster, which has no following bit*/
		uint x0;
		x0 = uint(cwdbase[cluster].bitmask) << 31 >> 31;
		switch (x0)
		{
			case 0:
			{
				/*0 -> 1*/
				ushort * base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort));

				/*for statistic*/
				bytes += ((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort);
				/**************/

				CodeWord *cwdend = (CodeWord*)base;
				TrieNode *lkpend = (TrieNode*)(cwdend + (1 << CLUSTER_));
				for(int i = 0; i < (1 << CLUSTER_); i++)
					cwdend[i] = cwdbase[i];
				cwdend[clusterMax].bitmask |= 1;
				for(int i = 0; i < totalLookup; i++, lkpend++, lkpbase++){
					*lkpend = *lkpbase;
				}
				*lkpend = node;
				free((ushort *)cwdbase);
				break;
			}
			case 1:
			{
				/*1 ->1 */
				lkpbase[totalLookup - 1] = node;
				break;
			}
		}
	}
	else if (bit == bitMax)
	{
		 /* the last bit of every cluster, which has infuluence on two cluser i.e. two codeword*/
		uint x0,x1;
		uint off = OFFSET;
		x0 = uint(cwdbase[cluster].bitmask) << 31 >>31;
		x1 = uint(cwdbase[cluster+1].bitmask) << (31 - off) >> 31;
	    switch((x0 << 1) + x1)
		{
			case 0: //00 -> 11
			{
				ushort * base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 2) * sizeof(ushort));

				/*for statistic*/
				bytes += ((1 << CLUSTER_) + totalLookup + 2) * sizeof(ushort);
				/**************/

				CodeWord *cwdend = (CodeWord*)base;
				TrieNode *lkpend = (TrieNode*)(cwdend + (1 << CLUSTER_));
				/*code word*/
				for(int i = 0; i < (1 << CLUSTER_); i++)
					cwdend[i] = cwdbase[i];
				cwdend[cluster].bitmask |= 1;
				cwdend[cluster+1].bitmask |= (1 << OFFSET);

				cwdend[cluster+1].before++;
				for(int i = cluster+2; i < (1 << CLUSTER_); i++)
					cwdend[i].before += 2;
				/*lookup table*/
				for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++)
					*lkpend = *lkpbase;
				*lkpend++ = node;
				*lkpend++ = *(lkpbase - 1);
				for (int i = numOfLookup ; i < totalLookup; i++, lkpend++, lkpbase++)
					*lkpend = *lkpbase; 
				free((ushort *)cwdbase);
				break;
			}
			case 1: //01 -> 11 or 10
			{
				if (lkpbase[numOfLookup].iptr == node.iptr)
				{			
					cwdbase[cluster].bitmask |= 1;
					cwdbase[cluster+1].bitmask &= ~(1 << OFFSET);
					cwdbase[cluster+1].before++; // important!
				}
				else
				{
					/* 01->11*/
					ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort));

					/*for statistic*/
					bytes += ((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort);
					/**************/

					CodeWord *cwdend = (CodeWord *)base;
					TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
					/*code word*/
					for (int i = 0; i < (1 << CLUSTER_); i++)
						cwdend[i] = cwdbase[i];
					cwdend[cluster].bitmask |= 1;

					for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
						cwdend[i].before += 1;
					/*lookup table*/
					for (int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					*lkpend++ = node;
					for (int i = numOfLookup; i < totalLookup; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					free((ushort *)cwdbase);
				}
				break;
			}
			case 2: //10 -> 11 or 01
			{
				if (lkpbase[numOfLookup - 2].iptr == node.iptr)
				{
					/*10 ->01*/
					cwdbase[cluster].bitmask &= 0xFE;
					cwdbase[cluster+1].bitmask |= (1 << OFFSET);
					cwdbase[cluster + 1].before--;
				}
				else
				{
					/*10 -> 11 */

					ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort));

					/*for statistic*/
					bytes += ((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort);
					/**************/

					CodeWord *cwdend = (CodeWord *)base;
					TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
					/*code word*/
					for (int i = 0; i < (1 << CLUSTER_); i++)
						cwdend[i] = cwdbase[i];
					cwdend[cluster + 1].bitmask |= (1 << OFFSET);

					for (int i = cluster + 2; i < (1 << CLUSTER_); i++)
						cwdend[i].before += 1;
					/*lookup table*/
					for (int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					*lkpend++ = node;
					for (int i = numOfLookup - 1; i < totalLookup; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					free((ushort *)cwdbase);
				}
				break;
			}
			case 3: //11 -> 11 or 10 or 01 or 00
			{
				if (lkpbase[numOfLookup - 2].iptr == node.iptr)
				{
					if (lkpbase[numOfLookup].iptr == node.iptr)
					{
						/*11 -> 00*/
						ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup - 2) * sizeof(ushort));

						/*for statistic*/
						bytes += ((1 << CLUSTER_) + totalLookup - 2) * sizeof(ushort);
						/**************/

						CodeWord *cwdend = (CodeWord *)base;
						TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
						/*code word*/
						for (int i = 0; i < (1 << CLUSTER_); i++)
							cwdend[i] = cwdbase[i];

						cwdend[cluster].bitmask  &= 0xFE;
						cwdend[cluster+1].bitmask &= ~(1 << OFFSET);

						cwdend[cluster+1].before -= 1;
						for (int i = cluster + 2; i < (1 << CLUSTER_); i++)
							cwdend[i].before -= 2;
						/*lookup table*/
						for (int i = 0; i < numOfLookup - 1 ; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						lkpbase += 2;
						for (int i = numOfLookup + 1; i < totalLookup; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						free((ushort *)cwdbase);
					}
					else
					{
						/*11 -> 01*/
						ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort));

						/*for statistic*/
						bytes += ((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort);
						/**************/

						CodeWord *cwdend = (CodeWord *)base;
						TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
						/*code word*/
						for (int i = 0; i < (1 << CLUSTER_); i++)
							cwdend[i] = cwdbase[i];

						cwdend[cluster].bitmask  &= 0xFE;

						for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
							cwdend[i].before -= 1;
						/*lookup table*/
						for (int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						lkpbase++;
						for (int i = numOfLookup; i < totalLookup; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						free((ushort *)cwdbase);
					}
				}
				else
				{
					if (lkpbase[numOfLookup].iptr == node.iptr)
					{
						/*11 -> 10*/
						ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort));


						/*for statistic*/
						bytes += ((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort);
						/**************/

						CodeWord *cwdend = (CodeWord *)base;
						TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
						/*code word*/
						for (int i = 0; i < (1 << CLUSTER_); i++)
							cwdend[i] = cwdbase[i];

						cwdend[cluster+1].bitmask &= ~(1 << OFFSET);

						for (int i = cluster + 2; i < (1 << CLUSTER_); i++)
							cwdend[i].before -= 1;
						/*lookup table*/
						for (int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						lkpbase++;
						for (int i = numOfLookup; i < totalLookup; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						free((ushort *)cwdbase);
					}
					else
					{
						/*11 -> 11*/
						lkpbase[numOfLookup - 1] = node;
					}
				}
				break;
			}
		}
	}
	else
	{
		/*ordinary situation*/
		ushort x;
		uint off0 = OFFSET - bit;
		uint off1 = off0 - 1;
		x = uint(cwdbase[cluster].bitmask) << (30 - off1) >> 30;
	    switch(x)
		{
			case 0: //00 -> 11
			{
				ushort * base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 2) * sizeof(ushort));

				/*for statistic*/
				bytes += ((1 << CLUSTER_) + totalLookup + 2) * sizeof(ushort);
				/**************/

				CodeWord *cwdend = (CodeWord*)base;
				TrieNode *lkpend = (TrieNode*)(cwdend + (1 << CLUSTER_));
				/*code word*/
				for(int i = 0; i < (1 << CLUSTER_); i++)
					cwdend[i] = cwdbase[i];
				cwdend[cluster].bitmask |= (1 << off0);
				cwdend[cluster].bitmask |= (1 << off1);

				for(int i = cluster + 1; i < (1 << CLUSTER_); i++)
					cwdend[i].before += 2;
				/*lookup table*/
				for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++)
					*lkpend = *lkpbase;
				*lkpend++ = node;
				*lkpend++ = *(lkpbase - 1);
				for (int i = numOfLookup ; i < totalLookup; i++, lkpend++, lkpbase++)
					*lkpend = *lkpbase; 
				free((ushort *)cwdbase);
				break;
			}
			case 1: //01 -> 11 or 10
			{

				if (lkpbase[numOfLookup].iptr == node.iptr)
				{
					/* 01->10*/
					cwdbase[cluster].bitmask |= (1 << off0);
					cwdbase[cluster].bitmask &= ~(1 << off1);
				}
				else
				{
					/* 01->11*/
					ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort));

					/*for statistic*/
					bytes += ((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort);
					/**************/

					CodeWord *cwdend = (CodeWord *)base;
					TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
					/*code word*/
					for (int i = 0; i < (1 << CLUSTER_); i++)
						cwdend[i] = cwdbase[i];
					cwdend[cluster].bitmask |= (1 << off0);

					for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
						cwdend[i].before += 1;
					/*lookup table*/
					for (int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					*lkpend++ = node;
					for (int i = numOfLookup; i < totalLookup; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					free((ushort *)cwdbase);
				}
				break;
			}
			case 2: //10 -> 11 or 01
			{
				if (lkpbase[numOfLookup - 2].iptr == node.iptr)
				{
					/*10 ->01*/				
					cwdbase[cluster].bitmask &= (~(1 << off0));
					cwdbase[cluster].bitmask |= (1 << off1);
				}
				else
				{
					/*10 ->11*/
					ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort));

					/*for statistic*/
					bytes += ((1 << CLUSTER_) + totalLookup + 1) * sizeof(ushort);
					/**************/

					CodeWord *cwdend = (CodeWord *)base;
					TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
					/*code word*/
					for (int i = 0; i < (1 << CLUSTER_); i++)
						cwdend[i] = cwdbase[i];
					cwdend[cluster].bitmask |= (1 << off1);

					for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
						cwdend[i].before += 1;
					/*lookup table*/
					for (int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;
					*lkpend++ = node;
					for (int i = numOfLookup - 1; i < totalLookup; i++, lkpend++, lkpbase++)
						*lkpend = *lkpbase;

					//printf("****************************************");
					//printf("\nold\n");
					//printf("totallookup: %d\n", totalLookup);
					//printf("current: %d\n", current);
					//for (int i = 0; i < (1 << CLUSTER_); i++)
					//{
					//	printf("%x\t%u\n", cwdbase[i].bitmask, cwdbase[i].before);
					//	/*if (cwdbase[i].before > 200)
					//	{
					//		printf("error");
					//	}*/
					//}
					//printf("----------------------------------\n");
					//lkpbase = (TrieNode *)(cwdbase + (1 << CLUSTER_));
					//for (int i = 0; i < totalLookup; i++)
					//{
					//	if (lkpbase[i].t == PORTTYPE)
					//	{
					//		printf("PORT:%hu\n", lkpbase[i].p);
					//	}
					//	else
					//	{
					//		printf("IPTR:%hu\n", lkpbase[i].iptr);
					//	}				
					//	/*if (cwdbase[i].before > 200)
					//	{
					//	printf("error");
					//	}*/
					//}
					//	

					//printf("\nnew\n");
					//for (int i = 0; i < (1 << CLUSTER_); i++)
					//	printf("%x\t%u\n", cwdend[i].bitmask, cwdend[i].before);
					//free((ushort *)cwdbase);
					//printf("----------------------------------\n");
					//lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
					//for (int i = 0; i < totalLookup+1; i++)
					//{
					//	if (lkpend[i].t == PORTTYPE)
					//	{
					//		printf("PORT:%hu\n", lkpend[i].p);
					//	}
					//	else
					//	{
					//		printf("IPTR:%hu\n", lkpend[i].iptr);
					//	}
					//	/*if (cwdbase[i].before > 200)
					//	{
					//	printf("error");
					//	}*/
					//}
				}
				break;
			}
			case 3: //11 -> 11 or 10 or 01 or 00
			{
				if (lkpbase[numOfLookup - 2].iptr == node.iptr)
				{
					if (lkpbase[numOfLookup].iptr == node.iptr)
					{
						/*11 -> 00*/
						ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup - 2) * sizeof(ushort));

						/*for statistic*/
						bytes += ((1 << CLUSTER_) + totalLookup - 2) * sizeof(ushort);
						/**************/

						CodeWord *cwdend = (CodeWord *)base;
						TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
						/*code word*/
						for (int i = 0; i < (1 << CLUSTER_); i++)
							cwdend[i] = cwdbase[i];

						cwdend[cluster].bitmask &= (~(1 << off0));
						cwdend[cluster].bitmask &= (~(1 << off1));

						for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
							cwdend[i].before -= 2;
						/*lookup table*/
						for (int i = 0; i < numOfLookup - 1 ; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						lkpbase += 2;
						for (int i = numOfLookup + 1; i < totalLookup; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						free((ushort *)cwdbase);
					}
					else
					{
						/*11 -> 01*/
						ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort));

						/*for statistic*/
						bytes += ((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort);
						/**************/

						CodeWord *cwdend = (CodeWord *)base;
						TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
						/*code word*/
						for (int i = 0; i < (1 << CLUSTER_); i++)
							cwdend[i] = cwdbase[i];

						cwdend[cluster].bitmask  &= (~(1 << off0));

						for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
							cwdend[i].before -= 1;
						/*lookup table*/
						for (int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						lkpbase++;
						for (int i = numOfLookup; i < totalLookup; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						free((ushort *)cwdbase);
					}
				}
				else
				{
					if (lkpbase[numOfLookup].iptr == node.iptr)
					{
						/*11 -> 10*/
						ushort *base = ltb[current].chk->base[group] = (ushort *)malloc(((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort));

						/*for statistic*/
						bytes += ((1 << CLUSTER_) + totalLookup - 1) * sizeof(ushort);
						/**************/

						CodeWord *cwdend = (CodeWord *)base;
						TrieNode *lkpend = (TrieNode *)(cwdend + (1 << CLUSTER_));
						/*code word*/
						for (int i = 0; i < (1 << CLUSTER_); i++)
							cwdend[i] = cwdbase[i];

						cwdend[cluster].bitmask &= (~(1 << off1));

						for (int i = cluster + 1; i < (1 << CLUSTER_); i++)
							cwdend[i].before -= 1;
						/*lookup table*/
						for (int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						lkpbase++;
						for (int i = numOfLookup; i < totalLookup; i++, lkpend++, lkpbase++)
							*lkpend = *lkpbase;
						free((ushort *)cwdbase);
					}
					else
					{
						/*11 -> 11*/
						lkpbase[numOfLookup - 1] = node;
					}
				}
				break;
			}
		}
	}
}	

/*	
{
	CodeWord * cwdbase = (CodeWord *)ltb[p->iptr].chk->base[bix];
	TrieNode * lkpbase = (TrieNode *)(cwdbase + (1 << GROUP));
	int numOfLookup = cwdbase[group].before + bits8[cwdbase[group].bitmask >> (OFFSET - cluster)];
	int totalLookup = cwdbase[(1 << GROUP) - 1].before + bits8[cwdbase[(1 << GROUP) - 1].bitmask];
			
	if(group == 0 && cluster == 0){
		if(bits8[cwdbase[0].bitmask >> (OFFSET - 1)] == 1){
			ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 1) * sizeof(ushort));
			CodeWord *cwdend = (CodeWord*)base;
			TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
			for(int i = 0; i < (1 << GROUP); i++)
				cwdend[i] = cwdbase[i];
			cwdend[0].bitmask |= (1 << (OFFSET - 1));
			for(int i = 1; i < (1 << GROUP); i++)
				cwdend[i].before++;
			*lkpend = vh[p->index];
			lkpend++;
			for(int i = 0; i < totalLookup; i++, lkpend++, lkpbase++){
				*lkpend = *lkpbase;
			}
			free((ushort *)cwdbase);
		}else{
			*lkpbase = vh[p->index];
		}
	}else{				
		if(cwdbase[group].bitmask & (1 << (OFFSET - cluster))){
			if(group == ((1 << GROUP) - 1) && cluster == ((1 << CLUSTER) - 1)){
				lkpbase[totalLookup - 1] = vh[p->index];
			}else{
				if(cluster == ((1 << CLUSTER) - 1)){
					if(cwdbase[group + 1].bitmask & (1 << ((1 << CLUSTER) - 1))){
						lkpbase[numOfLookup - 1] = vh[p->index];
					}else{
						ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 1) * sizeof(ushort));
						CodeWord *cwdend = (CodeWord*)base;
						TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
						for(int i = 0; i < (1 << GROUP); i++)
							cwdend[i] = cwdbase[i];
						cwdend[group + 1].bitmask |= (1 << OFFSET);
						for(int i = group + 2; i < (1 << GROUP); i++)
							cwdend[i].before++;									
						for(int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						*lkpend = vh[p->index];
						lkpend++;
						for(int i = 0; i < totalLookup - numOfLookup + 1; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						free((ushort *)cwdbase);
					}
				}else{
					if(cwdbase[group].bitmask & (1 << (OFFSET - cluster - 1))){
						lkpbase[numOfLookup - 1] = vh[p->index];
					}else{
						ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 1) * sizeof(ushort));
						CodeWord *cwdend = (CodeWord*)base;
						TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
						for(int i = 0; i < (1 << GROUP); i++)
							cwdend[i] = cwdbase[i];
						cwdend[group].bitmask |= (1 << (OFFSET - cluster - 1));
						for(int i = group + 1; i < (1 << GROUP); i++)
							cwdend[i].before++;									
						for(int i = 0; i < numOfLookup - 1; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						*lkpend = vh[p->index];
						lkpend++;
						for(int i = 0; i < totalLookup - numOfLookup + 1; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						free((ushort *)cwdbase);
					}
				}
			}
		}else{
			if(group == ((1 << GROUP) - 1) && cluster == ((1 << CLUSTER) - 1)){
				ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 1) * sizeof(ushort));
				CodeWord *cwdend = (CodeWord*)base;
				TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
				for(int i = 0; i < (1 << GROUP); i++)
					cwdend[i] = cwdbase[i];
				cwdend[(1 << GROUP) - 1].bitmask |= 1;															
				for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++){
					*lkpend = *lkpbase;
				}
				*lkpend = vh[p->index];						
				free((ushort *)cwdbase);							
			}else{
				if(cluster == ((1 << CLUSTER) - 1)){
					if(cwdbase[group + 1].bitmask & (1 << OFFSET)){
						ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 1) * sizeof(ushort));
						CodeWord *cwdend = (CodeWord*)base;
						TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
						for(int i = 0; i < (1 << GROUP); i++)
							cwdend[i] = cwdbase[i];
						cwdend[group].bitmask |= 1;
						for(int i = group + 1; i < (1 << GROUP); i++)
							cwdend[i].before++;	
						for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						*lkpend = vh[p->index];
						lkpend++;
						for(int i = 0; i < totalLookup - numOfLookup; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						free((ushort *)cwdbase);
					}else{
						ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 2) * sizeof(ushort));
						CodeWord *cwdend = (CodeWord*)base;
						TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
						for(int i = 0; i < (1 << GROUP); i++)
							cwdend[i] = cwdbase[i];
						cwdend[group].bitmask |= 1;
						cwdend[group + 1].bitmask |= (1 << (OFFSET));
						cwdend[group + 1].before++;
						for(int i = group + 2; i < (1 << GROUP); i++)
							cwdend[i].before += 2;									
						for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						*lkpend = vh[p->index];
						lkpend++;
						lkpbase--;
						for(int i = 0; i < totalLookup - numOfLookup + 1; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						free((ushort *)cwdbase);
					}
				}else{
					if(cwdbase[group].bitmask & (1 << (OFFSET - cluster - 1))){
						ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 1) * sizeof(ushort));
						CodeWord *cwdend = (CodeWord*)base;
						TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
						for(int i = 0; i < (1 << GROUP); i++)
							cwdend[i] = cwdbase[i];
						cwdend[group].bitmask |= (1 << (OFFSET - cluster));
						for(int i = group + 1; i < (1 << GROUP); i++)
							cwdend[i].before++;
						for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						*lkpend = vh[p->index];
						lkpend++;
						for(int i = 0; i < totalLookup - numOfLookup; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						free((ushort *)cwdbase);
					}else{
						ushort * base = ltb[p->iptr].chk->base[bix]	= (ushort *)malloc(((1 << GROUP) + totalLookup + 2) * sizeof(ushort));
						CodeWord *cwdend = (CodeWord*)base;
						TrieNode *lkpend = (TrieNode*)(cwdend + (1 << GROUP));
						for(int i = 0; i < (1 << GROUP); i++)
							cwdend[i] = cwdbase[i];
						cwdend[group].bitmask |= (1 << (OFFSET - cluster));
						cwdend[group].bitmask |= (1 << (OFFSET - cluster - 1));									
						for(int i = group + 1; i < (1 << GROUP); i++)
							cwdend[i].before += 2;									
						for(int i = 0; i < numOfLookup; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						*lkpend = vh[p->index];
						lkpend++;
						lkpbase--;
						for(int i = 0; i < totalLookup - numOfLookup + 1; i++, lkpend++, lkpbase++){
							*lkpend = *lkpbase;
						}
						free((ushort *)cwdbase);
					}
				}
			}
		}							
	}			
}*/

void modifyltb_node(TrieNode ** v8ptr, ushort current, UpdateNode * p, TrieNode node)
{
	int level;
	int i, j;
	int start, end;
	TrieNode * ptr;
	TrieNode * v8 = v8ptr[current];

	int layer = p->layer == 2 ? LAYER2 : LAYER3;
	int masklen = 1 << (layer - p->level);
	int offset = ((p->index) << (layer - p->level));
	ushort * bitmask = (ushort *) malloc(sizeof(ushort) * (masklen));
	TrieNode * ports = (TrieNode *) malloc(sizeof(TrieNode) * (masklen));

	
	for (i = 0; i < masklen; i++)
		ports[i] = node;

	for (level = p->level + 1; level < layer; level++) 
	{
		i = (p->index << (level - p->level));
		j = ((p->index + 1) << (level - p->level));
		for (; i < j; i++) 
		{
			if (v8[i].t == PORTTYPE) 
			{
				start = (i << (layer - level)) -  offset;
				end = ((i + 1) << (layer - level))  - offset;
				for (; start < end; start++) 
				{
					ports[start] = v8[i];
				}
			}
		}
	}
	for (i = 0, ptr = v8 + offset; i < masklen; i++, ptr++) 
	{
		if (ptr->iptr == 0) 
		{
		} 
		else if (ptr->t == PORTTYPE) 
		{
			ports[i] = *ptr;
		} 
		else 
		{
			if(v8ptr[ptr->iptr][0].iptr != ports[i].iptr)
			{
				v8ptr[ptr->iptr][0] = ports[i];
				if(v8ptr[ptr->iptr][1].t != PORTTYPE)
				{
					insertNode(&head, makeNode(ptr->iptr, DELETE, ZLEVEL, 0, layer + 1));
				}
			}
			ports[i] = *ptr;
		}
	}

	TrieNode previous;
	previous.iptr = 0;

	for (i = 0; i < (masklen); i++) 
	{
		if (previous.iptr == ports[i].iptr) 
		{
			bitmask[i] = 0;
		} 
		else 
		{
			bitmask[i] = 1;
			previous.iptr = ports[i].iptr;
		}
	}

	ushort ** base = ltb[current].chk->base;
	int GROUP_ = ltb[current].chk->groupWidth;
	int group, cluster, bit;
	int CLUSTER_ = layer - GROUP_ - BIT;
	

	for (i = 0, group = ((p->index - (1 << p->level)) << (GROUP_ - p->level))/*??*/;i < masklen; i += (1 << (CLUSTER_ + BIT)), group++)
	{
		bitmask[i] = 1;
		int cnt = 0;
		for (start = i; start < i + (1 << (CLUSTER_ + BIT)); start++)
			if (bitmask[start] == 1)
				cnt++;
		if (base[group] != NULL)
		{
			free(base[group]);
		}
		base[group] = (ushort *)malloc(sizeof(ushort) * ((1 << CLUSTER_) + cnt));

		/*for statistic*/
		bytes += sizeof(ushort) * ((1 << CLUSTER_) + cnt);
		pointer_number += 1;

		CodeWord *cwd = (CodeWord *)(base[group]);
		TrieNode *lookup = (TrieNode *)(base[group] + (1 << CLUSTER_));
		int num8;
		int preOne = 0;
		int curOne = 0;
		for (cluster = 0; cluster < (1 << CLUSTER_); cluster++)
		{
			num8 = 0;
			curOne = 0;
			for (bit = i + (cluster << BIT); bit < i + ((cluster + 1) << BIT); bit++)
			{
				num8 <<= 1;
				if (bitmask[bit] == 1)
				{
					num8++;
					curOne++;
					*lookup = ports[bit];
					lookup++;
				}
			}
			cwd->before = preOne;
			cwd->bitmask = num8;
			cwd++;
			preOne += curOne;
		}
	}
	free(bitmask);
	free(ports);
}


void modifyltb_refresh(TrieNode ** v8ptr, ushort current, int layer_n)
{
	int i, j;
	TrieNode *ptr;
	TrieNode *v8 = v8ptr[current];
	int start, end;
	int level;
	int layer = layer_n == 2 ? LAYER2 : LAYER3;
	ushort *bitmask = (ushort *)malloc(sizeof(ushort) * (1 << layer));
	TrieNode *ports = (TrieNode *)malloc(sizeof(TrieNode) * (1 << layer));
	memset(bitmask, 0, sizeof(bitmask));

	if (v8[1].t == PORTTYPE)
	{
		ptr = &v8[1];
	}
	else
	{
		ptr = &v8[0];
	}

	for (i = 0; i < (1 << layer); i++)
		ports[i] = *ptr;

	for (level = 1; level < layer; level++)
	{
		i = (1 << level);
		j = i << 1;
		for (; i < j; i++)
		{
			if (v8[i].t == PORTTYPE)
			{
				start = (i << (layer - level)) - (1 << layer);
				end = ((i + 1) << (layer - level)) - (1 << layer );
				for (; start < end; start++)
				{
					ports[start] = v8[i];
				}
			}
		}
	}
	for (i = 0, ptr = v8 + (1 << layer); i < (1 << layer); i++, ptr++)
	{
		if (ptr->iptr == 0)
		{
		}
		else if (ptr->t == PORTTYPE) 
		{
			ports[i] = *ptr;
		} 
		else 
		{
			if(v8ptr[ptr->iptr][0].iptr != ports[i].iptr)
			{
				v8ptr[ptr->iptr][0] = ports[i];
				if(v8ptr[ptr->iptr][1].t != PORTTYPE)
				{
					insertNode(&head, makeNode(ptr->iptr, DELETE, ZLEVEL, 0, layer + 1));
				}
			}
			ports[i] = *ptr;
		}
	}

	TrieNode previous;
	previous.iptr = 0;

	for (i = 0; i < (1 << layer); i++)
	{
		if (previous.iptr == ports[i].iptr)
		{
			bitmask[i] = 0;
		}
		else
		{
			bitmask[i] = 1;
			previous.iptr = ports[i].iptr;
		}
	}
	//Chunk *chkptr = (Chunk *)malloc(sizeof(Chunk));
	Chunk *chkptr = ltb[current].chk;
	ushort ** base = chkptr->base;
	int GROUP_ = chkptr->groupWidth;
	int CLUSTER_ = layer - GROUP_ - BIT;
	for (i = 0; i < (1 << GROUP_); i++)
		free(base[i]);

	int group, cluster, bit;
	//chkptr->type = BITTYPE;
	//chkptr->groupWidth = width;
	//ushort **base = chkptr->base = (ushort **)malloc(sizeof(ushort *) * (1 << GROUP_));


	for (i = 0, group = 0; group < (1 << GROUP_); i += (1 << (CLUSTER_ + BIT)), group++)
	{
		bitmask[i] = 1;
		int cnt = 0;
		for (start = i; start < i + (1 << (CLUSTER_ + BIT)); start++)
			if (bitmask[start] == 1)
				cnt++;
		base[group] = (ushort *)malloc(sizeof(ushort) * ((1 << CLUSTER_) + cnt));

		/*for statistic*/
		bytes += sizeof(ushort) * ((1 << CLUSTER_) + cnt);
		/**********/

		CodeWord *cwd = (CodeWord *)(base[group]);
		TrieNode *lookup = (TrieNode *)(base[group] + (1 << CLUSTER_));
		int num8;
		int preOne = 0;
		int curOne = 0;
		for (cluster = 0; cluster < (1 << CLUSTER_); cluster++)
		{
			num8 = 0;
			curOne = 0;
			for (bit = i + (cluster << BIT); bit < i + ((cluster + 1) << BIT); bit++)
			{
				num8 <<= 1;
				if (bitmask[bit] == 1)
				{
					num8++;
					curOne++;
					*lookup = ports[bit];
					lookup++;
				}
			}
			cwd->before = preOne;
			cwd->bitmask = num8;
			cwd++;
			preOne += curOne;
		}
	}
	/*for statistic*/
	pointer_number += 1;
	free(bitmask);
	free(ports);

}

void modifyChunk(char * file)
{
	group_size = 0;
	now_one_number = 0;

	uint index = 0;
	uint ip = 0;
	ushort width = 0;
	int layer = 0;

	FILE *temp = fopen("mybitmap_eqix_lookup_update_result.txt","at");
	fprintf(temp,"%d\t%d\t",freq,freqshift);
	fclose(temp);

	FILE *fp = fopen(file, "r");
	if (fp == NULL)
	{
		perror("Open file...");
		//printf("err no: %d\n", errno);
		exit(1);
	}
	//while (inputUpdateChunk(fp, &ip, &layer, &width))
	while (inputUpdateChunk(fp, &index, &layer, &width))
	{
		if (layer == 2 || layer == 3)
		{
			//index = lookChunk(ip, layer);
			if (index != 0 && ltb[index].spe != 0 && ltb[index].chk->type == BITTYPE)
			{

//				if (ltb[index].chk->groupWidth != width)
//				{
					updateChunk(index, width, layer);
//				}
			}
		}
	}
}

int inputUpdateChunk(FILE * fp, uint * ipptr, int * layerptr, ushort * widthptr)
{
	int fre = 0;
	uint ip = 0;
	int layer = 0;
	int width = 0;
	if ((fscanf(fp, "%u\t/%d\t%d\n", &ip, &layer, &fre)) == 3) 
	{	
		width = 0;
		if (layer == 2)
		{
			//ip = ip << (32 - LAYER1);
			while (fre > freq && width < 3)
			{
				fre = fre >> freqshift;
				width++;
			}
			*ipptr = ip;
			*layerptr = layer;
			*widthptr = width;
		}
		else if (layer == 3)
		{
			while (fre > freq && width < 5)
			{
				fre = fre >> freqshift;
				width++;
			}
			//ip = ip << 8;
			*ipptr = ip;
			*layerptr = layer;
			*widthptr = width;
		}
		else
		{
			/*do not consider layer ==  1*/
		}
		return 1;
	}
	return 0;
}

int lookChunk(uint ip, int layer)
{
	ushort cur = 0;
	ushort group, cluster, bit;
	ushort * base;
	TrieNode *ptr, *node;
	CodeWord *cwd;
	Sparse * spe;
	Chunk * chk;
	int i;
	uint speip;

	node = ports16 + (ip >> (32 - LAYER1));
	if (node->t == PORTTYPE)
		return 0;
	cur = node->iptr;

	if (layer == 2)
		return cur;
	spe = ltb[cur].spe;
	if (spe->type == SPRTYPE) 
	{
		speip = (ip << LAYER1) >> (32 - LAYER2);
		Special *sptr = spe->special;
		for (i = 0; i < SPARSE; i++, sptr++) 
		{
			if (sptr->mask == 0u) 
			{
				return 0;
			} 
			else if ((speip & sptr->mask) == sptr->bits) 
			{
				if (spe->lookup[i].t == PORTTYPE)
					return 0;
				cur = spe->lookup[i].iptr;
				return cur;
			}
		}
		return 0;
	} 
	else 
	{
		chk = ltb[cur].chk;
		ushort GW = chk->groupWidth;
		ushort CW = LAYER2 - BIT - GW;
		uint mip = (ip << LAYER1) >> (32 - LAYER2);
		group = mip >> (CW + BIT);
		cluster = (mip << (32 - CW - BIT)) >> (32 - CW);
		bit = mip & 0x7;
		base = chk->base[group];
		cwd = (CodeWord *) base;
		ptr = (TrieNode *) (base + (1 << CW));
		node = ptr + cwd[cluster].before + bits8[cwd[cluster].bitmask >> (OFFSET - bit)] - 1;
		//node = ptr + cwd[cluster].before + popcnt(cwd[cluster].bitmask >> (OFFSET - bit)) - 1;
		if (node->t == PORTTYPE)
			return 0;
		cur = node->iptr;
		return cur;
	}
/*next:
	spe = ltb[cur].spe;
	if (spe->type == SPRTYPE) 
	{
		return 0;
	} 
	else 
	{
		return cur;
	}*/
}

void updateChunk(int index, ushort width, int layer)
{
	int i, j;
	TrieNode *ptr;
	TrieNode *v8 = trie.v8ptr[index];
	int start, end;
	int level;
	int layer_depth = layer == 2 ? LAYER2 : LAYER3;
	ushort *bitmask = (ushort *)malloc(sizeof(ushort) * (1 << layer_depth));
	TrieNode *ports = (TrieNode *)malloc(sizeof(TrieNode) * (1 << layer_depth));
	memset(bitmask, 0, sizeof(bitmask));

	if (v8[1].t == PORTTYPE)
	{
		ptr = &v8[1];
	}
	else
	{
		ptr = &v8[0];
	}

	for (i = 0; i < (1 << layer_depth); i++)
		ports[i] = *ptr;

	for (level = 1; level < layer_depth; level++)
	{
		i = (1 << level);
		j = i << 1;
		for (; i < j; i++)
		{
			if (v8[i].t == PORTTYPE)
			{
				start = (i << (layer_depth - level)) - (1 << layer_depth);
				end = ((i + 1) << (layer_depth - level)) - (1 << layer_depth);
				for (; start < end; start++)
				{
					ports[start] = v8[i];
				}
			}
		}
	}
	for (i = 0, ptr = v8 + (1 << layer_depth); i < (1 << layer_depth); i++, ptr++)
	{
		if (ptr->iptr == 0)
		{
		}
		else
		{
			ports[i] = *ptr;
		}
	}

	TrieNode previous;
	previous.iptr = 0;

	for (i = 0; i < (1 << layer_depth); i++)
	{
		if (previous.iptr == ports[i].iptr)
		{
			bitmask[i] = 0;
		}
		else
		{
			bitmask[i] = 1;
			previous.iptr = ports[i].iptr;
		}
	}
	//Chunk *chkptr = (Chunk *)malloc(sizeof(Chunk));
	int group, cluster, bit;
	int GROUP_ = width;
	int CLUSTER_ = layer_depth - GROUP_ - BIT;

	Chunk* chkptr = ltb[index].chk;
	ushort ** base_old = chkptr->base;
	ushort group_old = chkptr->groupWidth;
	for (i = 0; i < (1 << group_old); i++)
		free(base_old[i]);
	free(base_old);

	chkptr->type = BITTYPE;
	chkptr->groupWidth = width;
	ushort **base = chkptr->base = (ushort **)malloc(sizeof(ushort *) * (1 << GROUP_));

	for (i = 0, group = 0; group < (1 << GROUP_); i += (1 << (CLUSTER_ + BIT)), group++)
	{
		bitmask[i] = 1;
		int cnt = 0;
		for (start = i; start < i + (1 << (CLUSTER_ + BIT)); start++)
			if (bitmask[start] == 1)
				cnt++;
		base[group] = (ushort *)malloc(sizeof(ushort) * ((1 << CLUSTER_) + cnt));
		group_size += sizeof(ushort) * ((1 << CLUSTER_) + cnt);
		CodeWord *cwd = (CodeWord *)(base[group]);
		TrieNode *lookup = (TrieNode *)(base[group] + (1 << CLUSTER_));
		int num8;
		int preOne = 0;
		int curOne = 0;
		for (cluster = 0; cluster < (1 << CLUSTER_); cluster++)
		{
			num8 = 0;
			curOne = 0;
			for (bit = i + (cluster << BIT); bit < i + ((cluster + 1) << BIT); bit++)
			{
				num8 <<= 1;
				if (bitmask[bit] == 1)
				{
					num8++;
					curOne++;
					*lookup = ports[bit];
					lookup++;
				}
			}
			cwd->before = preOne;
			cwd->bitmask = num8;
			cwd++;
			preOne += curOne;
		}
		now_one_number += preOne;
	}
	free(bitmask);
	free(ports);
}


void announce(RouterEntry rentry)
{
	TrieNode * v16 = trie.v16;
	TrieNode ** v8ptr = trie.v8ptr;
	//printf("port is %d\n", map[rentry.port]);
	announceEntry(rentry, v16, v8ptr);
	disposeUpdate(v16, v8ptr);
}

void withdraw(RouterEntry rentry)
{
	TrieNode * v16 = trie.v16;
	TrieNode ** v8ptr = trie.v8ptr;
	//printf("port is %d\n", map[rentry.port]);
	withdrawEntry(rentry, trie.v16, trie.v8ptr);
	disposeUpdate(v16, v8ptr);
}

int updateLookup(RouterEntry * list, int max) {
	RouterEntry * p = list;
	initHead(&head);
	int all = 0;
	for (p = list; p < list + max; p++) {
		if (p->type == 0) {
			all += lookup(p->ipnet);
		}
		else if (p->type == 1) {
			announceEntry(*p, trie.v16, trie.v8ptr);
			disposeUpdate(trie.v16, trie.v8ptr);
		}
		else if (p->type == 2) {
			withdrawEntry(*p, trie.v16, trie.v8ptr);
			disposeUpdate(trie.v16, trie.v8ptr);
		}
	}
	return all;
}

