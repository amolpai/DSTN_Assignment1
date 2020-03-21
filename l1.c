#include "l1.h"

int cacheHitCount = 0;
int cacheMissCount = 0;

L1 L1_instance; // L1 cache is global
char bus16B[L1_BLOCK]; //bus for getting data from l2

short l1_getAddressFromTLB() //getting address from TLB
{
	return 0;
}

void l1_spaceRemover(char *s)
{
	char *d = s;
	while (*d!=0)
	{
		if (*d==' ')
		{
			d++;
			continue;
		}
		*s = *d;
		s++;
		d++;
	}
	*s = *d;
}


void l1_fill(char *arr, int size, int value)
{
	for (int i=0; i<size; i++)
	{
		*(arr+i) = value;	
	}
}


void l1_BinaryRepInt(int n)
{
	unsigned int x = 0b10000000000000000000000000000000;
	for (int i=0; i<32; i++)
	{
		printf("%d", (n&x)?1:0);
		x = x>>1;
	}
	printf("\n");
}


void l1_BinaryRepShort(short n)
{
	unsigned short x = 0b1000000000000000;
	for (int i=0; i<16; i++)
	{
		printf("%d", (n&x)?1:0);
		if (i%4==3)
			printf("\n");
		x = x>>1;
	}
}

int l1_or_array(char *arr, int size)
{	
	int ans = 0;
	for (int i=0; i<size; i++)
	{
		ans = ans | *(arr+i);
	}
	return ans;
}


void L1_row_initialize(L1_row *L1row)
{
	l1_fill((char *)L1row, sizeof(*L1row),0);
}

void L1_initialize()
{
	for (int i =0; i<ENTRIES; i++)
	{
		L1_row_initialize(&(L1_instance.L1row[i]));
	}
}


void l1_valid_TagPrint ()
{
	for (int i=0; i<ENTRIES; i++)
	{
		printf(" : %d", L1_instance.L1row[i].valid_Tag[0]);
		printf(" : %d", L1_instance.L1row[i].valid_Tag[1]);
		printf(" : %d", L1_instance.L1row[i].valid_Tag[2]);
		printf(" : %d\n", L1_instance.L1row[i].valid_Tag[3]);
	}
}

void l1_checkInitialization()
{
	for (int i=0; i<ENTRIES; i++)
	{
		printf("%d\n", l1_or_array((char *)&(L1_instance.L1row[i]), sizeof(L1_instance.L1row[i])));
	}
}

short l1_TLBtoFrame(int TLBaddress)
{
	short framebits = TLBaddress >> 10;
	return framebits;
}

int l1_LRU_victimBlock(int index)
{
	short lruBits = L1_instance.L1row[index].LRUBits;
	if ((lruBits & 0b1111000000000000) == 0)
		return 0;
	else if ((lruBits & 0b0000111100000000) == 0)
		return 1;
	else if ((lruBits & 0b0000000011110000) == 0)
		return 2;
	else 
		return 3;
}

void l1_printLRUBits(int index)
{
	l1_BinaryRepShort(L1_instance.L1row[index].LRUBits);
}

void l1_LRU_Update(int index, char wayPrediciton)  //on hit
{
	if (DEBUG) 
	{
		// printf("wayPrediciton : %d\n", wayPrediciton);
		printf("lruBits : before update\n");	
		l1_printLRUBits(index);
	}
	switch(wayPrediciton)
	{
		case 0:
			L1_instance.L1row[index].LRUBits |= 0b1111000000000000;
			L1_instance.L1row[index].LRUBits &= 0b0111011101110111;
			break;
		case 1:
			L1_instance.L1row[index].LRUBits |= 0b0000111100000000;
			L1_instance.L1row[index].LRUBits &= 0b1011101110111011;
			break;
		case 2:
			L1_instance.L1row[index].LRUBits |= 0b0000000011110000;
			L1_instance.L1row[index].LRUBits &= 0b1101110111011101;
			break;
		case 3:
			L1_instance.L1row[index].LRUBits |= 0b0000000000001111;
			L1_instance.L1row[index].LRUBits &= 0b1110111011101110;
			break;
	}
	if (DEBUG) 
	{
		printf("lruBits : after update\n");
		l1_printLRUBits(index);
	}
}

void l1_getBlockFromL2(int TLBaddress)
{
	srand(time(0));
	for (int i=0; i<L1_BLOCK; i++)
		bus16B[i] = rand();
}


void l1_cacheMiss(int TLBaddress, int index)
{
	short framebits = l1_TLBtoFrame(TLBaddress);
	framebits = framebits | 0b1000000000000000;
	l1_getBlockFromL2(TLBaddress);
	int victimBlock = l1_LRU_victimBlock(index);
	if (DEBUG) printf("victimBlock : %d\n", victimBlock);
	L1_instance.L1row[index].valid_Tag[victimBlock] = framebits;
	for (int i =0; i<L1_BLOCK; i++)
	{
		L1_instance.L1row[index].data[L1_BLOCK*victimBlock + i] = bus16B[i];
	}
	L1_instance.L1row[index].wayPrediciton = victimBlock;
}



int l1_comparator(int TLBaddress, char wayPrediciton, int index)
{
	short frameBits = l1_TLBtoFrame(TLBaddress);
	short valid_frameBits = frameBits | 0b1000000000000000;
	short valid_Tag;
	switch(wayPrediciton)
	{
		case 0:
			valid_Tag = L1_instance.L1row[index].valid_Tag[0];
			break;
		case 1:
			valid_Tag = L1_instance.L1row[index].valid_Tag[1];
			break;
		case 2:
			valid_Tag = L1_instance.L1row[index].valid_Tag[2];
			break;
		case 3:
			valid_Tag = L1_instance.L1row[index].valid_Tag[3];
			break;
		default:
			printf("Invalid wayPrediciton Number.\n");
	}
	if ((valid_Tag & 0b1000000000000000) == 0)
		return -1; //invalid 
	else if (valid_Tag == valid_frameBits)
	{	
		l1_LRU_Update(index, wayPrediciton);
		return wayPrediciton; //hit
	}
	else return -2;  //cache miss
}


char l1_runner(int TLBaddress, int index, int offset)
{
	int return_comparator;
	return_comparator = l1_comparator(TLBaddress, 	L1_instance.L1row[index].wayPrediciton, index);
	if (return_comparator >= 0)
	{
		// cache hit
		if (DEBUG) printf("Cache Hit on way Prediction : %d\n", L1_instance.L1row[index].wayPrediciton);
		if (LOG) printf("Cache Hit\n");
		cacheHitCount++;
		return  L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset];
	}
	if (DEBUG) printf("Cache Miss on Way Prediction : %d\n",L1_instance.L1row[index].wayPrediciton);

	// checks done in parallel in hardware
	return_comparator = l1_comparator(TLBaddress, (L1_instance.L1row[index].wayPrediciton+1)%4, index);
	if (return_comparator >= 0)
	{
		// cache hit
		if (DEBUG) printf("Cache Hit after Way Prediction : %d\n", (L1_instance.L1row[index].wayPrediciton+1)%4);
		if (LOG) printf("Cache Hit\n");
		cacheHitCount++;
		return  L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset];
	}
	return_comparator = l1_comparator(TLBaddress, (L1_instance.L1row[index].wayPrediciton+2)%4, index);
	if (return_comparator >= 0)
	{
		// cache hit
		if (DEBUG) printf("Cache Hit after Way Prediction : %d\n", (L1_instance.L1row[index].wayPrediciton+2)%4);
		if (LOG) printf("Cache Hit\n");
		cacheHitCount++;
		return  L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset];
	}
	return_comparator = l1_comparator(TLBaddress, (L1_instance.L1row[index].wayPrediciton+3)%4, index);
	if (return_comparator >= 0)
	{
		// cache hit
		if (DEBUG) printf("Cache Hit after Way Prediction : %d\n", (L1_instance.L1row[index].wayPrediciton+3)%4);
		if (LOG) printf("Cache Hit\n");
		cacheHitCount++;
		return  L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset];
	}

	// cache miss
	cacheMissCount++;
	if(DEBUG) printf("Complete Cache Miss\n");
	if (LOG) printf("Cache miss\n");
	l1_cacheMiss(TLBaddress, index);
	l1_runner(TLBaddress, index, offset);
}



int main()   //main for testing
{
	
	printf("sizeof cache: %ld\n", sizeof(L1_instance));
	printf("sizeof row : %ld\n", sizeof(L1_instance.L1row[0]));
	L1_initialize();

	FILE * fp = fopen("VORTEX.txt", "r");
	char input[50];
	while (fgets(input, 50, fp)!=NULL)
	{
		printf("%s", input);
		l1_spaceRemover(input);
		int tlbaddress =strtol(input, NULL, 16); 
		// tlbaddress = tlbaddress & 0b00000001111111111111111111111111;
		// int tlbaddress =strtol(input, NULL, 2); 
		// printf("%d\n", tlbaddress);
		int offset = tlbaddress & 0b00000000000000000000000000001111; 
		int index = (tlbaddress & 0b00000000000000000000001111110000) >> 4;
		printf("data = %d\n", l1_runner(tlbaddress, index,offset));
	}
	// checkInitialization();
	printf("Total Cache Hits: %d\n", cacheHitCount);
	printf("Total Cache Miss: %d\n", cacheMissCount);
	printf("Hit ratio : %f\n", (float)(cacheHitCount)/(cacheHitCount + cacheMissCount));
}


// hit rates when run on address 
// APSI.txt
// Total Cache Hits: 54605
// Total Cache Miss: 2285
// Hit ratio : 0.959835


// CC1.txt
// Total Cache Hits: 223342
// Total Cache Miss: 7804
// Hit ratio : 0.966238

// LI.txt
// Total Cache Hits: 90587
// Total Cache Miss: 7550
// Hit ratio : 0.923067

// M88KSIM.txt
// Total Cache Hits: 30563
// Total Cache Miss: 8840
// Hit ratio : 0.775652

// VORTEX.txt
// Total Cache Hits: 12225
// Total Cache Miss: 1089
// Hit ratio : 0.918206