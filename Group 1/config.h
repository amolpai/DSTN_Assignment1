#ifndef _CONFIG_H_
#define _CONFIG_H_

#define tlb_lookup_time 1
#define tlb_miss_OS_overhead 5
#define l1_cache_indexing_time 1
#define l1_cache_lookup_time 1
#define l2_cache_lookup_time 1
#define mm_page_lookup_time 1
#define swap_space_to_mm_time 1
#define disk_to_mm_time 1
#define cpu_to_l1_write_time 1
#define l1_to_l2_write_time 1
#define l2_to_l1_write_time 1
#define l2_to_mm_write_time 1
#define mm_to_l2_write_time 1
#define mm_to_disk_write_time 1
#define mm_to_swap_space_write_time 1
#define page_fault_time 1
#define restart_overhead_time 1
#define context_switch_time 1

#endif