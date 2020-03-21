#include "l1_struct.h"

extern short l1_getAddressFromTLB();
extern void l1_fill(char *arr, int size, int value);
extern void l1_BinaryRepInt(int n);
extern void l1_BinaryRepShort(short n);
extern int l1_or_array(char *arr, int size);
extern void L1_row_initialize(L1_row *L1row);
extern void L1_initialize();
extern void l1_valid_TagPrint ();
extern void l1_checkInitialization();
extern short l1_TLBtoFrame(int TLBaddress);
extern int l1_LRU_victimBlock(int index);
extern void l1_printLRUBits(int index);
extern void l1_LRU_Update(int index, char wayPrediciton);
extern void l1_getBlockFromL2(int TLBaddress);
extern void l1_cacheMiss(int TLBaddress, int index);
extern int l1_comparator(int TLBaddress, char wayPrediciton, int index);
extern char l1_runner(int TLBaddress, int index, int offset);
extern void l1_spaceRemover(char *s);

