#include <stdio.h>	
#include <stdlib.h>
#include <time.h>
#define ENTRIES 64
#define L1_BLOCK 16
#define DEBUG 0
#define LOG 1

typedef struct 
{
	short valid_Tag[4];
	char data[L1_BLOCK*4];

	short LRUBits;
	char wayPrediciton;

	// char padding;
}L1_row;

typedef struct
{
	L1_row L1row[ENTRIES];
} L1;
