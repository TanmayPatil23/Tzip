#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef GET_BIT_H
	#define GET_BIT_H

// Defining the INT_BIT according to the arch of machine.
/*
	Here the INT_BIT is number of bits in an integer : sizeof(int) * 8
	which can be 8, 16, 32, 64 according to the arch.
	This can be used to efficiently "get" or "put" atmost INT_BIT bits.
*/
#if !defined(INT_BIT)
	#if INT_MAX == 0x7fff
		#define INT_BIT 16
	#elif INT_MAX == 0x7fffffff
		#define INT_BIT 32
	#else
		#define INT_BIT (8 * sizeof(int))
	#endif
#endif


// sets bit i.e. sets bit to 1
#define pset_bit() *put_buf |= (1 << put_count)

 // writes a ONE (1) bit.  
#define put_one() { pset_bit(); advance_buf(); }

 // writes a ZERO (0) bit.  
#define put_zero() advance_buf()

 // just increment the put_buf buffer for faster processing. 
#define advance_buf()		\
{                          \
	if ( (++put_count) == 8 ) { \
		put_count = 0; \
		if ( (++put_buf_count) == put_bufsize ){ \
			put_buf = put_buf_start; \
			fwrite( put_buf, put_bufsize, 1, putout ); \
			memset( put_buf, 0, put_bufsize ); \
			put_buf_count = 0; \
			nbytes_out += put_bufsize; \
		} \
		else put_buf++; \
	} \
}


// extern varialbes to use int the .c files
extern FILE *getin, *putout;
extern unsigned int put_bufsize, get_bufsize;
extern unsigned char *put_buf, *put_buf_start, put_count;
extern unsigned char *get_buf, *get_buf_start, *get_buf_end, get_count;
extern unsigned int put_buf_count, nfread;
extern unsigned int bit_read, nbits_read;
extern unsigned long nbytes_out;

// function prototypes
void init_buffer_sizes(unsigned int size);
void init_put_buffer(void);
void init_get_buffer(void);
void free_put_buffer(void);
void free_get_buffer(void);
void flush_put_buffer(void);
int  get_bit(void);
int  gfgetc(void);
void pfputc(int c);
unsigned int get_nbits(int size);
void put_nbits(unsigned int k, int size);
int get_symbol(int size);
unsigned long get_nbytes_out(void);

#endif
