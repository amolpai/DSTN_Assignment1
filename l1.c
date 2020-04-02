#include "l1.h"
// #include "l2_cache.h"


void l1_space_remover(char *s);
void l1_fill(char *arr, int size, int value);
void l1_binary_representation_int(int n);
void l1_binary_representation_short(short n);
int l1_or_array(char *arr, int size);
void l1_row_initialize(L1_row *L1row);
void l1_initialize();
void l1_valid_tag_print ();
void l1_check_initialization();
short l1_physical_address_to_framebits(int physical_address);
int l1_physical_address_to_offset(int physical_address);
int l1_physical_address_to_index(int physical_address);
int l1_lru_victim_block(int index);
void l1_print_lru_bits(int index);
void l1_lru_update(int index, char wayPrediciton);
void l1_service_cache_miss(int physical_address, int index);
int l1_cache_hit_or_miss(short framebits, char wayPrediciton, int index);



int cacheHitCount = 0;
int cacheMissCount = 0;

L1 L1_instance; // L1 cache is global
char bus16B[L1_BLOCK]; //bus for getting data from l2
char processor_to_l1_bus;


void l2_write_from_l1_to_l2(int physical_address)      	//defined in l2, here to just test
{
	return ;
}
void l2_read_to_l1(int physical_address)				//defined in l2.
{
	srand(time(0));
	for (int i=0; i<L1_BLOCK; i++)
		bus16B[i] = rand();
}


void l1_space_remover(char *s)
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



void l1_binary_representation_int(int n)
{
	unsigned int x = 0b10000000000000000000000000000000;
	for (int i=0; i<32; i++)
	{
		printf("%d", (n&x)?1:0);
		x = x>>1;
	}
	printf("\n");
}


void l1_binary_representation_short(short n)
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


void l1_row_initialize(L1_row *L1row)
{
	l1_fill((char *)L1row, sizeof(*L1row),0);
}

void l1_initialize()
{
	for (int i =0; i<ENTRIES; i++)
	{
		l1_row_initialize(&(L1_instance.L1row[i]));
	}
}


void l1_valid_tag_print ()
{
	for (int i=0; i<ENTRIES; i++)
	{
		printf(" : %d", L1_instance.L1row[i].valid_Tag[0]);
		printf(" : %d", L1_instance.L1row[i].valid_Tag[1]);
		printf(" : %d", L1_instance.L1row[i].valid_Tag[2]);
		printf(" : %d\n", L1_instance.L1row[i].valid_Tag[3]);
	}
}

void l1_check_initialization()
{
	for (int i=0; i<ENTRIES; i++)
	{
		printf("%d\n", l1_or_array((char *)&(L1_instance.L1row[i]), sizeof(L1_instance.L1row[i])));
	}
}

short l1_physical_address_to_framebits(int physical_address)
{
	short framebits = physical_address >> 10;
	return framebits;
}

int l1_physical_address_to_offset(int physical_address)
{
	return (physical_address & 0b00000000000000000000000000001111);
}

int l1_physical_address_to_index(int physical_address)
{
	return ((physical_address & 0b00000000000000000000001111110000) >> 4);
}


int l1_lru_victim_block(int index)
{
	short lruBits = L1_instance.L1row[index].LRUBits;
	for (int i=0; i<4; i++){
		if ((L1_instance.L1row[index].valid_Tag[i] & 0b1000000000000000) == 0)  //if valid bit is zero, use this
			return i;
	}
	if ((lruBits & 0b1111000000000000) == 0)
		return 0;
	else if ((lruBits & 0b0000111100000000) == 0)
		return 1;
	else if ((lruBits & 0b0000000011110000) == 0)
		return 2;
	else 
		return 3;
}

void l1_print_lru_bits(int index)
{
	l1_binary_representation_short(L1_instance.L1row[index].LRUBits);
}

void l1_lru_update(int index, char wayPrediciton)  //on hit
{
	if (DEBUG) 
	{
		// printf("wayPrediciton : %d\n", wayPrediciton);
		printf("lruBits : before update\n");	
		l1_print_lru_bits(index);
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
		l1_print_lru_bits(index);
	}
}



void l1_service_cache_miss(int physical_address, int index)
{
	short framebits = l1_physical_address_to_framebits(physical_address);
	framebits = framebits | 0b1000000000000000;
	l2_read_to_l1(physical_address);
	int victimBlock = l1_lru_victim_block
(index);
	if (DEBUG) printf("victimBlock : %d\n", victimBlock);
	L1_instance.L1row[index].valid_Tag[victimBlock] = framebits;
	for (int i =0; i<L1_BLOCK; i++)
	{
		L1_instance.L1row[index].data[L1_BLOCK*victimBlock + i] = bus16B[i];
	}
	L1_instance.L1row[index].wayPrediciton = victimBlock;
}



int l1_cache_hit_or_miss(short framebits, char wayPrediciton, int index)
{
	short valid_framebits = framebits | 0b1000000000000000;
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
	else if (valid_Tag == valid_framebits)
	{	
		l1_lru_update(index, wayPrediciton);
		return wayPrediciton; //hit
	}
	else return -2;  //cache miss
}


char l1_read_from_l1_to_processor(int physical_address, int index, int offset)
{
	short framebits = l1_physical_address_to_framebits(physical_address);
	int return_comparator;
	return_comparator = l1_cache_hit_or_miss(framebits, L1_instance.L1row[index].wayPrediciton, index);
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
	for (int i=1; i<=3; i++)
	{
		return_comparator = l1_cache_hit_or_miss(framebits, (L1_instance.L1row[index].wayPrediciton+i)%4, index);
		if (return_comparator >= 0)
		{
			// cache hit
			if (DEBUG) printf("Cache Hit after Way Prediction : %d\n", (L1_instance.L1row[index].wayPrediciton+i)%4);
			if (LOG) printf("Cache Hit\n");
			cacheHitCount++;
			return  L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset];
		}
	}
	// cache miss
	cacheMissCount++;
	if(DEBUG) printf("Complete Cache Miss\n");
	if (LOG) printf("Cache miss\n");
	l1_service_cache_miss(physical_address, index);
	return l1_read_from_l1_to_processor(physical_address, index, offset);
}




void l1_write_from_processor_to_l1(int physical_address, int index, int offset)
{
	short framebits = l1_physical_address_to_framebits(physical_address);
	int return_comparator;
	return_comparator = l1_cache_hit_or_miss(framebits, L1_instance.L1row[index].wayPrediciton, index);
	if (return_comparator >= 0)
	{
		// cache hit
		if (DEBUG) printf("Cache Hit on way Prediction : %d\n", L1_instance.L1row[index].wayPrediciton);
		if (LOG) printf("Cache Hit\n");
		cacheHitCount++;
		L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset] = processor_to_l1_bus;
		for (int j=0; j<L1_BLOCK; j++)
		{
			bus16B[j] = L1_instance.L1row[index].data[L1_BLOCK*return_comparator + j];
		}
		l2_write_from_l1_to_l2(physical_address);
		return ;
	}
	if (DEBUG) printf("Cache Miss on Way Prediction : %d\n",L1_instance.L1row[index].wayPrediciton);

	// checks done in parallel in hardware
	for (int i=1; i<=3; i++)
	{
		return_comparator = l1_cache_hit_or_miss(framebits, (L1_instance.L1row[index].wayPrediciton+i)%4, index);
		if (return_comparator >= 0)
		{
			// cache hit
			if (DEBUG) printf("Cache Hit after Way Prediction : %d\n", (L1_instance.L1row[index].wayPrediciton+i)%4);
			if (LOG) printf("Cache Hit\n");
			cacheHitCount++;
			L1_instance.L1row[index].data[L1_BLOCK*return_comparator + offset] = processor_to_l1_bus;
			for (int j=0; j<L1_BLOCK; j++)
			{
				bus16B[j] = L1_instance.L1row[index].data[L1_BLOCK*return_comparator + j];
			}
			l2_write_from_l1_to_l2(physical_address);
			return ;
		}
	}
	// cache miss
	cacheMissCount++;
	if(DEBUG) printf("Complete Cache Miss\n");
	if (LOG) printf("Cache miss\n");
	l1_service_cache_miss(physical_address, index);
	l1_write_from_processor_to_l1(physical_address, index, offset);
}



int main()   //main for testing
{
	
	printf("sizeof cache: %ld\n", sizeof(L1_instance));
	printf("sizeof row : %ld\n", sizeof(L1_instance.L1row[0]));
	l1_initialize();

	FILE * fp = fopen("VORTEX.txt", "r");
	char input[50];
	int cnt=1;
	while (fgets(input, 50, fp)!=NULL)
	{
		printf("%s", input);
		l1_space_remover(input);
		int virtual_address =strtol(input, NULL, 16);
		int phy_add = virtual_address & 0b00000001111111111111111111111111;
		int offset =  l1_physical_address_to_offset(phy_add);
		int index = l1_physical_address_to_index(phy_add);
		if(cnt)
			l1_read_from_l1_to_processor(phy_add, index, offset);
		else
			l1_write_from_processor_to_l1(phy_add, index, offset);
		cnt++;
		cnt=cnt%2;
		// printf("data = %d\n", l1_read_from_l1_to_processor(tlbaddress, index,offset));
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