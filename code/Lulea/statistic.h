/*
 ============================================================================
 Name        : statistic.h
 Author      : Mzaort
 Date		 : 2011-11-1
 Version     : 1.0
 Copyright   : All rights reserved
 Description : In C, Ansi-style
 ============================================================================
*/
#include "bitType.h"

#ifndef STATISTIC_H_
#define STATISTIC_H_

typedef struct{
	ushort evl1;
	ushort evl2;
	ushort evl3;
}evlNum;

typedef struct{
	ushort sparse;
	ushort dense;
	ushort denseplus;
}denNum;

typedef struct{
	uint totalPointer;
	uint entryNumber;
	uint portNumber;
};

#endif /* STATISTIC_H_ */
