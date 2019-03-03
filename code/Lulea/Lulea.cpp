#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "bitType.h"
#include "global.h"

static struct {
	TrieNode * v16;
	TrieNode ** v16ptr;
} trie;

TrieNode * v16_leaf = (TrieNode *)malloc(sizeof(Level16));
TrieNode ** v16ptr_leaf = (TrieNode **)malloc(sizeof(TrieNode *) * (1
	<< MAX_CHUNK));

FILE *fp;
//FILE *fpw = fopen("result.txt", "w");
maptable mt;
static LookupTalbe ltb;
static ushort ltbcsr = 0;
static ushort ltbcnt = 0;
static int cnt_one=0;
static int storage = 0;
ushort val[] = { 0, 128, 136, 138, 139, 142, 143, 168, 170, 171, 174, 175,
			184, 186, 187, 190, 191, 232, 234, 235, 238, 239, 248, 250, 251,
			254, 255 };

int triveMap(ushort n, ushort loc) {
	if (n == 676)
		return -1;
	else if (n == 677)
		return 0;
	else {
		ulong x = mt[n];
		x = (x << (loc * 4)) >> 60;
		return x;
	}
}

void writeTo(int k, ushort temp) {
	ulong result = 0;
	ushort count = 0;
	int i;
	ushort bit[16];
	for (i = 0; i < 16; i++) {
		bit[i] = temp & 1;
		temp = temp >> 1;
	}

	for (i = 14; i >= 0; i--) {
		count += bit[i];
		result = (result << 4) + count;
	}
	mt[k] = result;
}

void print() {
	int k;
	uint temp1, temp2;
	for (k = 0; k < 676; k++) {
		temp1 = mt[k] & (-1);
		temp2 = (mt[k] >> 32) & (-1);
		printf("%x,%x\n", temp2, temp1);
	}
}

void generate() {
	int i, j, k = 0;
	ushort temp;
	ushort map4[6] = { 0, 8, 10, 11, 14, 15 };
	int map8len = 27;
	ushort *map8 = (ushort*) malloc(sizeof(ushort) * map8len);
	ushort * m8ptr = map8 + 1;
	*m8ptr++ = 1 << 7;
	for (i = 1; i < 6; i++) {
		for (j = 1; j < 6; j++) {
			*m8ptr++ = (map4[i] << 4) + map4[j];
		}
	}
	memset(mt, 0, sizeof(maptable));
	for (i = 1; i < map8len; i++) {
		for (j = 1; j < map8len; j++) {
			temp = (map8[i] << 8) + map8[j];
			writeTo(k, temp);
			k++;
		}
	}
}

void openFile(char * c) {
	fp = fopen(c, "r");
	if (fp == NULL) {
		perror("Open file abcde");
		//printf("errno: %d\n", errno);
		exit(1);
	}
}
void traverse(TrieNode *v16, uint i, uint limit) {
	uint j = (i << 1);
	uint k;
	if (i < limit && v16[i].t != 0) {
		traverse(v16, j, limit);
		traverse(v16, j + 1, limit);
		if (j >= limit)
			return;
		switch (v16[i].t) {
		case 1:
			if (v16[j].t == 0 && v16[j + 1].t != 0) {
				v16[j].t = 1;
				v16[j].p = v16[i].p;
				v16[i].t = 3;
			} else if (v16[j].t != 0 && v16[j + 1].t == 0) {
				v16[j + 1].t = 1;
				v16[j + 1].p = v16[i].p;
				v16[i].t = 3;
			} else if (v16[j].t != 0 && v16[j + 1].t != 0) {
				v16[i].t = 3;
			}
			break;
		case 3:
			if (v16[j].t == 0 && v16[j + 1].t != 0) {
				k = i >> 1;
				while (v16[k].t != 1) {
					k >>= 1;
				}
				v16[j].t = 1;
				v16[j].p = v16[k].p;
			} else if (v16[j].t != 0 && v16[j + 1].t == 0) {
				k = i >> 1;
				while (v16[k].t != 1) {
					k >>= 1;
				}
				v16[j + 1].t = 1;
				v16[j + 1].p = v16[k].p;
			}
			break;
		default: {

		}
		}

	}
}

void completeTrie(TrieNode * v16, uint height) {
	int level, i, k;

	for (i = (1 << height); i < (1 << (height + 1)); i++) {
		if (v16[i].t == 1 || v16[i].t == 2) {
			k = i >> 1;
			while (k > 0 && v16[k].t == 0) {
				v16[k].t = 3;
				k >>= 1;
			}
		}
	}
	for (level = height - 1; level >= 0; level--) {
		for (i = (1 << level); i < (1 << (level + 1)); i++) {
			if (v16[i].t == 1) {
				k = i >> 1;
				while (k > 0 && v16[k].t == 0) {
					v16[k].t = 3;
					k >>= 1;
				}
			}
		}
	}

	traverse(v16, 1, (1 << (height + 1)));
}


uint findtoUpdate(TrieNode * v16, int temp) {
	while (v16[temp].t != 1) {
		temp >>= 1;
	}
	return v16[temp].p;
}

void makeTrie23(TrieNode * v16, TrieNode ** v16ptr) {
	int level, i, j, k, n;
	TrieNode * chunk;
	for (i = (1 << 8); i < (1 << 9); i++) {
		if (v16[i].t == 2) {
			chunk = v16ptr[v16[i].p];
			if (chunk[1].t == 0) {
				chunk[1].t = 1;
				chunk[1].p = findtoUpdate(v16, i >> 1);
			}
			makeTrie23(v16ptr[v16[i].p], v16ptr);
		}
	}
	completeTrie(v16, 8);
	for (level = 7; level >= 0; level--) {
		for (i = (1 << level); i < (1 << (level + 1)); i++) {
			if (v16[i].t == 1) {
				k = (i << (8 - level));
				n = k + (1 << (8 - level));
				v16[k].t = 1;
				v16[k].p = v16[i].p;
				for (j = k + 1; j < n; j++) {
					v16[j].t = 1;
					v16[j].p = 0;
				}
			}
		}
	}
}

void makeTrie(TrieNode * v16, TrieNode ** v16ptr) {
	int level, i, j, k, n;
	TrieNode * chunk;
	for (i = (1 << 16); i < (1 << 17); i++) {
		if (v16[i].t == 2) {
			chunk = v16ptr[v16[i].p];
			if (chunk[1].t == 0) {
				chunk[1].t = 1;
				chunk[1].p = findtoUpdate(v16, i >> 1);
			}
			makeTrie23(v16ptr[v16[i].p], v16ptr);
		}
	}
	completeTrie(v16, 16);
	for (level = 15; level >= 0; level--) {
		for (i = (1 << level); i < (1 << (level + 1)); i++) {
			if (v16[i].t == 1) {
				k = (i << (16 - level));
				n = k + (1 << (16 - level));
				v16[k].t = 1;
				v16[k].p = v16[i].p;
				for (j = k + 1; j < n; j++) {
					v16[j].t = 1;
					v16[j].p = 0;
				}
			}
		}
	}
}


ushort find(ushort k) {
	int low = 0, mid, high = 26;


	while (low <= high) {
		mid = (low + high) / 2;
		if (val[mid] == k)
			return mid;
		else if (val[mid] < k)
			low = mid + 1;
		else
			high = mid - 1;
	}
	return -1;
}

void put(uword * thd, int i, ushort f8, ushort e8) {
	ushort gf8 = find(f8);
	ushort ge8 = find(e8);

	if (gf8 == 0 && ge8 == 0)
		thd[i].inx = 676;
	else if (gf8 == 1 && ge8 == 0)
		thd[i].inx = 677;
	else
		thd[i].inx = (gf8 - 1) * 26 + (ge8 - 1);
}

ushort trive(TrieNode * v16, uword * thd, int j, int k) {
	ushort f8 = 0u, e8 = 0u;
	ushort x = 0u;
	int m;
	for (m = k; m < k + 8; m++) {
		f8 <<= 1;
		if ((v16[m].t == 1 && v16[m].p != 0) || (v16[m].t == 2)) {
			x++;
			f8++;
		}
	}
	for (m = k + 8; m < k + 16; m++) {
		e8 <<= 1;
		if ((v16[m].t == 1 && v16[m].p != 0) || (v16[m].t == 2)) {
			x++;
			e8++;
		}
	}
	put(thd, j, f8, e8);
	return x;
}

void makeLtb23(TrieNode * v16, TrieNode ** v16ptr) {
	int i, j, k;
	uint x, y, z, h;
	ushort leave = 0;
	int allOne = 0;
	int all16;
	ushort index;
	int the = ltbcsr;
	ltbcsr++;
	ushort * thx;
	uword * thd;
	TrieNode * ptr;
	TrieNode * tptr;

	for (ptr = v16 + (1 << 8); ptr < (v16 + (1 << 9)); ptr++) {
		if (ptr->t == 1 && ptr->p != 0)
			allOne++;
		else if (ptr->t == 2)
			allOne++;
	}
	cnt_one += allOne;
	if (allOne <= 8) {
		ltb[the].codeword = NULL;
		thx = ltb[the].baseindex = (ushort *) malloc(sizeof(ushort) * (allOne
				+ 1));
		thx[allOne] = 257;
		tptr = ltb[the].lookup = (TrieNode *) malloc(sizeof(TrieNode) * allOne);
		storage += sizeof(ushort) * (allOne + 1) + 2 * allOne;
		for (ptr = v16 + (1 << 8), index = 0u; ptr < (v16 + (1 << 9)); index++, ptr++) {
			if (ptr->t == 1 && ptr->p != 0) {
				*thx = index;
				thx++;
				tptr -> t = 1u;
				tptr -> p = ptr->p;
				tptr++;
			} else if (ptr->t == 2) {
				*thx = index;
				thx++;
				tptr -> t = 2u;
				tptr -> p = ltbcsr;
				tptr++;
				makeLtb23(v16ptr[ptr->p], v16ptr);
			}
		}
	} else if (allOne < 64) {
		leave = 0;
		ltb[the].baseindex = NULL;
		thd = ltb[the].codeword = (uword *) malloc(sizeof(codeword8));
		
		for (i = 0, k = (1 << 8); i < (1 << 4); i++, k += 16) {
			x = trive(v16, thd, i, k);
			thd[i].offset = leave;
			leave += x;
		}
		ltb[the].lookup = (TrieNode *) malloc(sizeof(TrieNode) * allOne);
		tptr = ltb[the].lookup;

		storage += sizeof(codeword8) + 2 * allOne;
		for (ptr = v16 + (1 << 8); ptr < (v16 + (1 << 9)); ptr++) {
			if (ptr->t == 1 && ptr->p != 0) {
				tptr->p = ptr->p;
				tptr->t = ptr->t;
				tptr++;
			} else if (ptr->t == 2) {
				tptr->p = ltbcsr;
				tptr->t = 2;
				tptr++;
				makeLtb23(v16ptr[ptr->p], v16ptr);
			}
		}
	} else {
		thd = ltb[the].codeword = (uword *) malloc(sizeof(codeword8));
		thx = ltb[the].baseindex = (ushort *) malloc(sizeof(baseindex8));

		for (i = 0, j = 0, k = (1 << 8); i < (1 << 2); i++, j += 4, k += 64) {
			x = trive(v16, thd, j, k);
			y = trive(v16, thd, j + 1, k + 16);
			z = trive(v16, thd, j + 2, k + 32);
			h = trive(v16, thd, j + 3, k + 48);

			thd[j].offset = 0;
			thd[j + 1].offset = thd[j].offset + x;
			thd[j + 2].offset = thd[j + 1].offset + y;
			thd[j + 3].offset = thd[j + 2].offset + z;
			all16 = x + y + z + h;
			thx[i] = leave;
			leave = all16 + thx[i];
		}

		tptr = ltb[the].lookup = (TrieNode *) malloc(sizeof(TrieNode) * allOne);

		storage += sizeof(codeword8) + sizeof(baseindex8) + 2 * allOne;
		for (ptr = v16 + (1 << 8); ptr < (v16 + (1 << 9)); ptr++) {
			if (ptr->t == 1 && ptr->p != 0) {
				tptr->p = ptr->p;
				tptr->t = ptr->t;
				tptr++;
			} else if (ptr->t == 2) {
				tptr->p = ltbcsr;
				tptr->t = 2;
				tptr++;
				makeLtb23(v16ptr[ptr->p], v16ptr);
			}
		}
	}
}
void makeLtb(TrieNode * v16, TrieNode ** v16ptr) {
	int i, j, k;
	ushort x, y, z, h;
	ushort leave = 0;
	int allOne = 0;
	int all16;
	int the = ltbcsr;
	TrieNode * ptr;
	TrieNode * tptr;
	uword * thd;
	ushort * thx;
	ltbcsr++;
	thd = ltb[the].codeword = (uword *) malloc(sizeof(codeword16));
	thx = ltb[the].baseindex = (ushort *) malloc(sizeof(baseindex16));

	for (i = 0, j = 0, k = (1 << 16); i < (1 << 10); i++, j += 4, k += 64) {
		x = trive(v16, thd, j, k);
		y = trive(v16, thd, j + 1, k + 16);
		z = trive(v16, thd, j + 2, k + 32);
		h = trive(v16, thd, j + 3, k + 48);

		thd[j].offset = 0;
		thd[j + 1].offset = thd[j].offset + x;
		thd[j + 2].offset = thd[j + 1].offset + y;
		thd[j + 3].offset = thd[j + 2].offset + z;
		all16 = x + y + z + h;
		thx[i] = leave;
		leave = all16 + thx[i];
		allOne += all16;
	}

	tptr = ltb[the].lookup = (TrieNode *) malloc(sizeof(TrieNode) * allOne);

	storage += sizeof(codeword8) + sizeof(baseindex8) + 2 * allOne;
	cnt_one += allOne;

	for (k = (1 << 16), ptr = &v16[k]; k < (1 << 17); k++, ptr++) {
		if (ptr->t == 1 && ptr->p != 0) {
			tptr->p = ptr->p;
			tptr->t = ptr->t;
			tptr++;
		} else if (ptr->t == 2) {
			tptr->p = ltbcsr;
			tptr->t = 2;
			tptr++;
			makeLtb23(v16ptr[ptr->p], v16ptr);
		}
	}

}

int input(RouterEntry * rentry) {
	uint u1, u2, u3, u4, u5, u6;
	ushort f8 = 0x00ff;
	rentry->ipnet = 0;
	if ((fscanf(fp, "%d.%d.%d.%d/%d\t%d", &u1, &u2, &u3, &u4, &u5, &u6)) == 6) {
		//printf("%d.%d.%d.%d/%d\t%d\n", u1, u2, u3, u4, u5, u6);
		rentry->ipnet = ((u1 & f8) << 24) + ((u2 & f8) << 16)
				+ ((u3 & f8) << 8) + ((u4 & f8));
		rentry->length = u5 & f8;
		rentry->port = u6;
		return 1;
	}
	return 0;
}
ushort search(ushort n, ushort * base) {
	//int i = 0;
	//ushort * p = base;
	//while (n >= *p) {
	//	i++;
	//	p++;
	//}
	//return i - 1;
	int i = 0;
	int loc;
	ushort * p;
	while (base[i] < 257) {
		i++;
	}
	loc = (i + 1) / 2;
	p = base + loc;
	if (n < *p) {
		loc = 0;
	}
	p = base + loc;
	while (n >= *p) {
		loc++;
		p++;

	}
	return loc - 1;
}

int lookup3(uint ipaddr, ushort cur) {
	ushort ip;
	uword code;
	ushort ix, bix, bit, ten, six, pter;
	int pix;
	ip = (ushort) ((ipaddr & 0xff));
	if (ltb[cur].codeword == NULL) {
		pter = search(ip, ltb[cur].baseindex);
		return ltb[cur].lookup[pter].p;
	} else if (ltb[cur].baseindex == NULL) {
		ix = ip >> 4;
		bit = ip & 0xf;
		code = ltb[cur].codeword[ix];
		ten = code.inx;
		six = code.offset;
		pix = six + triveMap(ten, bit);
		return ltb[cur].lookup[pix].p;

	} else {
		ix = ip >> 4;
		bit = ip & 0xf;
		bix = ip >> 6;
		code = ltb[cur].codeword[ix];
		ten = code.inx;
		six = code.offset;
		pix = ltb[cur].baseindex[bix] + six + triveMap(ten, bit);
		return ltb[cur].lookup[pix].p;
	}
}

int lookup2(uint ipaddr, ushort cur) {
	ushort ip;
	uword code;
	ushort ix, bix, bit, ten, six, pter;
	int pix;
	ip = (ushort) ((ipaddr & 0xff00) >> 8);
	if (ltb[cur].codeword == NULL) {
		pter = search(ip, ltb[cur].baseindex);
		if (ltb[cur].lookup[pter].t == 1)
			return ltb[cur].lookup[pter].p;
		else
			return lookup3(ipaddr, ltb[cur].lookup[pter].p);
	} else if (ltb[cur].baseindex == NULL) {
		ix = ip >> 4;
		bit = ip & 0xf;
		code = ltb[cur].codeword[ix];
		ten = code.inx;
		six = code.offset;
		pix = six + triveMap(ten, bit);
		if (ltb[cur].lookup[pix].t == 1)
			return ltb[cur].lookup[pix].p;
		else {
			return lookup3(ipaddr, ltb[cur].lookup[pix].p);
		}
	} else {
		ix = ip >> 4;
		bit = ip & 0xf;
		bix = ip >> 6;
		code = ltb[cur].codeword[ix];
		ten = code.inx;
		six = code.offset;
		pix = ltb[cur].baseindex[bix] + six + triveMap(ten, bit);
		if (ltb[cur].lookup[pix].t == 1)
			return ltb[cur].lookup[pix].p;
		else {
			return lookup3(ipaddr, ltb[cur].lookup[pix].p);
		}
	}
}

int lookup(uint ipaddr) {
	uint cur = 0;
	ushort ix, bix, bit, ten, six;
	int pix;
	uword code;
	ix = (ushort) (ipaddr >> 20);
	bix = (ushort) (ipaddr >> 22);
	bit = (ushort) ((ipaddr >> 16) & 0xf);
	code = ltb[cur].codeword[ix];
	ten = code.inx;
	six = code.offset;
	pix = ltb[cur].baseindex[bix] + six + triveMap(ten, bit);
	if (ltb[cur].lookup[pix].t == 1)
		return ltb[cur].lookup[pix].p;
	else {
		cur = ltb[cur].lookup[pix].p;
		return lookup2(ipaddr, cur);
	}
}
void freeLtb()
{
	for (int i = 0; i < (1 << MAX_CHUNK); i++)
	{
		free(ltb[i].codeword);
		free(ltb[i].baseindex);
		free(ltb[i].lookup);
	}
	ltbcsr = 0;
	memset(ltb, 0, sizeof(LookupEntry) * (1 << MAX_CHUNK));
}

void freeMemory(TrieNode * v16, TrieNode ** v16ptr) {
	TrieNode * ptr;
	TrieNode * ptr2;
	TrieNode * arr;
	for (ptr = v16 + (1 << 16); ptr < v16 + (1 << 17); ptr++) {
		if (ptr->t == 2) {
			arr = v16ptr[ptr->p];
			for (ptr2 = arr + (1 << 8); ptr2 < arr + (1 << 9); ptr2++) {
				if (ptr2->t == 2) {
					free(v16ptr[ptr2->p]);
				}
			}
			free(arr);
		}
	}
}

void copyMemory(TrieNode * v16_leaf, TrieNode ** v16ptr_leaf, TrieNode * v16, TrieNode ** v16ptr) {
	TrieNode * ptr;
	TrieNode * ptr2;
	TrieNode * arr;
	memcpy(v16_leaf, v16, sizeof(Level16));
	memcpy(v16ptr_leaf, v16ptr, sizeof(TrieNode *) * (1 << MAX_CHUNK));
	for (ptr = v16 + (1 << 16); ptr < v16 + (1 << 17); ptr++) {
		if (ptr->t == 2) {
			arr = v16ptr[ptr->p];
			v16ptr_leaf[ptr->p] = (TrieNode *)malloc(sizeof(Level8));
			if ((v16ptr_leaf[ptr->p]) == 0) printf("malloc error");
			memcpy(v16ptr_leaf[ptr->p], arr, sizeof(Level8));
			for (ptr2 = arr + (1 << 8); ptr2 < arr + (1 << 9); ptr2++) {
				if (ptr2->t == 2) {
					v16ptr_leaf[ptr2->p] = (TrieNode *)malloc(sizeof(Level8));
					memcpy(v16ptr_leaf[ptr2->p], v16ptr[ptr2->p], sizeof(Level8));
				}
			}
		}
	}
}


int construct(TrieNode * v16, TrieNode ** v16ptr, char * file) {
	RouterEntry rentry;
	int count = 0, ctemp;
	uint temp, temp2, temp3;
	openFile(file);
	memset(v16, 0, sizeof(Level16));
	v16[1].t = 1;
	v16[1].p = ((1 << 30) - 1);
	while (input(&rentry)) {
		//printf("%X/%d\t%d\n", rentry.ipnet, rentry.length, rentry.port);
		if (rentry.length < 16) {
			temp = (1 << rentry.length)
					+ (rentry.ipnet >> (32 - rentry.length));
			v16[temp].t = 1;
			v16[temp].p = rentry.port;
		} else if (rentry.length == 16) {
			temp = (1 << 16) + (rentry.ipnet >> 16);
			switch (v16[temp].t) {
			case 0:
				v16[temp].t = 1;
				v16[temp].p = rentry.port;
				break;
			case 1:
				v16[temp].p = rentry.port;
				break;
			default:
				v16ptr[v16[temp].p][1].t = 1;
				v16ptr[v16[temp].p][1].p = rentry.port;
			}
		} else if (rentry.length < 24) {
			temp = (1 << 16) + (rentry.ipnet >> 16);
			temp2 = (1 << (rentry.length - 16)) + ((rentry.ipnet & 0xffff)
					>> (32 - rentry.length));
			switch (v16[temp].t) {
			case 0: {
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16[temp].t = 2;
				v16[temp].p = count;
				v16ptr[count][temp2].t = 1;
				v16ptr[count][temp2].p = rentry.port;
				count++;
				break;
			}
			case 1: {
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16ptr[count][1].t = 1;
				v16ptr[count][1].p = v16[temp].p;
				v16[temp].t = 2;
				v16[temp].p = count;
				v16ptr[count][temp2].t = 1;
				v16ptr[count][temp2].p = rentry.port;
				count++;
				break;
			}
			default: {
				v16ptr[v16[temp].p][temp2].t = 1;
				v16ptr[v16[temp].p][temp2].p = rentry.port;
			}
			}
		} else if (rentry.length == 24) {
			temp = (1 << 16) + (rentry.ipnet >> 16);
			temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
			switch (v16[temp].t) {
			case 0: {
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16[temp].t = 2;
				v16[temp].p = count;
				v16ptr[count][temp2].t = 1;
				v16ptr[count][temp2].p = rentry.port;
				count++;
				break;
			}
			case 1: {
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16ptr[count][1].t = 1;
				v16ptr[count][1].p = v16[temp].p;
				v16[temp].t = 2;
				v16[temp].p = count;
				v16ptr[count][temp2].t = 1;
				v16ptr[count][temp2].p = rentry.port;
				count++;
				break;
			}
			default: {
				switch (v16ptr[v16[temp].p][temp2].t) {
				case 0: {
					v16ptr[v16[temp].p][temp2].t = 1;
					v16ptr[v16[temp].p][temp2].p = rentry.port;
					break;
				}
				case 1: {
					v16ptr[v16[temp].p][temp2].t = 1;
					v16ptr[v16[temp].p][temp2].p = rentry.port;
					break;
				}
				default: {
					v16ptr[v16ptr[v16[temp].p][temp2].p][1].t = 1;
					v16ptr[v16ptr[v16[temp].p][temp2].p][2].p = rentry.port; //???Ϊʲô��2 
				}
				}
			}
			}
		} else {
			temp = (1 << 16) + (rentry.ipnet >> 16);
			temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
			temp3 = (1 << (rentry.length - 24)) + ((rentry.ipnet & 0xff) >> (32
					- rentry.length));
			switch (v16[temp].t) {
			case 0: {
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16[temp].t = 2;
				v16[temp].p = count;
				ctemp = count;
				count++;
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16ptr[ctemp][temp2].t = 2;
				v16ptr[ctemp][temp2].p = count;
				v16ptr[count][temp3].t = 1;
				v16ptr[count][temp3].p = rentry.port;
				count++;
				break;
			}
			case 1: {
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16ptr[count][1].t = 1;
				v16ptr[count][1].p = v16[temp].p;
				v16[temp].t = 2;
				v16[temp].p = count;
				ctemp = count;
				count++;
				v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
				memset(v16ptr[count], 0, sizeof(Level8));
				v16ptr[ctemp][temp2].t = 2;
				v16ptr[ctemp][temp2].p = count;
				v16ptr[count][temp3].t = 1;
				v16ptr[count][temp3].p = rentry.port;
				count++;
				break;
			}
			default: {
				switch (v16ptr[v16[temp].p][temp2].t) {
				case 0: {
					v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
					memset(v16ptr[count], 0, sizeof(Level8));
					v16ptr[v16[temp].p][temp2].t = 2;
					v16ptr[v16[temp].p][temp2].p = count;
					v16ptr[count][temp3].t = 1;
					v16ptr[count][temp3].p = rentry.port;
					count++;
					break;
				}
				case 1: {
					v16ptr[count] = (TrieNode *) malloc(sizeof(Level8));
					memset(v16ptr[count], 0, sizeof(Level8));
					v16ptr[count][1].t = 1;
					v16ptr[count][1].p = v16ptr[v16[temp].p][temp2].p;
					v16ptr[v16[temp].p][temp2].t = 2;
					v16ptr[v16[temp].p][temp2].p = count;
					v16ptr[count][temp3].t = 1;
					v16ptr[count][temp3].p = rentry.port;
					count++;
					break;
				}
				default: {
					v16ptr[v16ptr[v16[temp].p][temp2].p][temp3].t = 1;
					v16ptr[v16ptr[v16[temp].p][temp2].p][temp3].p = rentry.port;
				}
				}
			}
			}
		}
	}
	return count;
}

void start(char * file) {
	trie.v16 = (TrieNode *) malloc(sizeof(Level16));
	trie.v16ptr = (TrieNode **) malloc(sizeof(TrieNode *) * (1<< MAX_CHUNK));

	generate();
	ltbcnt = construct(trie.v16, trie.v16ptr, file);
	printf("ltbcnt is :%d", ltbcnt);
	printf("Trie has been created....\n");

	copyMemory(v16_leaf, v16ptr_leaf, trie.v16, trie.v16ptr);
	makeTrie(v16_leaf, v16ptr_leaf);
	printf("comp has been created....\n");
	
	makeLtb(v16_leaf, v16ptr_leaf);
	printf("ltb has been created....\n");

	//fprintf(fpw,"%d \t %d \n",cnt_one, storage + sizeof(LookupEntry)*ltbcsr + sizeof(maptable));
	cnt_one = 0;
	storage = 0;
	//GetCycleCount();
	freeMemory(v16_leaf, v16ptr_leaf);
}

void freeMem()
{
	freeLtb();
	freeMemory(trie.v16, trie.v16ptr);
	//freeMemory(v16_leaf, v16ptr_leaf);
	free(v16ptr_leaf);
	free(v16_leaf);
	free(trie.v16ptr);
	free(trie.v16);
}

void update(char * file) {
	RouterEntry rentry;
	FILE * fp, *fpout;

	struct timeval start0, start1,start2, start3;
	double sum = 0;

	char c;
	int j;

	fp = fopen(file, "r");
	if (fp == NULL) {
		perror("Open file");
		//printf("err no: %d\n", errno);
		exit(1);
	}

	TrieNode ** v16ptr = trie.v16ptr;
	TrieNode * v16 = trie.v16;
	//TrieNode * v16_leaf = (TrieNode *)malloc(sizeof(Level16));
	//TrieNode ** v16ptr_leaf = (TrieNode **)malloc(sizeof(TrieNode *) * (1 << MAX_CHUNK));
	j = 0;
	//printf("%d::", j++);
	while (inputUpdate2(&rentry, &c, fp) && j++ < UPDATEMAX )
	{		
		freeLtb();
		memset(v16_leaf, 0, sizeof(Level16));
		memset(v16ptr_leaf, 0, sizeof(TrieNode*)*(1 << MAX_CHUNK));
		if (c == 'A') {
			gettimeofday(&start0, NULL);
			announceEntry(rentry, v16, v16ptr);
			gettimeofday(&start1, NULL);
			copyMemory(v16_leaf, v16ptr_leaf, v16, v16ptr);
			gettimeofday(&start2, NULL);
			makeTrie(v16_leaf, v16ptr_leaf);
			makeLtb(v16_leaf, v16ptr_leaf);
			gettimeofday(&start3, NULL);
			sum += (start3.tv_sec-start2.tv_sec + start1.tv_sec-start0.tv_sec)*1000000.0 +
					(start3.tv_usec-start2.tv_usec + start1.tv_usec-start0.tv_usec)*1.0;
		}
		if (c == 'W') {
			gettimeofday(&start0, NULL);
			withdrawEntry(rentry, v16, v16ptr);
			gettimeofday(&start1, NULL);
			copyMemory(v16_leaf, v16ptr_leaf, v16, v16ptr);
			gettimeofday(&start2, NULL);
			makeTrie(v16_leaf, v16ptr_leaf);
			makeLtb(v16_leaf, v16ptr_leaf);
			gettimeofday(&start3, NULL);
			sum += (start3.tv_sec-start2.tv_sec + start1.tv_sec-start0.tv_sec)*1000000.0 +
					(start3.tv_usec-start2.tv_usec + start1.tv_usec-start0.tv_usec)*1.0;
		}
		freeMemory(v16_leaf, v16ptr_leaf);
		//printf("%d\r", j);
	}
	double update_speed = j * 1.0 / (sum / 1000000) / 1000000;

	FILE * myfp = fopen("lulea.update.txt","at");
	fprintf(myfp,"org\t%lf\n",update_speed);
	fclose(myfp);

	printf("update_speed:%lf\n",update_speed);
	fclose(fp);
	//fclose(fpout);
}


void withdrawEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v16ptr) {
	uint temp, temp2, temp3;
	int ctemp;
	if (rentry.length < 16) {
		temp = (1 << rentry.length)
			+ (rentry.ipnet >> (32 - rentry.length));
		if (v16[temp].t == 1) {
			v16[temp].t = 0;
		}
	}
	else if (rentry.length == 16) {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		switch (v16[temp].t) {
		case 0:
			break;
		case 1:
			v16[temp].t = 0;
			break;
		default:
			if (v16ptr[v16[temp].p][1].t == 1) {
				v16ptr[v16[temp].p][1].t = 0;
			}
			
		}
	}
	else if (rentry.length < 24) {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		temp2 = (1 << (rentry.length - 16)) + ((rentry.ipnet & 0xffff)
			>> (32 - rentry.length));
		switch (v16[temp].t) {
		case 0: {
			break;
		}
		case 1: {
			break;
		}
		default: {
			if (v16ptr[v16[temp].p][temp2].t == 1) {
				v16ptr[v16[temp].p][temp2].p = 0;
			}
			
		}
		}
	}
	else if (rentry.length == 24) {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		switch (v16[temp].t) {
		case 0: {
			break;
		}
		case 1: {
			break;
		}
		default: {
			switch (v16ptr[v16[temp].p][temp2].t) {
			case 0: {
				break;
			}
			case 1: {
				v16ptr[v16[temp].p][temp2].t = 0;
				break;
			}
			default: {
				v16ptr[v16ptr[v16[temp].p][temp2].p][1].t = 0;
			}
			}
		}
		}
	}
	else {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp3 = (1 << (rentry.length - 24)) + ((rentry.ipnet & 0xff) >> (32
			- rentry.length));
		switch (v16[temp].t) {
		case 0: {
			break;
		}
		case 1: {
			break;
		}
		default: {
			switch (v16ptr[v16[temp].p][temp2].t) {
			case 0: {
				break;
			}
			case 1: {
				break;
			}
			default: {
				v16ptr[v16ptr[v16[temp].p][temp2].p][temp3].t = 0;
			}
			}
		}
		}
	}
}

void announceEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v16ptr) {
	uint temp, temp2, temp3;
	int ctemp;
	if (rentry.length < 16) {
		temp = (1 << rentry.length)
			+ (rentry.ipnet >> (32 - rentry.length));
		v16[temp].t = 1;
		v16[temp].p = rentry.port;
	}
	else if (rentry.length == 16) {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		switch (v16[temp].t) {
		case 0:
			v16[temp].t = 1;
			v16[temp].p = rentry.port;
			break;
		case 1:
			v16[temp].p = rentry.port;
			break;
		default:
			v16ptr[v16[temp].p][1].t = 1;
			v16ptr[v16[temp].p][1].p = rentry.port;
		}
	}
	else if (rentry.length < 24) {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		temp2 = (1 << (rentry.length - 16)) + ((rentry.ipnet & 0xffff)
			>> (32 - rentry.length));
		switch (v16[temp].t) {
		case 0: {
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16[temp].t = 2;
			v16[temp].p = ltbcnt;
			v16ptr[ltbcnt][temp2].t = 1;
			v16ptr[ltbcnt][temp2].p = rentry.port;
			ltbcnt++;
			break;
		}
		case 1: {
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16ptr[ltbcnt][1].t = 1;
			v16ptr[ltbcnt][1].p = v16[temp].p;
			v16[temp].t = 2;
			v16[temp].p = ltbcnt;
			v16ptr[ltbcnt][temp2].t = 1;
			v16ptr[ltbcnt][temp2].p = rentry.port;
			ltbcnt++;
			break;
		}
		default: {
			v16ptr[v16[temp].p][temp2].t = 1;
			v16ptr[v16[temp].p][temp2].p = rentry.port;
		}
		}
	}
	else if (rentry.length == 24) {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		switch (v16[temp].t) {
		case 0: {
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16[temp].t = 2;
			v16[temp].p = ltbcnt;
			v16ptr[ltbcnt][temp2].t = 1;
			v16ptr[ltbcnt][temp2].p = rentry.port;
			ltbcnt++;
			break;
		}
		case 1: {
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16ptr[ltbcnt][1].t = 1;
			v16ptr[ltbcnt][1].p = v16[temp].p;
			v16[temp].t = 2;
			v16[temp].p = ltbcnt;
			v16ptr[ltbcnt][temp2].t = 1;
			v16ptr[ltbcnt][temp2].p = rentry.port;
			ltbcnt++;
			break;
		}
		default: {
			switch (v16ptr[v16[temp].p][temp2].t) {
			case 0: {
				v16ptr[v16[temp].p][temp2].t = 1;
				v16ptr[v16[temp].p][temp2].p = rentry.port;
				break;
			}
			case 1: {
				v16ptr[v16[temp].p][temp2].t = 1;
				v16ptr[v16[temp].p][temp2].p = rentry.port;
				break;
			}
			default: {
				v16ptr[v16ptr[v16[temp].p][temp2].p][1].t = 1;
				v16ptr[v16ptr[v16[temp].p][temp2].p][1].p = rentry.port;
			}
			}
		}
		}
	}
	else {
		temp = (1 << 16) + (rentry.ipnet >> 16);
		temp2 = (1 << 8) + ((rentry.ipnet & 0xffff) >> 8);
		temp3 = (1 << (rentry.length - 24)) + ((rentry.ipnet & 0xff) >> (32
			- rentry.length));
		switch (v16[temp].t) {
		case 0: {
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16[temp].t = 2;
			v16[temp].p = ltbcnt;
			ctemp = ltbcnt;
			ltbcnt++;
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16ptr[ctemp][temp2].t = 2;
			v16ptr[ctemp][temp2].p = ltbcnt;
			v16ptr[ltbcnt][temp3].t = 1;
			v16ptr[ltbcnt][temp3].p = rentry.port;
			ltbcnt++;
			break;
		}
		case 1: {
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16ptr[ltbcnt][1].t = 1;
			v16ptr[ltbcnt][1].p = v16[temp].p;
			v16[temp].t = 2;
			v16[temp].p = ltbcnt;
			ctemp = ltbcnt;
			ltbcnt++;
			v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
			memset(v16ptr[ltbcnt], 0, sizeof(Level8));
			v16ptr[ctemp][temp2].t = 2;
			v16ptr[ctemp][temp2].p = ltbcnt;
			v16ptr[ltbcnt][temp3].t = 1;
			v16ptr[ltbcnt][temp3].p = rentry.port;
			ltbcnt++;
			break;
		}
		default: {
			switch (v16ptr[v16[temp].p][temp2].t) {
			case 0: {
				v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
				memset(v16ptr[ltbcnt], 0, sizeof(Level8));
				v16ptr[v16[temp].p][temp2].t = 2;
				v16ptr[v16[temp].p][temp2].p = ltbcnt;
				v16ptr[ltbcnt][temp3].t = 1;
				v16ptr[ltbcnt][temp3].p = rentry.port;
				ltbcnt++;
				break;
			}
			case 1: {
				v16ptr[ltbcnt] = (TrieNode *)malloc(sizeof(Level8));
				memset(v16ptr[ltbcnt], 0, sizeof(Level8));
				v16ptr[ltbcnt][1].t = 1;
				v16ptr[ltbcnt][1].p = v16ptr[v16[temp].p][temp2].p;
				v16ptr[v16[temp].p][temp2].t = 2;
				v16ptr[v16[temp].p][temp2].p = ltbcnt;
				v16ptr[ltbcnt][temp3].t = 1;
				v16ptr[ltbcnt][temp3].p = rentry.port;
				ltbcnt++;
				break;
			}
			default: {
				v16ptr[v16ptr[v16[temp].p][temp2].p][temp3].t = 1;
				v16ptr[v16ptr[v16[temp].p][temp2].p][temp3].p = rentry.port;
			}
			}
		}
		}
	}
}

int inputUpdate2(RouterEntry * rentry, char * c, FILE *fp) {
	uint t1, t2;
	char ch;
	uint u1, u2, u3, u4, u5, u6 = 0;
	ushort f8 = 0x00ff;
	rentry->ipnet = 0;
	if (fscanf(fp, "%c", &ch) == 1) {
		*c = ch;
		if (ch == 'A') {
			fscanf(fp, "%d.%d.%d.%d/%d\t%d\n", &u1, &u2, &u3, &u4, &u5, &u6);
			rentry->port = u6;
			//printf("A: %d.%d.%d.%d/%d\t%d\n", u1, u2, u3, u4, u5, u6);
			
		}
		else if (ch == 'W') {
			fscanf(fp, "%d.%d.%d.%d/%d\n", &u1, &u2, &u3, &u4, &u5);
			//printf("W: %d.%d.%d.%d/%d\n", u1, u2, u3, u4, u5);
		}
		else {
			return 1;
		}
		rentry->ipnet = ((u1 & f8) << 24) + ((u2 & f8) << 16)
			+ ((u3 & f8) << 8) + ((u4 & f8));
		rentry->length = u5 & f8;
		return 1;
	}
	return 0;
}
