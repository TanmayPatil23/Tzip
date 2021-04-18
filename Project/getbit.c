#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getbit.h"


// using hte extern variables

FILE *getin, *putout;
unsigned int put_bufsize = 8192, get_bufsize = 8192;
unsigned char *put_buf, *put_buf_start, put_count;
unsigned char *get_buf, *get_buf_start, *get_buf_end, get_count;
unsigned int put_buf_count, nfread;
unsigned int bit_read = 0, nbits_read = 0;
unsigned long nbytes_out;

// initialize sizes of the get and put buffers respectively
void init_buffer_sizes(unsigned int size){
	put_bufsize = get_bufsize = size;
	return;
}

// init put buffer
void init_put_buffer(void){
	put_count = 0;
	put_buf = NULL;
	put_buf_start = NULL;
	put_buf_count = 0;
	nbytes_out = 0;

	 // Allocate MEMORY for BUFFERS. 
	while(1) {
		put_buf = (unsigned char *) malloc( sizeof(char) * put_bufsize );
		if (put_buf) {
			put_buf_start = put_buf;
			break;
		}
		else {
			put_bufsize -= 1024;
			if (put_bufsize == 0) {
				fprintf(stderr, "\nmemory allocation error!");
				exit(0);
			}
		}
	}
	memset(put_buf, 0, put_bufsize);
	return;
}

// init get buffer
void init_get_buffer(void){
	get_buf = NULL;
	get_buf_start = NULL;
	get_buf_end = NULL;
	get_count = 0, nfread = 0;

	 // Allocate MEMORY for BUFFERS. 
	while(1) {
		get_buf = (unsigned char *) malloc( sizeof(char) * get_bufsize );
		if (get_buf) {
			get_buf_start = get_buf;
			break;
		}
		else {
			get_bufsize -= 1024;
			if ( get_bufsize == 0 ) {
				fprintf(stderr,"\nmemory allocation error!");
				exit(0);
			}
		}
	}
	nfread = fread (get_buf, 1, get_bufsize, getin);
	get_buf_end = (unsigned char *) (get_buf + nfread);
	return;
}


// free / reset put buffer
void free_put_buffer(void){
	put_buf = put_buf_start;
	if(put_buf) 
		free(put_buf);
	return;
}

// free / reset get buffer

void free_get_buffer(void){
	get_buf = get_buf_start;
	if(get_buf) 
		free(get_buf);
	return;
}


// flush / write the encoded codes into put buffer
void flush_put_buffer(void) {
	if (put_buf_count || put_count) {
		fwrite( put_buf_start, put_buf_count+(put_count ? 1 : 0), 1, putout);
		put_buf = put_buf_start; put_buf_count = 0; put_count = 0;
		nbytes_out = 0;
		memset(put_buf, 0, put_bufsize);
	}
	return;
}

// read bit of data from get buffer i.e. input buffer
int get_bit(void) {
	if(nfread){
		if ((*get_buf) & (1<<(get_count++))) 
			bit_read = 1;
		else 
			bit_read = 0;
		if (get_count == 8) { //finished 8 bits
			get_count = 0; 
			if((++get_buf) == get_buf_end) { //if end of buffer
				 // then fill buffer again. 
				get_buf = get_buf_start;
				nfread = fread ( get_buf, 1, get_bufsize, getin );
				get_buf_end = (unsigned char *) (get_buf + nfread);
			}
		}
	}
	else return EOF;
	return bit_read;
}

// gets a byte of data from input buffer
int gfgetc(void) {
	int c;
	// char x;
	// unsigned int c;
	
	if(nfread){
		// printf("%c\n", *get_buf);
		// x = *get_buf++;
		// c = (uint)*(get_buf);
		c = (int)(*get_buf++);
		if (get_buf == get_buf_end) {
			get_buf = get_buf_start;
			nfread = fread ( get_buf, 1, get_bufsize, getin );
			get_buf_end = (unsigned char *) (get_buf + nfread);
		}
		return c;
	}
	else 
		return EOF;
}

 // Puts a byte into the output buffer. 
void pfputc(int c) {
	*put_buf++ = (unsigned char)c;
	if ((++put_buf_count) == put_bufsize){
		fwrite( put_buf_start, put_bufsize, 1, putout );
		put_buf = put_buf_start;
		put_buf_count = 0;
		nbytes_out += put_bufsize;
		memset( put_buf, 0, put_bufsize );
	}
	return;
}

// get input of more than one bits at time
unsigned int get_nbits(int size) {
	unsigned int in_cnt = 0, k = (*get_buf) >> get_count;
	if ( size >= (8-get_count) ) { //past one byte
		size -= (8-get_count);
		in_cnt += (8-get_count);
		get_count = 0;
		if ( (++get_buf) == get_buf_end ) {  //end of buffer? 
			get_buf = get_buf_start;
			nfread = fread ( get_buf, 1, get_bufsize, getin );
			get_buf_end = (unsigned char *) (get_buf + nfread);
		}
		
		if (size) do {
			k |= ((*get_buf) << in_cnt);
			if ( size >= 8 ) {  //past one byte? 
				size -= 8;
				in_cnt += 8;
				if ( (++get_buf) == get_buf_end ) {
					get_buf = get_buf_start;
					nfread = fread ( get_buf, 1, get_bufsize, getin );
					get_buf_end = (unsigned char *) (get_buf + nfread);
				}
			}
			else 
				break;
		} while (size);
	}
	get_count += size;
	in_cnt += size;

	return (k << (INT_BIT-in_cnt)) >> (INT_BIT-in_cnt);
}

 // output more bits at a time; faster. 
void put_nbits( unsigned int k, int size ) {
	k = (k << (INT_BIT - size)) >> (INT_BIT - size);

	*put_buf |= (k<<(put_count));
	if ( size >= (8-put_count) ) {  //past one byte? 
		size -= (8-put_count);
		k >>= (8-put_count);
		put_count = 0;
		if ( (++put_buf_count) == put_bufsize ){
			fwrite( put_buf_start, put_bufsize, 1, putout );
			put_buf = put_buf_start;
			put_buf_count = 0;
			nbytes_out += put_bufsize;
			memset( put_buf, 0, put_bufsize );
		}
		else put_buf++;
		
		if ( size ) do {
			*put_buf |= k;
			if ( size >= 8 ) {  //past one byte? 
				size -= 8;
				k >>= 8;
				if ( (++put_buf_count) == put_bufsize ){
					fwrite( put_buf_start, put_bufsize, 1, putout );
					put_buf = put_buf_start;
					put_buf_count = 0;
					nbytes_out += put_bufsize;
					memset( put_buf, 0, put_bufsize );
				}
				else put_buf++;
			}
			else break;
		} while ( size );
	}
	put_count += size;
}

// reads a symbol of len = size i.e. of len = size bits
int get_symbol(int size){
	unsigned int in_cnt = 0, k = (*get_buf) >> get_count;
	if (nfread == 0) 
		return EOF;

	if ( size >= (8-get_count) ) { /* past one byte? */
		size -= (8-get_count);
		in_cnt += (8-get_count);
		get_count = 0;
		if ( (++get_buf) == get_buf_end ) { /* end of buffer? */
			get_buf = get_buf_start;
			nfread = fread ( get_buf, 1, get_bufsize, getin );
			get_buf_end = (unsigned char *) (get_buf + nfread);
			 // we still have some bits to read but no more bits
				// from the file; return end-of-file.
			
			if ( size > 0 && nfread == 0 ) {
				 // store the actual bits read. 
				nbits_read = (k << (INT_BIT-in_cnt)) >> (INT_BIT-in_cnt);
				 // use get_count to store the number of bits read. 
				get_count = in_cnt;
				return EOF;
			}
		}
		if ( size ) do {
			k |= ((*get_buf) << in_cnt);
			if ( size >= 8 ) { /* past one byte? */
				size -= 8;
				in_cnt += 8;
				if ( (++get_buf) == get_buf_end ) {
					get_buf = get_buf_start;
					nfread = fread ( get_buf, 1, get_bufsize, getin );
					get_buf_end = (unsigned char *) (get_buf + nfread);
					if ( size > 0 && nfread == 0 ) {
						 // store the actual bits read. 
						nbits_read = (k << (INT_BIT-in_cnt)) >> (INT_BIT-in_cnt);
						 // use get_count to store the number of bits read. 
						get_count = in_cnt;
						return EOF;
					}
				}
			}
			else break;
		} while ( size );
	}
	get_count += size;
	in_cnt += size;

	return (k << (INT_BIT-in_cnt)) >> (INT_BIT-in_cnt);
}

// returns total bytes to put into put buffer
unsigned long get_nbytes_out(void) {
	return (nbytes_out + put_buf_count);
}
