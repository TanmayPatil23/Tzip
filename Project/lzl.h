#include <stdio.h>
#include <stdlib.h>

#ifndef LZL_H
	#define LZL_H

#define LZ_NULL -1  
#define LZ_MAX_SYM 256 // max number of chars to encode
#define LZ_MAX_BUFSIZE 8192 // max buffersize for the linked list

extern int lzprev[];
extern int lznxt[];

// List list heads indexed by 256 characters
extern int lzlist[];

// function prototypes
void init_lzlist(void);
void insert_lznode(unsigned char c, int i);
void delete_lznode(unsigned char c, int i);

#endif
