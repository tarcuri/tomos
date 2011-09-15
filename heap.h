#ifndef HEAP_H
#define HEAP_H

#include "pg.h"
#include "mm.h"

#define HEAP_MAGIC_TAG_31	0x1A4BC8D6

// place the kernel heap directly after the page table
#define HEAP_BASE_ADDRESS	(((unsigned int) pg_k_ptable_base) + 0x1000)
#define HEAP_LIMIT_ADDRESS	0x00400000	// expandable to 4MB
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

// functions
void heap_init(void);
void dump_heap_index(heap_t *);

void *kmalloc(unsigned int size, int align, heap_t *heap);

static int find_smallest_hole(unsigned int, int, heap_t *);
static void remove_from_index(unsigned int, heap_t *);
static void insert_into_index(heap_header_t *, heap_t *);

#endif
