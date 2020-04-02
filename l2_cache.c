#include "l2_cache.h"
#include "bus_structures.h"

l2_cache l2;
counter_lru lru_counter;

l2_address l2_get_l2_address_from_physical_address(int physical_address);
int l2_get_physical_address_from_l2_address(l2_address l2_addr);
int l2_get_victim_block(int set_index);
void l2_update_lru_counter(int set_index, int mru_block_index);
void l2_get_block_from_mm(l2_address addr);
void l2_write_dirty_block_back_to_mm(l2_address l2_addr, int victim_block);
int l2_service_cache_miss(l2_address l2_addr);
int l2_cache_hit_or_miss(l2_address l2_addr);

//Function to print cache state for all blocks corresponding to a given index(ie for a set)
void l2_print_l2_cache_set(int physical_address) {
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
	l2_set set = l2.l2_sets[l2_addr.index];
	printf("INDEX: 0x%x\n", l2_addr.index);
	printf("-----------------------------------------------------------------\n");
	printf("| VALID | DIRTY |   TAG    |\t\tDATA\t\t\t|\n");
	printf("-----------------------------------------------------------------\n");
	for(int i=0;i<WAYS;i++) {
		printf("|   %-1d   |   %-1d   | 0x%-5x  |   %-32s |\n", set.l2_entries[i].valid, set.l2_entries[i].dirty, 
				set.l2_entries[i].tag, set.l2_entries[i].data);
	}
	printf("-----------------------------------------------------------------\n\n");
}

//Function to print lru counter state for given set
void l2_print_lru_counter_for_set(int physical_address) {
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
	l2_set set = l2.l2_sets[l2_addr.index];
	
	printf("-----------------------------------------------------------------\n");
	printf("BLOCK: ");
	for(int i=0;i<WAYS;i++) 
		printf("%d ", i);
	
	printf("\nCOUNT: ");
	for(int i=0;i<WAYS;i++) 
		printf("%d ", lru_counter.lru_sets[l2_addr.index].lru_bits[i]);
	printf("\n-----------------------------------------------------------------\n\n");
}

//Function to convert physical address into l2_address
//l2_address: 20 bit tag, 7 bit index, 5 bit offset
l2_address l2_get_l2_address_from_physical_address(int physical_address) {
	l2_address l2_addr;
	l2_addr.offset = (physical_address & 0x001f);
	l2_addr.index = (physical_address & 0x0fe0) >> 5;
	l2_addr.tag = (physical_address & 0xfffff000) >> 12;
	
	return l2_addr;
}

//Function to convert l2_address into physical address
//l2_address: 20 bit tag, 7 bit index, 5 bit offset
int l2_get_physical_address_from_l2_address(l2_address l2_addr) {
	int physical_address = 0x0;
	physical_address |= l2_addr.offset;
	physical_address |= (l2_addr.index << 5);
	physical_address |= (l2_addr.tag << 12);
	
	return physical_address;
}

//Function to get victim block for replacement. First search for blocks with valid==0.
//If no such blocks are found, use lru_counter to get lru block
int l2_get_victim_block(int set_index) {
	
	for(int i=0;i<WAYS;i++) {
		if(l2.l2_sets[set_index].l2_entries[i].valid == 0)
			return i;
	}
	
	for(int i=0;i<WAYS;i++) {
		if(lru_counter.lru_sets[set_index].lru_bits[i] == 0)
			return i;
	}
	return -1; //Should ideally be unreachable since at least one counter entry will be 0 at all times
	
}

//Function to update lru counter given a set index and mru block index.
//Count corresponding to mru block index is set to 7, all other blocks 
//in the same get their count decremented by 1. If count<1, count is set
//to 0.
void l2_update_lru_counter(int set_index, int mru_block_index) {
	
	lru_counter.lru_sets[set_index].lru_bits[mru_block_index] = 7;
	
	for(int i=0;i<WAYS;i++) {
		if(i == mru_block_index)
			continue;
		
		if(lru_counter.lru_sets[set_index].lru_bits[i] > 1)
			lru_counter.lru_sets[set_index].lru_bits[i]--;
		else
			lru_counter.lru_sets[set_index].lru_bits[i]=0;
	}

	return;
}

//Function to prompt mm to write data corresponding to given physical address
//to 32B bus.
void l2_get_block_from_mm(l2_address addr) {
	int physical_address = l2_get_physical_address_from_l2_address(addr);
	//mm_write_to_l2(physical_address);MM writes block into bus32B
	
	return;
}

//Function to write victim block back to MM
void l2_write_dirty_block_back_to_mm(l2_address l2_addr, int victim_block) {
	int physical_address = l2_get_physical_address_from_l2_address(l2_addr);
	
	for(int i=0;i<L2_BLOCK;i++)
		bus32B[i] = l2.l2_sets[l2_addr.index].l2_entries[victim_block].data[i];
	//mm_write_to_mm(int physical_address, main_memory_struct **main_memory, char *data)
	l2.l2_sets[l2_addr.index].l2_entries[victim_block].dirty = 0;
}

//This function assigns a block in the cache. If the given block has modified
//data, the block is written back to MM. New data is obtained from MM and 
//LRU counter is updated to reflect this most recent access.
int l2_service_cache_miss(l2_address l2_addr) {
	int victim_block = l2_get_victim_block(l2_addr.index);
	
	if(l2.l2_sets[l2_addr.index].l2_entries[victim_block].dirty)
		l2_write_dirty_block_back_to_mm(l2_addr, victim_block);
	
	l2_get_block_from_mm(l2_addr);
	
	l2_entry new_entry;
	new_entry.valid = 1;
	new_entry.dirty = 0;
	new_entry.tag = l2_addr.tag;
	for(int i=0;i<L2_BLOCK;i++)
		new_entry.data[i] = bus32B[i];
	
	l2.l2_sets[l2_addr.index].l2_entries[victim_block] = new_entry;
	l2_update_lru_counter(l2_addr.index, victim_block);
	
	return victim_block;
}

//Checks if given block exists in L2_cache by comparing tags in all ways
//of the given index with the tag of the given address.
int l2_cache_hit_or_miss(l2_address l2_addr) {
	l2_set set_l2 = l2.l2_sets[l2_addr.index];
	int tag = l2_addr.tag;
	
	for(int i=0;i<WAYS;i++) {
		if(set_l2.l2_entries[i].valid == 1 && set_l2.l2_entries[i].tag == tag) {
			//L2 Cache hit
			l2_update_lru_counter(l2_addr.index, i);
			return i;
		}
	}
	return -1;
}

//Function to write from L1 to L2. L1 writes data to bus16B and that data
//is written into L2
void l2_write_from_l1_to_l2(int physical_address){
   
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
   
	int wayNo = l2_cache_hit_or_miss(l2_addr);
	if(wayNo == -1) {
		wayNo = l2_service_cache_miss(l2_addr);
	}
	
	l2.l2_sets[l2_addr.index].l2_entries[wayNo].dirty = 1;

	if(l2_addr.offset < L2_BLOCK/2) {
		for(int i=0;i<L2_BLOCK/2;i++)
			l2.l2_sets[l2_addr.index].l2_entries[wayNo].data[i] = bus16B[i]; 
	} else {
		for(int i=0;i<L2_BLOCK/2;i++)
			l2.l2_sets[l2_addr.index].l2_entries[wayNo].data[i+L2_BLOCK/2] = bus16B[i];
	}
    
   return;
}

//Function to read to L1. L2 returns data corresponding to given physical address.
void l2_read_to_l1(int physical_address) {
	
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
	
	int wayNo = l2_cache_hit_or_miss(l2_addr);
	if(wayNo == -1) {
		wayNo = l2_service_cache_miss(l2_addr);
	}
	
	//Transfer data from l2 to bus16B
	int offset = l2_addr.offset;
	if(offset < L2_BLOCK/2) {
		for(int i=0;i<L2_BLOCK/2;i++)
			bus16B[i] = l2.l2_sets[l2_addr.index].l2_entries[wayNo].data[i];
	} else {
		for(int i=L2_BLOCK/2;i<L2_BLOCK;i++)
			bus16B[i] = l2.l2_sets[l2_addr.index].l2_entries[wayNo].data[i];
	}
}

//Initialise L2 cache and LRU counter
void l2_initialize() {
	for(int i=0;i<SETS;i++) {
		for(int j=0;j<WAYS;j++) {
			l2.l2_sets[i].l2_entries[j].valid = 0;
			l2.l2_sets[i].l2_entries[j].dirty = 0;
			l2.l2_sets[i].l2_entries[j].tag = 0;
			
			for(int k=0;k<L2_BLOCK;k++)
				l2.l2_sets[i].l2_entries[j].data[k] = '0';
			l2.l2_sets[i].l2_entries[j].data[L2_BLOCK+1] = '\0';
				
			lru_counter.lru_sets[i].lru_bits[j]=0;
		}
	}
}

//Before terminating, the modified data in L2 cache should be written 
//back to MM to prevent loss of data.
void l2_terminate() {
	for(int i=0;i<SETS;i++) {
		for(int j=0;j<WAYS;j++) {
			if(l2.l2_sets[i].l2_entries[j].dirty==1) {
				l2_address l2_addr;
				l2_addr.tag=l2.l2_sets[i].l2_entries[j].tag;
				l2_addr.index=i;
				l2_addr.offset=0;
				l2_write_dirty_block_back_to_mm(l2_addr, j);
			}
		}
	}
}


