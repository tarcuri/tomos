#include "heap.h"
#include "pg.h"
#include "dev/console.h"

#include <assert.h>


// initialize the kernel heap
void heap_init()
{
  k_heap = (heap_t *) k_heap_loc;

  // initialize the index with one big hole
  k_heap->index = (heap_header_t **) ((unsigned int) k_heap + sizeof(heap_t));	// why am I doing this again? JamesM?
  k_heap->base = ((unsigned int) k_heap + sizeof(heap_t)
                                        + (HEAP_INDEX_SIZE * sizeof(heap_header_t)));

  k_heap->end = (unsigned int) k_heap->base + HEAP_INITIAL_SIZE;
  k_heap->limit = HEAP_LIMIT_ADDRESS;

  k_heap->super = 1;
  k_heap->write = 1;

  c_printf("[heap]    kernel heap initialized:\n");

  // create the initial hole
  heap_header_t *header = (heap_header_t *) k_heap->base;
  heap_footer_t *footer = (heap_footer_t *) (k_heap->end - sizeof(heap_footer_t));

  header->tag_hole = HEAP_MAGIC_TAG_31 | 0x1;	// this is a hole!
  header->mem_size = ((unsigned int) footer) - ((unsigned int) header + sizeof(heap_header_t));

  footer->tag = HEAP_MAGIC_TAG_31;
  footer->header = header;

  // clear the index and insert the first hole 
  int i;
  for (i = 0; i < HEAP_INDEX_SIZE; ++i)
    k_heap->index[i] = (heap_header_t *) 0x0;

  // make the first insertion manual, index is assumed to be populated from here
  k_heap->index[0] = header;
  k_heap->index_size = 1;
}

void dump_heap_index(heap_t *heap)
{
  int i;
  for (i = 0; i < heap->index_size; ++i) {
    heap_header_t *h = heap->index[i];
    c_printf("heap[%d]: head: 0x%x, foot: 0x%x, size: %d bytes\n",
             i, h, ((unsigned int)h + sizeof(heap_header_t) + h->mem_size), h->mem_size);
  }
}

// TODO: must be able to return a phsyical address!!!
void *halloc(unsigned int size, int align, heap_t *h)
{
  heap_t *heap = h;

  int hole_index = find_smallest_hole(size, align, heap);

  // claim this header
  heap_header_t *header = heap->index[hole_index];
  heap_footer_t *orig_footer = (heap_footer_t *) (((unsigned int) header)
                                                + header->mem_size + sizeof(heap_header_t));

  header->tag_hole = HEAP_MAGIC_TAG_31;
  remove_from_index(hole_index, heap);

  // worth splitting?
  int split = 0;
  if (header->mem_size > (2 * (sizeof(heap_header_t) + sizeof(heap_footer_t)) + size + 4))
    split = 1;

  unsigned int address;

  if (align) {
    address = ((unsigned int) header & 0xFFFFF000) + 0x1000;

    // adjust the header size
    header->mem_size = header->mem_size - (address - (unsigned int) header);

    // copy the header to its offset
    heap_header_t *offset = (heap_header_t *) ((unsigned int)address - sizeof(heap_header_t));
    header = (heap_header_t *) memmove((void *) offset, (void *) header, sizeof(heap_header_t));
  } else {
    address = ((unsigned int)header) + sizeof(heap_header_t);
    header->mem_size = size;
  }

  // create the newly allocated blocks footer
  heap_footer_t *new_footer = (heap_footer_t *) (address + size);
  new_footer->tag = HEAP_MAGIC_TAG_31;
  new_footer->header = header;

  // now split the hole
  if (split) {
    // create a new header just beyond the new_footer
    heap_header_t *new_header = (heap_header_t *)(((unsigned int)new_footer) + sizeof(heap_footer_t));
    new_header->tag_hole = HEAP_MAGIC_TAG_31 | 0x1;
    new_header->mem_size =   ((unsigned int) orig_footer)
                           - ((unsigned int) new_header + sizeof(heap_header_t));

    orig_footer->header = new_header;
    insert_into_index(new_header, heap);
  }

  //c_printf("allocated %d bytes at 0x%x\n", size, address);
  return (void *) address;
}

void hfree(void *p, heap_t *heap)
{
  heap_header_t *header = (heap_header_t *) ((unsigned int)p - sizeof(heap_header_t));

  // make sure this memory isn't already free, then free it
  assert((header->tag_hole & 0x1) == 0);
  header->tag_hole = HEAP_MAGIC_TAG_31 | 0x1;
  insert_into_index(header, heap);
}

/*
 *
 *
 */
uint32_t kmalloc(uint32_t size)
{
  return kmalloc_p(size, 0, 0);
}

uint32_t kmalloc_a(uint32_t size, int align)
{
  return kmalloc_p(size, align, 0);
}

uint32_t kmalloc_p(uint32_t size, int align, uint32_t *phys)
{
	void *addr = halloc(size, align, k_heap);
	if (phys) {
		page_t *page = pg_get_page((uint32_t)addr, 0, kernel_pg_directory);
		*phys = page->frame * 0x1000 + ((uint32_t) addr & 0xFFF);
    }
    return (uint32_t) addr;
}

void kfree(void *p)
{
	hfree(p, k_heap);
}

// private static functions
static int find_smallest_hole(unsigned int size, int align, heap_t *heap)
{
  // does not care about overhead
  int i;
  for (i = 0; i < heap->index_size; ++i) {
    heap_header_t *h = heap->index[i];

    if (h->mem_size >= size) {
      // NOTE: user wants memory that *he has access to* to be page aligned
      //       so that the returned header address will NOT be page aligned
      if (align) {
        unsigned int aligned = ((unsigned int) h & 0xFFFFF000) + 0x1000;
        unsigned int aligned_size = h->mem_size - (aligned - (unsigned int) h);

        if (size > aligned_size)	// can't fit the aligned block
          continue;
      }
      return i;
    }
  }

  return -1;
}

static void remove_from_index(unsigned int hole_index, heap_t *heap)
{
  // shift up holes from index[hole]
  int i;
  for (i = hole_index; i < heap->index_size; ++i)
    heap->index[i] = heap->index[i + 1];

  heap->index_size -= 1;
}

static void insert_into_index(heap_header_t *h, heap_t *heap)
{
  assert((heap->index_size + 1) < HEAP_INDEX_SIZE);

  // sorted by increasing hole size
  int i;
  for (i = 0; i < heap->index_size; ++i)
    if (h->mem_size <= heap->index[i]->mem_size)
      break;

  // shift holes i+ down, going from the end back to i
  int j;
  for (j = heap->index_size; j > i; --j)
    heap->index[j] = heap->index[j - 1];

  heap->index[i] = h;
  heap->index_size += 1;
}
