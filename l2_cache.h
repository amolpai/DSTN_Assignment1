#include "l2_structures.h"

extern void l2_read_to_l1(int physical_address);
extern void l2_write_from_l1_to_l2(int physical_address);
extern void l2_initialize();
extern void l2_terminate();
extern void l2_print_l2_cache_set(int physical_address);
extern void l2_print_lru_counter_for_set(int physcial_address);




