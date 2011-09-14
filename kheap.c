#include "kheap.h"

#include "dev/console.h"
#include "mm.h"

#include "ordered_array.h"

void kheap_init()
{
  // kernel heap will start directly after the initial (statically allocated)
  // page directory and page table.

  // verify that kernel heap and end addresses are page aligned
  kernel_heap_base_address = (unsigned int) KHEAP_BASE_ADDRESS;
  kernel_heap_end_address =  (unsigned int) KHEAP_END_ADDRESS;

  ASSERT((kernel_heap_base_address % 0x1000) == 0);
  ASSERT((kernel_heap_end_address % 0x1000) == 0);

  // initialize the index
  kernel_heap.index = place_ordered_array((void *) kernel_heap_base_address,
                                          HEAP_INDEX_SIZE, heap_header_t_less_than);

  unsigned int heap_data_base = kernel_heap_base_address + sizeof(type_t) * HEAP_INDEX_SIZE;

  // page align the start address
  if (heap_data_base & 0xFFFFF000 != 0) {
    heap_data_base &= 0xFFFFF000;
    heap_data_base += 0x1000;
  }

  // write the start, end and max into the heap
  kernel_heap.base 	 = heap_data_base;
  kernel_heap.end  	 = heap_data_base + 0x00100000;	// 1MB inital size
  kernel_heap.max	 = KHEAP_END_ADDRESS;
  kernel_heap.supervisor = 1;
  kernel_heap.read_only  = 1;

  // start off with one large hole
  heap_header_t *hole = (heap_header_t *) heap_data_base;
  hole->size = kernel_heap.end - heap_data_base;
  hole->tag_hole = KHEAP_MAGIC_TAG_31 | 1;		// last bit means this is a hole

  insert_ordered_array((void *)hole, &kernel_heap.index);

  c_printf("[kheap]   kernel heap initialized, base: %x\n", kernel_heap_base_address);
  c_printf("[kheap]   initial heap size: %d KB [%x]\n", hole->size/1000, hole);
}

// debug routine, print heap map
void heap_dump_words(heap_t *heap, unsigned int n)
{
  unsigned short *word = (unsigned short *) heap->base;

  unsigned int i;
  for (i = 0; i < n; i++) {
    if ((i % 4) == 0)
      c_printf("\n");
    c_printf(" %x", *word++);
  }
}

char heap_header_t_less_than(void *a, void *b)
{
 return (((heap_header_t*)a)->size < ((heap_header_t*)b)->size) ? 1 : 0;
}

heap_t *create_heap(unsigned int start, unsigned int end, unsigned int max,
                    unsigned char supervisor, unsigned char read_only)
{
  heap_t *heap = (heap_t*) 0x0; // kmalloc(sizeof(heap_t));


  // make sure start, end address are page aligned

  //ASSERT(start%0x1000 == 0);
  return heap;
}

int smallest_hole(unsigned int size, int align, heap_t *heap)
{
  // search the ordered index for the smallest hole that will fit
  heap_header_t *header;
  unsigned int itr = 0;

  while (itr < heap->index.size) {
    header = (heap_header_t *) lookup_ordered_array(itr, &heap->index);

    if (align) {
      // Page-align the starting point of this header.
      unsigned int location = (unsigned int) header;
      int offset = 0;
      if ((location + sizeof(heap_header_t)) & 0xFFFFF000 != 0)
        offset = 0x1000 - ((location + sizeof(heap_header_t)) % 0x1000);

      int hole_size = (int) header->size - offset;
      // Can we fit now?
      if (hole_size >= (int)size)
        break;
    } else if (header->size >= size)
      break;

    itr++;
  }

  if (itr == heap->index.size)
    return -1; // We got to the end and didn't find anything.
  else
    return itr;
}

void *kh_alloc(unsigned int size, int align, heap_t *heap)
{
  unsigned int address = 0;

  int hole_index = smallest_hole(size + sizeof(heap_header_t) + sizeof(heap_footer_t), align, heap);

  if (hole_index == -1)
    return 0;	// couldn't find a hole!

  heap_header_t *header = (heap_header_t *) lookup_ordered_array(hole_index, &heap->index);

  remove_ordered_array(hole_index, &heap->index);

  // should we split the hole? can we fit another header, footer and a byte?
  heap_header_t *split = 0;
  unsigned int split_size = 0;
  if (header->size > (size + (2 * (sizeof(heap_header_t) + sizeof(heap_footer_t)) + 1))) {
    // well add the header address after to account for alignment
    split = (heap_header_t *) (size + sizeof(heap_header_t) + sizeof(heap_footer_t));

    split->size = header->size;
  }

  if (align) {
    // page align the address
    header = (heap_header_t *) ((unsigned int)header & 0xFFFFF000);
    header = (heap_header_t *) ((unsigned int)header + 0x1000);
  }

  if (split) {
    split = (heap_header_t *) ((unsigned int) header + (unsigned int) split);
    split->tag_hole = KHEAP_MAGIC_TAG_31 | 0x1;
    split->size = (split->size - (size + sizeof(heap_header_t) + sizeof(heap_footer_t)));
    
    // TODO: take alignment into account..
  }

  // fill out the header
  header->tag_hole = KHEAP_MAGIC_TAG_31 & 0xFFFFFFFE;	// mask out the hole bit
  header->size = size;	// header will be a negative offset from the returned address

  address = (unsigned int) header + sizeof(heap_header_t);

  // and footer
  heap_footer_t *foot = (heap_footer_t *) ((unsigned int)address + size);
  foot->tag = KHEAP_MAGIC_TAG_31 | 0x0;
  foot->header = header;

  c_printf("allocated %d bytes at 0x%x [0x%x], next hole at %x\n", size, address, foot, split);

  if (split)
   insert_ordered_array((void *)split, &heap->index);

  return (void *) address;
}

void kh_free(void *p, heap_t *heap)
{
 
}
