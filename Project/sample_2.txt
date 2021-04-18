/*
	--------------------------- LZ77 COMPRESSION ---------------------------
	Returns the pair of (len, pos) as output.
	1. The search function always has a scope to be improved.
		Here I have used list search which is greedy in nature, so is efficient 
		in a lot of cases where we have repetetive prefixes
	2. I have used a table or say list that has pointers for symbols in input buffer
		to hold the position for them in the window for the character
		Hence, search function can quickly skip through the placeholders.
	3. Lists are DLL, to make the deletions of node faster using prev and next pointer arrays
	4. Decompression is even faster as one needs to extract symbols from window on the basis of (len, pos)

	ALGO :
	->	A prefix match length less than MIN_MATCH_LEN is given len = 1, and is independently encoded
		with the prefix bit.
	->	The match len between [MIN_MATCH_LEN, PAT_BUFSIZE] are mapped to codes from [0, PAT_BUFSIZE - MIN_MATCH_LEN]
	->	if match len == 1 ? do not omit position in search and advance a byte.
	------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utps.h"
#include "getbit.c"
#include "lzl.c"

// struct for storing filestamp
typedef struct {
	char algorithm[5];
	ulong file_size;
} file_stamp;


// struct to store the pairs of (len, pos)
typedef struct {
	uint pos, len;
} dpos_t;

// the decompressor's must also equal these values
/* 
	else program wasn't working as expected 
 	file contents were altered after decompression 
*/
#define WIN_BUFSIZE 4096
#define NUM_POS_BITS 12
#define WIN_MASK 4095

#define PAT_BUFSIZE 18
#define NUM_LEN_BITS 4
#define MIN_MATCH_LEN 3

dpos_t dpos;
uchar win_buf[WIN_BUFSIZE];
uchar pattern[PAT_BUFSIZE];

uint win_count = 0, pat_count = 0, buf_count = 0;

void copyright(void);
dpos_t search (uchar w[], uchar p[]);
dpos_t brutef_search (uchar w[], uchar p[]);
void put_codes(dpos_t *dpos);

int main(int argc, char *argv[]) {
	unsigned long in_file_len = 0, out_file_len = 0;
	float ratio = 0.0;
	unsigned int i;
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
		fprintf(stderr, "[USAGE] ./lz77 infile outfile \t__ for compression\n\t./lz77 --info \t\t__ for info\n\t./lz77 -i \t\t__ for info\n");
		return 0;
	}
	

	if ((getin = fopen( argv[1], "rb" )) == NULL ) {
		fprintf(stderr, "[ERROR] Cannot open input file.\n");
		return 0;
	}
	if ( (putout = fopen( argv[2], "wb" )) == NULL ) {
		fprintf(stderr, "[ERROR] Cannot open output file.\n");
		return 0;
	}
	init_put_buffer();

	fprintf(stderr, "\t\t-----------[Compression Algorithm working fine]-------------\n\n");

	fprintf(stderr, "\t\t[INPUT] Input File : %s\n\n", argv[1]);

	// display file length. 
	fseek( getin, 0, SEEK_END );
	in_file_len = ftell( getin );

	// Write the FILE STAMP. 
	rewind( putout );
	strcpy( fstamp.algorithm, "LZ77" );
	fstamp.file_size = in_file_len;
	fwrite( &fstamp, sizeof(file_stamp), 1, putout );

	 // start Compressing to output file. 
	fprintf(stderr, "\t\t\tCompressing Your file...\n\n");
	fprintf(stderr, "\t\t[OUTPUT] Output File: %s\n\n", argv[2]);

	 // initialize the table of pointers. 
	init_lzlist();

	 // initialize the sliding-window. 
	memset( win_buf, 32, WIN_BUFSIZE );

	 // initialize the search list. 
	for (i = 0; i < WIN_BUFSIZE; i++) {
		lzprev[i] = -1;
		lznxt[i] = -1;
		insert_lznode( win_buf[i], i );
	}

	 // make sure to rewind the input file 
	rewind(getin);

 	 // fill the pattern buffer. 
	buf_count = fread(pattern, 1, PAT_BUFSIZE, getin);

	 // initialize the input buffer. 
	init_get_buffer();

	 // compress 
	while(buf_count > 0) {// untill look-ahead buffer not empty 
		dpos = search( win_buf, pattern );
		 // encode window position or len codes. 
		put_codes(&dpos);
	}
	flush_put_buffer();
	fprintf(stderr, "\t\t[COMPLETED]\n\n");

	 // get outfile's size and get compression ratio. 
	out_file_len = ftell( putout );

	fprintf(stderr, "\t\t\t-----------[Results of Compression]-------------\n\n");
	fprintf(stderr, "Size of input file = %lu bytes\n", in_file_len );
	fprintf(stderr, "Size of output file = %lu bytes\n", out_file_len );

	ratio = (((float) in_file_len - (float) out_file_len) / (float) in_file_len ) * (float) 100;
	fprintf(stderr, "Compression ratio: %.2f%%\n", ratio);
	
	free_put_buffer();
	free_get_buffer();
	if (getin) 
		fclose( getin );
	if (putout) 
		fclose( putout );

	return 0;
}

void copyright( void ) {
	fprintf(stderr, "\n\n\t--------------------------- LZ77 COMPRESSION ---------------------------\nReturns the pair of (len, pos) as output.\n1. The search function always has a scope to be improved.\n    Here I have used list search which is greedy in nature, so is efficient\n    in a lot of cases where we have repetetive prefixes\n2. I have used a table or say list that has pointers for symbols in input buffer\n    to hold the position for them in the window for the character\n    Hence, search function can quickly skip through the placeholders.\n3. Lists are DLL, to make the deletions of node faster using prev and next pointer arrays\n4. Decompression is even faster as one needs to extract symbols from window on the basis of (len, pos)\n\nALGO :\n->	A prefix match length less than MIN_MATCH_LEN is given len = 1, and is independently encoded\n   with the prefix bit.\n->	The match len between [MIN_MATCH_LEN, PAT_BUFSIZE] are mapped to codes from [0, PAT_BUFSIZE - MIN_MATCH_LEN]\n->	if match len == 1 ? do not omit position in search and advance a byte.\n------------------------------------------------------------------------\n");
	return;
}

dpos_t search(uchar w[], uchar p[]) {
	register unsigned int i, j, k;
	dpos_t dpos = {0, 1};

	 // point to start of lzlist[c] 
	i = lzlist[(uchar)p[pat_count]]; // start window search here.. 
	if (buf_count > 1) 
		while( i != -1 ) {
		 	// start search at the second characters. 
			if ((j = pat_count + 1) == PAT_BUFSIZE) 
				j = 0;
			k = 1;
			do {
				if (p[j] != w[(i+k) & WIN_MASK]) {
					break;
				}
				 // rotate if necessary. 
				if ((++j) == PAT_BUFSIZE)
					j = 0;
			} while((++k) < buf_count);

			if (k > dpos.len) {
				 // if greater than previous length, record it. 
				dpos.pos = i;
				dpos.len = k;

				 // maximum match, end the search. 
				if ( k == buf_count ) break;
		}
		 // point to next occurrence of the first character. 
		i = lznxt[i];
	}

	if (dpos.len >= MIN_MATCH_LEN) 
		buf_count -= dpos.len;
	else 
		buf_count -= 1;

	return dpos;
}

// brute_force_search -- use if you don't use lzl implementation and array implementation

/*
dpos_t brutef_search(uchar w[], uchar p[]){
	register int i, j, k;
	dpos_t dpos = {0, 0};

	
	// brute-force search.
	// start reading at i = 0.
	
	for (i = 0; i < WIN_BUFSIZE; i++){
		j = pat_count;
		k = 0;
		do {
			if(p[j] != w[(i+k) & WIN_MASK]) {
				break;
			}
			 // rotate if necessary. 
			if ((++j) == PAT_BUFSIZE) 
				j = 0;
		} while((++k) < buf_count);

		if ( k > dpos.len ) {
			 // if greater than previous length, record it. 
			dpos.pos = i;
			dpos.len = k;

			 // maximum match, end the search. 
			if (k == buf_count) 
				break;
		}
	}

	if (dpos.len >= MIN_MATCH_LEN)
		buf_count -= dpos.len;
	else 
		buf_count -= 1;

	return dpos;
}
*/
void put_codes(dpos_t *dpos){
	int i, j, k;

	 // a match length < MIN_MATCH_LEN gets a length of 1. 
	if ( dpos->len < MIN_MATCH_LEN ) {
		dpos->len = 1;
		put_one();  
	}
	else {
		put_zero();
		k = dpos->len;
		k -= MIN_MATCH_LEN;
		 
		// output length code. 
		put_nbits( k, NUM_LEN_BITS );
	}

	 // encode position for match len >= MIN_MATCH_LEN. 
	if (dpos->len >= MIN_MATCH_LEN) {
		k = (unsigned int) dpos->pos;
		put_nbits(k, NUM_POS_BITS);
	}
	else if (dpos->len == 1) {
		
		// but if len < MIN_MATCH_LEN, do not encode window position,
		// just the byte.
		
		k = (unsigned char)pattern[pat_count];
		put_nbits(k, 8);
	}

	 // if its a match, then move the buffer. 
	j = win_count;
	for (i = 0; i < (dpos->len); i++, j++) {
		 // first remove this position from its character list. 
		delete_lznode( *(win_buf + (j & WIN_MASK)), j & WIN_MASK );

		 // write the character to the window buffer. 
		*(win_buf + (j & WIN_MASK)) = *(pattern + ((pat_count+i) % PAT_BUFSIZE));

		 // insert this position to the new character's list. 
		insert_lznode(*(win_buf + (j & WIN_MASK)), j & WIN_MASK);
	}

	 // get dpos.len bytes 
	for (i = 0; i < (dpos->len); i++){
		if((k=gfgetc()) != EOF) {
			*(pattern + ((pat_count + i) % PAT_BUFSIZE)) =(uchar)k;
		}
		else 
			break; 
	}

	 // update counters. 
	buf_count += i;
	win_count = (win_count+dpos->len) & WIN_MASK;
	if ((pat_count = (pat_count + dpos->len)) >= PAT_BUFSIZE) {
		pat_count -= PAT_BUFSIZE;
	}
	return;
}
