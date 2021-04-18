#include <stdio.h>
#include <stdlib.h>
#include "lzl.h"

// Using the external variables


int lzlist[LZ_MAX_SYM]; // arr of listheads.
int lzprev[LZ_MAX_BUFSIZE]; // arr containing the pointers to "prev" nodes.
int lznxt[LZ_MAX_BUFSIZE]; // arr containing the pointers to "next" nodes.

// initializing the arr of listheads to NULL(-1) 

void init_lzlist(void){
	unsigned int i;
	for(i = 0; i < LZ_MAX_SYM; i++) {
		lzlist[i] = LZ_NULL;
	}
	return;
}


// insertion in lzlist 
/*
	This function just appends the node to the the beginning of the list.
	This thing works because we have greedy search indexed by the characters,
	so it does not matter if we append the list head in ascending or descending
	sorted order
*/
void insert_lznode(unsigned char c, int i) {
	int k = lzlist[c];
	lzlist[c] = i;
	lzprev[i] = LZ_NULL;
	lznxt[i] = k;
	if (k != LZ_NULL) 
		lzprev[k] = i;
	return;
}

// deletion of the node
/*
	This deletion is faster as we need not traverse the list
	as we have pointers left to the nodes in the list respectively
*/ 

void delete_lznode(unsigned char c, int i) {
	if (lzlist[c] == i) { // the head of the list
		lzlist[c] = lznxt[i];
		if (lzlist[c] != LZ_NULL)  
			lzprev[lzlist[c]] = LZ_NULL;
	}
	else {
		lznxt[ lzprev[i] ] = lznxt[i];
		if (lznxt[i] != LZ_NULL)
			lzprev[lznxt[i]]= lzprev[i];
	}
	return;
}