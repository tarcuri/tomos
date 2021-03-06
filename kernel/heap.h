#ifndef HEAP_H
#define HEAP_H

#include "mm.h"

#define HEAP_MAGIC_TAG_31	0x1A4BC8D6

#define NULL                    0

#include <stdint.h>

//#define HEAP_BASE_ADDRESS	(unsigned int)(&kernel_end + 0x4000)
#define HEAP_BASE_ADDRESS	(unsigned int)(mm_highest_allocd + 0x1000)
#define HEAP_INITIAL_SIZE	0x100000
#define HEAP_LIMIT_ADDRESS	0x01000000	// expandable to 16MB
#define HEAP_INDEX_SIZE		0x1000		// 4096 holes

// does a heap blocks size inlcude header information?

typedef struct heap_header
{
  unsigned int tag_hole;
  unsigned int mem_size;
} heap_header_t;

typedef struct heap_footer
{
  unsigned int tag;
  heap_header_t *header;
} heap_footer_t;

typedef struct heap
{
  heap_header_t **index;		// index of free headers
  unsigned int index_size;
  unsigned int base;	// base address of allocable region
  unsigned int end;	// current end of the region (<= limit)
  unsigned int limit;	// address of the maximum expansion point
  unsigned int super;	// set if this is a kernel heap
  unsigned int write;	// set if this heap is writable
} heap_t;

// global data
heap_t *k_heap;

unsigned int k_heap_loc;

// functions

/*
 * heap_init(void)
 *
 * 1. define heap base address 0x4000 bytes beyond the end of the kernel
 * 2. initialize heap index one past the base address
 *
 */
void heap_init(void);
void heap_expand(uint32_t bytes, heap_t *h);
void dump_heap_index(heap_t *);

// alloc a contiguous region of more, optionally page aligned
void *halloc(unsigned int size, int align, heap_t *h);
void hfree(void *, heap_t *heap);

uint32_t kmalloc(uint32_t size);
uint32_t kmalloc_a(uint32_t size, int align);
uint32_t kmalloc_p(uint32_t size, int align, uint32_t *phys);

static int find_smallest_hole(unsigned int, int, heap_t *);
static void remove_from_index(unsigned int, heap_t *);
static void insert_into_index(heap_header_t *, heap_t *);

#endif
