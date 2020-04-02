#include "l1_struct.h"

extern char l1_read_from_l1_to_processor(int physical_address, int index, int offset);
extern void l1_write_from_processor_to_l1(int physical_address, int index, int offset);