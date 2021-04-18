#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utps.h"
#include "getbit.c"

typedef struct {
	char algorithm[5];
	ulong file_size;
} file_stamp;

typedef struct {
	uint pos, len;
} dpos_t;

#define WIN_BUFSIZE 4096
#define NUM_POS_BITS 12
#define WIN_MASK 4095

#define PAT_BUFSIZE 18
#define NUM_LEN_BITS 4
#define MIN_MATCH_LEN 3

dpos_t dpos;
uchar win_buf[WIN_BUFSIZE];
uchar pattern[PAT_BUFSIZE];
uint win_count = 0;

void copyright(void);

int main(int argc, char *argv[] ){
	unsigned long fsize=0;
	uint i, k;
	FILE *out;
	file_stamp fstamp;

	if (argc != 3 || argc == 1) {
		if(argc != 1  && strcmp(argv[1], "--info") == 0) {
			copyright();
			return 0;
		}	 
		if( argc != 1 && strcmp(argv[1], "-i") == 0 ) {
			copyright();
			return 0;
		}
		fprintf(stderr, "[ERROR] Looks like we encountered an error :( \n");
		fprintf(stderr, "[USAGE] ./lz77d infile outfile \t__ for decompression\n\t./lz77d --info \t\t__ for info\n\t./lz77d -i \t\t__ for info\n");
		return 0;
	}


	if ( (getin = fopen( argv[1], "rb" )) == NULL ) {
		fprintf(stderr, "[ERROR] Cannot open the input file\n");
		return 0;
	}
	fread( &fstamp, sizeof(file_stamp), 1, getin );
	init_get_buffer();

	if ( (out = fopen( argv[2], "wb" )) == NULL ) {
		fprintf(stderr, "[ERROR] Cannot open the output file\n");
		return 0;
	}

	fprintf(stderr, "Name of input  file : %s\n", argv[1] );
	fprintf(stderr, "Name of output file : %s\n", argv[2] );

	fprintf(stderr, "Decompressing...\n");

	/* initialize the sliding-window. */
	memset( win_buf, 32, WIN_BUFSIZE );

	fsize = fstamp.file_size;
	while( fsize ) {
			switch ( get_bit() ){
				case 0:
					 // get length. 
					dpos.len = get_nbits( NUM_LEN_BITS );

					 // get position. 
					dpos.pos = get_nbits( NUM_POS_BITS );

					 // map true values: 0 to 3, 1 to 4, and so on. 
					dpos.len += MIN_MATCH_LEN;
					
					 // if its a match, then "slide" the window buffer. 
					i = dpos.len;
					while (i--) {
						 // copy byte. 
						pattern[i] = win_buf[ (dpos.pos+i) & WIN_MASK ];
					}
					i = dpos.len;
					while ( i-- ) {
						win_buf[ (win_count+i) & WIN_MASK ] = pattern[ i ];
					}
					
					 // output string. 
					fwrite( pattern, dpos.len, 1, out );
					fsize -= dpos.len;
					win_count = (win_count + dpos.len) & WIN_MASK;
					break;
				
				case 1:
					k = get_nbits( 8 );
					fputc((unsigned char)k, out);
					win_buf[win_count] = (unsigned char)k;
					if ((++win_count) == WIN_BUFSIZE) 
						win_count = 0;
					fsize--;
					break;
			}
	}
	fprintf(stderr, "\n[COMPLETE]\n");
	free_get_buffer();
	if(getin) 
		fclose(getin);
	if(out)
		fclose(out);

	return 0;
}

void copyright( void ) {
	fprintf(stderr, "\n\n\t--------------------------- LZ77 COMPRESSION ---------------------------\nReturns the pair of (len, pos) as output.\n1. The search function always has a scope to be improved.\n    Here I have used list search which is greedy in nature, so is efficient\n    in a lot of cases where we have repetetive prefixes\n2. I have used a table or say list that has pointers for symbols in input buffer\n    to hold the position for them in the window for the character\n    Hence, search function can quickly skip through the placeholders.\n3. Lists are DLL, to make the deletions of node faster using prev and next pointer arrays\n4. Decompression is even faster as one needs to extract symbols from window on the basis of (len, pos)\n\nALGO :\n->	A prefix match length less than MIN_MATCH_LEN is given len = 1, and is independently encoded\n   with the prefix bit.\n->	The match len between [MIN_MATCH_LEN, PAT_BUFSIZE] are mapped to codes from [0, PAT_BUFSIZE - MIN_MATCH_LEN]\n->	if match len == 1 ? do not omit position in search and advance a byte.\n------------------------------------------------------------------------\n");
	return;
}
