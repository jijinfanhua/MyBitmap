/*
 * @# bitType.h 2011-10-19
 * @Author: Mzaort
 */
#ifndef FIB_H_
#define FIB_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEFAULT ((1 << 30) - 1)
typedef unsigned short	ushort;
typedef unsigned int	uint;

typedef struct {
	uint ipnet;
	ushort port;
	ushort length : 8;
	ushort type : 8;
}RouterEntryRib;

typedef struct Node{
	uint nexthop;
	struct Node * left;
	struct Node * right;
}Node;

void makeWholeTrie(char * file);
uint lookupNode(uint ip);
void modifyWholeTrie(char * file);
void freeRoot();
#endif
