#include "mm.h"
#include "cio.h"
#include "support.h"


void mm_init()
{
  extern unsigned int kernel_start;	// defined in linker script
  extern unsigned int kernel_end;

  mm_kernel_end  = (unsigned int) &kernel_end;
  mm_kernel_end_aligned = (mm_kernel_end & 0xFFFFF000) + 0x1000;
  mm_kernel_size = &kernel_end - &kernel_start;
  cio_printf("[mm]      kernel_size: %d bytes [end: 0x%x]\n", mm_kernel_size, mm_kernel_end);

  // align the memory base on a 4KB boundary
  // start allocable memory 1MB past kernel
  mm_high_mem_base      = ((mm_kernel_end & 0xFFFFFF000) + MM_FRAME_SIZE) + 0x100000;	
  mm_high_mem_limit     = _memory_ceiling - mm_high_mem_base;
  mm_total_frames	= mm_high_mem_limit / 4096;

  // configure the bitmap
  mm_bit_map = (unsigned char *) mm_kernel_end_aligned;	// mm bitmap follows kernel end
  mm_bit_map_length = ((mm_total_frames/8) & 0xfffffffc) + 4;	// align on dword

  cio_printf("[mm]      high-mem base: 0x%x, limit 0x%x\n", mm_high_mem_base, mm_high_mem_limit);
  cio_printf("[mm]      bit-map base: 0x%x, size: %d bytes\n", mm_bit_map, mm_bit_map_length);
  cio_printf("[mm]      total frames available: %d\n", mm_total_frames);

  mm_allocated_frames = 0;
}

// TODO: keep a pointer to the last allocated bit
static unsigned int mm_get_free_frame()
{
  unsigned int *bit_map = (unsigned int *) mm_bit_map;
  unsigned int bit_index = 0;

  do {
    // check a 32 bits at a time
    if (*bit_map != 0xffffffff) {
      // there are free frames here

      // just go bit-by-bit for simpliciy
      int i;
      for (i = 0; i < 32; ++i, ++bit_index) {
        if ( !(*bit_map & (1 << i)) ) {
          // found a free frame
          return bit_index;
        }
      }
    }

    bit_index += 32;
    bit_map   += 1;
  } while ((bit_map - (unsigned int *) mm_bit_map) < mm_bit_map_length);

  // uh oh, didn't find a page
  cio_printf("[mm]    couldn't find a free page! [%d allocated]\n", bit_index);
  panic("tomos is out of memory!\n");
}

// allocate a single page
void *mm_alloc_frame()
{
  // grab the next free page index
  unsigned int idx = mm_get_free_frame();

  // mark the frame as used
  MM_FRAME_ENABLE_BM(idx);
  mm_allocated_frames++;

  // now return the address of the page
  return (void *) MM_FRAME_ADDRESS(idx);;
}

void mm_free_frame(void *frame)
{
  // just clear the bit index
  unsigned int idx = ((unsigned int)frame - mm_high_mem_base) / MM_FRAME_SIZE;

  MM_FRAME_DISABLE_BM(idx);
}
