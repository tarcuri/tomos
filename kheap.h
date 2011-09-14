#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include "pg.h"

/**
 * kernel heap -- grows down
 *
 * Maintain lists of byte, word, and dword size free memory nodes.
 */

#define KHEAP_MAGIC_TAG_31	0xdead733c

#define KHEAP_END_ADDRESS	0x00400000	// 4MB
#define KHEAP_BASE_ADDRESS	(((unsigned int) pg_k_ptable_base) + 0x1000)	// right after the page table
#define HEAP_INDEX_SIZE		0x20000
#define HEAP_MIN_SIZE		0x70000

#include "ordered_array.h"

typedef struct heap_block_header
{
  unsigned int tag_hole;	// first bit is a hole flag (set if this block is a hole)
  unsigned int size;
} heap_header_t;

typedef struct heap_block_footer
{
  unsigned int tag;
  heap_header_t *header;
} heap_footer_t;

typedef struct heap_block
{
  unsigned int size;
  heap_header_t *header;
} heap_block_t;

typedef struct heap
{
  ordered_array_t index;
  unsigned int base;		// heap base
  unsigned int end;		// end address
  unsigned int max;		// maximum address of heap
  unsigned char supervisor;	// should extra pages be mapped supervisor only?
  unsigned char read_only;	// read only?
} heap_t;

heap_t kernel_heap;

unsigned int kernel_heap_base_address;
unsigned int kernel_heap_end_address;

void kheap_init(void);

heap_t *create_heap(unsigned int start, unsigned int end, unsigned int max,
                    unsigned char supervisor, unsigned char read_only);

/**
 *
 * Allocation
 * 
 * 1: Search the heap for the smallest hole that will fit.
 *    if there isn't a big enough hole:
 *      1: expand the heap
 *      2: if heap is empty, add a new entry
 *      3: else adjust the last entry (header, rewrite footer)
 *      4: to ease control flow, just recurse when the heap expands
 * 
 * 2: decide if the hole should be divided (we need less space)
 * 
 * 3: adjust block if it needs to be page-aligned
 * 
 * 4: write the new header/footer
 * 
 * 5: if it needed to be split, do it now and add a new hole to the index
 * 
 * 6: return the address of the block (perhaps offset for the header)
 * 
 */
void *kh_alloc(unsigned int size, int align, heap_t *heap);

void kh_free(void *p, heap_t *heap);
char heap_header_t_less_than(void *a, void *b);
#endif
