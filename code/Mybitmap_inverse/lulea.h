/*
 * @# bitType.h 2011-10-19
 * @Author: Mzaort
 */


#ifndef LULEA_H_
#define LULEA_H_

#define UPDATEMAX 10000000
#define TRACE_READ 10000000

#define GROUP 0
#define CLUSTER	5
#define GROUP2 0
#define CLUSTER2 3
#define BIT	3
#define MAX_CHUNK 18
#define DEFAULT 0
#define PORTTYPE 0xff
#define PORTLEN	8
#define MAXPORT	(1 << PORTLEN)
#define MAXCHUNKNUM ((1 << MAX_CHUNK) - (1 << PORTLEN)) //need modifying
#define OFFSET	((1 << BIT) - 1)
#define SPARSE	3
#define SPRTYPE		0xff
#define BITTYPE     0x01
#define ZLEVEL 9
/*********************/
#define INSERT 1
#define DELETE 2
#define HIGTH (CLUSTER + BIT)
#define BORDER1 (16 - CLUSTER - BIT)
#define BORDER2 (8 - CLUSTER - BIT)

#define LAYER1 18
#define LAYER2 6
#define LAYER3 8

#define popcnt(v) __builtin_popcountll(v)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *resultFile;

typedef unsigned short	ushort;
typedef unsigned int	uint;

typedef uint PortMap[MAXPORT];

typedef struct {
	char c;
	uint ipnet;
	ushort port;
	ushort length:8;
	ushort type:8;
}RouterEntry;

typedef union{
	ushort iptr;
	struct{
		ushort p: (PORTLEN);
		ushort t: (16 - PORTLEN);
	};
}TrieNode;

typedef struct{
	ushort bitmask:	(1 << BIT);
	ushort before:	(16 - (1 << BIT));
}CodeWord;

typedef TrieNode Chunk16[1 << 17];
typedef TrieNode Chunk8[1 << 9];
typedef TrieNode Chunk6[1 << 7];

typedef struct{
	ushort bits:8;
	ushort mask:8;
}Special;

typedef struct{
	ushort type: (16 - PORTLEN);
	ushort portfix:PORTLEN;
	Special	special[SPARSE];
	TrieNode lookup[SPARSE];
}Sparse;


typedef struct{
	ushort type : 8;
	ushort groupWidth: 8;
	ushort ** base;
}Chunk;

typedef union{
	Chunk * chk;
	Sparse * spe;
}LookupEntry;

typedef LookupEntry LookupTable[MAXCHUNKNUM];

/*******************************/
typedef struct UpdateNode{
	ushort	layer;
	ushort	level: 8;
	ushort	type:8;
	ushort	iptr;
	uint	index;
	struct UpdateNode * next;
}UpdateNode;
/*******************************/
inline unsigned long long GetCycleCount();

uint lookup(uint ip);
void start(char * file);
void freeMemory();
void update(char * file);

int input(RouterEntry * rentry, FILE * fp);
int inputUpdate(RouterEntry * rentry, char * c, FILE *fp);
int inputUpdate2(RouterEntry * rentry, char * c, FILE *fp);
ushort findMap(uint port);

/*********************************/

UpdateNode * makeNode(ushort iptr, ushort type, ushort level, uint index);
void initHead(UpdateNode * head);
void insertNode(UpdateNode * head, UpdateNode * e);
void removeCover(UpdateNode * head);
void withdrawEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v16ptr);
void announceEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v16ptr);
void disposeUpdate(TrieNode * v16, TrieNode ** v16ptr);
void modifyPorts16(TrieNode * v16, TrieNode ** v8ptr, UpdateNode * p);
void modifyltb(TrieNode ** v16ptr, ushort current, UpdateNode * p);
void freeUpdate(UpdateNode * p);
void modifyltb_node(TrieNode ** v16ptr, ushort current, UpdateNode * p, TrieNode node);
void modifyltb_special(TrieNode ** v8ptr, ushort current, UpdateNode * p, int len);
void modifyltb_spa2bit(TrieNode ** v16ptr, ushort current, ushort portfix, int cnt, Special * special, TrieNode * lookup, int layer);
void modifyltb_optimal(TrieNode ** v8ptr, ushort current, UpdateNode * p);
void modifyltb_refresh(TrieNode ** v16ptr, ushort current, int layer);
void modifyltb_new(TrieNode ** v16ptr, ushort current, UpdateNode * p);
void modifyChunk(char * file);
int inputUpdateChunk(FILE * fp, uint * ipptr, int * layerptr, ushort * widthptr);
int lookChunk(uint ip, int layer);
void updateChunk(int index, ushort width, int layer);
void overhead();
int updateLookup(RouterEntry * list, int max);

void announce(RouterEntry rentry);
void withdraw(RouterEntry rentry);
uint bit_inverse(TrieNode* ports, uint range);
#endif /* BITTYPE_H_ */




