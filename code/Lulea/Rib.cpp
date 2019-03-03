/*
 ============================================================================
 Name        : Lulea.c
 Author      : Mzaort
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "rib.h"
#include "global.h"

static Node root;


void freeNode(Node * node){
	if(!node){
		freeNode(node->left);
		freeNode(node->right);
		free(node);
		node = 0;
	}
}
void freeRoot(){
	freeNode(root.left);
	freeNode(root.right);
}

int inputRib(RouterEntryRib * rentry, FILE * fp) {
	uint u1, u2, u3, u4, u5, u6;
	ushort f8 = 0x00ff;

	rentry->ipnet = 0;
	if ((fscanf(fp, "%d.%d.%d.%d/%d\t%d", &u1, &u2, &u3, &u4, &u5, &u6)) == 6) {
		rentry->ipnet = ((u1 & f8) << 24) + ((u2 & f8) << 16)
				+ ((u3 & f8) << 8) + ((u4 & f8));
		rentry->length = u5 & f8;
		rentry->port = u6;
		return 1;
	}
	return 0;
}

int inputUpdateRib(RouterEntryRib * rentry, char * c, FILE *fp) {
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

void initRoot(){
	root.nexthop = DEFAULT;
	root.left = 0;
	root.right = 0;
}
void withdrawNode(RouterEntryRib rentry){
	Node *p;
	int i = 0;
	uint ip = rentry.ipnet;
	p = &root;
	int flag = 1;
	while(i < rentry.length){
		if(ip & 0x80000000){
			if(!p->right){
				flag = 0;
				break;
			}
			p = p->right;
		}else{
			if (!p->left) {
				flag = 0;
				break;
			}
			p = p->left;
		}
		ip <<= 1;
		i++;
	}
	if(flag){
		p->nexthop = 0;
	}

}

void addNode(RouterEntryRib rentry){
	Node *p;
	int i = 0;
	uint ip = rentry.ipnet;
	p = &root;
	while(i < rentry.length){
		if(ip & 0x80000000){
			if(!p->right){
				p->right = (Node *)calloc(1, sizeof(Node));
			}
			p = p->right;
		}else{
			if (!p->left) {
				p->left = (Node *) calloc(1, sizeof(Node));
			}
			p = p->left;
		}
		ip <<= 1;
		i++;
	}
	p->nexthop = rentry.port;
}

void announceNode(RouterEntryRib rentry){
	addNode(rentry);
}

void makeWholeTrie(char * file){
	initRoot();
	RouterEntryRib rentry;
	FILE *fp;
	fp = fopen(file, "r");
	if (fp == NULL) {
		perror("Open file...");
		//printf("err no: %d\n", errno);
		exit(1);
	}
	while (inputRib(&rentry, fp)) {
		addNode(rentry);
	}
	fclose(fp);
}
uint lookupNode(uint ip){
	Node *p;
	p = &root;
	uint port = 0;
	int i = 0;
	while(p){
		if(p->nexthop != 0){
			port = p->nexthop;
		}
		if (ip & 0x80000000) {
			if (!p->right) {
				break;
			}
			p = p->right;
		} else {
			if (!p->left) {
				break;
			}
			p = p->left;
		}
		ip <<= 1;
		i++;
	}
	return port;
}

void modifyWholeTrie(char * file){
	RouterEntryRib rentry;
	FILE * fp;
	char c;
	fp = fopen(file, "r");
	if (fp == NULL) {
		perror("Open file");
		//printf("err no: %d\n", errno);
		exit(1);
	}
	int j = 1;
	while (inputUpdateRib(&rentry, &c, fp) && j++ <= UPDATEMAX) {
		if (c == 'A') {
			announceNode(rentry);
		} else if (c == 'W') {
			withdrawNode(rentry);
		}
	}
	fclose(fp);
}

