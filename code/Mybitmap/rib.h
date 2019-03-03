/*
 * @# bitType.h 2011-10-19
 * @Author: Mzaort
 */
#ifndef FIB_H_
#define FIB_H_
#include "lulea.h"

typedef struct Node{
	uint nexthop;
	struct Node * left;
	struct Node * right;
}Node;

void makeWholeTrie(char * file);
uint lookupNode(uint ip);
void modifyWholeTrie(char * file);
void freeRoot();

void announceNode(RouterEntry rentry);
void withdrawNode(RouterEntry rentry);
int inputUpdateRib(RouterEntry * rentry, char * c, FILE *fp);
#endif
