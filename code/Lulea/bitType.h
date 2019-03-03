/*
 * @# bitType.h 2011-10-19
 * @Author: Mzaort
 */
#ifndef BITTYPE_H_
#define BITTYPE_H_
#define LENGTH 676
#define MAX_CHUNK 16

typedef unsigned short ushort;
typedef unsigned int uint;

typedef struct{
	uint p:30;
	uint t:2 ;
}TrieNode;

typedef TrieNode Level16[1<<17];
typedef TrieNode Level8[1<<9];

typedef struct {
	ushort inx :10;
	ushort offset : 6;
}uword;

typedef uword codeword16[1<<12];
typedef uword codeword8[1<<4];


typedef ushort baseindex16[1<<10];
typedef ushort baseindex8[1<<2];

typedef unsigned long long ulonglong;
typedef ulonglong maptable[LENGTH];

typedef struct {
	uint ipnet;
	uint port;
	ushort length;
}RouterEntry;

typedef struct {
	uword * codeword;
	ushort * baseindex;
	TrieNode * lookup;
}LookupEntry;

typedef LookupEntry LookupTalbe[1 << MAX_CHUNK];

void start(char * file);
int lookup(uint ipaddr);
void freeMem();
void update(char * file);

int inputUpdate2(RouterEntry * rentry, char * c, FILE *fp);
void announceEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v16ptr);
void withdrawEntry(RouterEntry rentry, TrieNode *v16, TrieNode **v16ptr);
#endif /* BITTYPE_H_ */
