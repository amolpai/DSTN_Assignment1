#include <stdio.h>	
#include <stdlib.h>
#include <stdbool.h>
#define SETS 128
#define L2_BLOCK 32
#define WAYS 8
#define READ 0
#define WRITE 1

typedef struct {
	int tag;
	int index;
	int offset;
} l2_address;

typedef struct {
	bool valid;
	bool dirty;
	int tag;
	char data[L2_BLOCK+1];
} l2_entry;

typedef struct {
	l2_entry l2_entries[WAYS];
} l2_set;

typedef struct {
	l2_set l2_sets[SETS];
} l2_cache;

typedef struct {
	int lru_bits[WAYS];
} lru_set;

typedef struct {
	lru_set lru_sets[SETS];
} counter_lru;

