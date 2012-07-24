#include "mm.h"
#include "kernel/heap.h"
#include "dev/console.h"
#include "multiboot.h"
#include "support.h"

#include <assert.h>

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// kernel allocation routines

void mm_init(void *mbd, int print)
{
  mm_grub_multiboot(mbd, print);

  mm_kernel_end  = (unsigned int) &kernel_end;
  mm_kernel_end_aligned = (mm_kernel_end & 0xFFFFF000) + 0x1000;
  mm_kernel_size = &kernel_end - &kernel_start;
  c_printf("[mm]      kernel_size: %d bytes [end: 0x%x]\n", mm_kernel_size, mm_kernel_end);

  // align the memory base on a 4KB boundary
  // start allocable memory 1MB past kernel
  mm_high_mem_base      = ((mm_kernel_end & 0xFFFFFF000) + MM_FRAME_SIZE) + 0x100000;	
  mm_high_mem_limit     = _memory_ceiling - mm_high_mem_base;
  mm_total_frames	= mm_high_mem_limit / 4096;

  // configure the bitmap
  mm_bit_map = (unsigned char *) mm_kernel_end_aligned;	// mm bitmap follows kernel end
  mm_bit_map_length = ((mm_total_frames/8) & 0xfffffffc) + 4;	// align on dword

  c_printf("[mm]      high-mem base: 0x%x, limit 0x%x\n", mm_high_mem_base, mm_high_mem_limit);
  c_printf("[mm]      bit-map base: 0x%x, size: %d bytes\n", mm_bit_map, mm_bit_map_length);
  c_printf("[mm]      total frames available: %d\n", mm_total_frames);

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
  c_printf("[mm]    couldn't find a free page! [%d allocated]\n", bit_index);
  panic("tomos is out of memory!\n");
}

void mm_set_frame(uint32_t idx)
{
  // mark the frame as used
  MM_FRAME_ENABLE_BM(idx);
  mm_allocated_frames++;

  if (MM_FRAME_ADDRESS(idx) > mm_highest_allocd)
    mm_highest_allocd = MM_FRAME_ADDRESS(idx);
}

void mm_clear_frame(void *frame)
{
  // just clear the bit index
  unsigned int idx = ((unsigned int)frame - mm_high_mem_base) / MM_FRAME_SIZE;

  MM_FRAME_DISABLE_BM(idx);
  mm_allocated_frames--;
}

/*
 *

The format of the Multiboot information structure (as defined so far) follows:

             +-------------------+
     0       | flags             |    (required)
             +-------------------+
     4       | mem_lower         |    (present if flags[0] is set)
     8       | mem_upper         |    (present if flags[0] is set)
             +-------------------+
     12      | boot_device       |    (present if flags[1] is set)
             +-------------------+
     16      | cmdline           |    (present if flags[2] is set)
             +-------------------+
     20      | mods_count        |    (present if flags[3] is set)
     24      | mods_addr         |    (present if flags[3] is set)
             +-------------------+
     28 - 40 | syms              |    (present if flags[4] or
             |                   |                flags[5] is set)
             +-------------------+
     44      | mmap_length       |    (present if flags[6] is set)
     48      | mmap_addr         |    (present if flags[6] is set)
             +-------------------+
     52      | drives_length     |    (present if flags[7] is set)
     56      | drives_addr       |    (present if flags[7] is set)
             +-------------------+
     60      | config_table      |    (present if flags[8] is set)
             +-------------------+
     64      | boot_loader_name  |    (present if flags[9] is set)
             +-------------------+
     68      | apm_table         |    (present if flags[10] is set)
             +-------------------+
     72      | vbe_control_info  |    (present if flags[11] is set)
     76      | vbe_mode_info     |
     80      | vbe_mode          |
     82      | vbe_interface_seg |
     84      | vbe_interface_off |
     86      | vbe_interface_len |
             +-------------------+

 *
 */

/* print the GRUB memory map */
void mm_grub_multiboot(void *mbd, int print)
{
  multiboot_info_t *mbi;
  mbi = (multiboot_info_t *) mbd; 

       /* Are mem_* valid? */
  if (print && CHECK_FLAG (mbi->flags, 0))
    c_printf("mem_lower = %uKB, mem_upper = %uKB\n",
               (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
     
  /* Are mmap_* valid? */
  if (CHECK_FLAG (mbi->flags, 6))
  {
    memory_map_t *mmap;

    _memory_ceiling = 0;
     
    if (print)
      c_printf("mmap_addr = 0x%x, mmap_length = 0x%x\n",
              (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    for (mmap = (memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
          mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size))) {

      if (mmap->type == 0x01) {
        if (print)
          c_printf(" Conventional RAM\n");
        if ((mmap->base_addr_low + mmap->length_low) > _memory_ceiling) {
          _memory_ceiling = mmap->base_addr_low + mmap->length_low;

	  // take off another 65k
          _memory_ceiling -= 0x10000;
        }
      } else if (print) {
        c_printf(" RESERVED\n");
      }

      // how retarded, the grub displaymem function doesn't confusingly concatenate
      // both parts of the address (high/low)
      if (print)
        c_printf("    base: 0x%x, len: 0x%x, type: 0x%x [%u bytes]\n",
                 (unsigned) mmap->base_addr_low,
                 (unsigned) mmap->length_low,
                 (unsigned) mmap->type,
                 (unsigned) mmap->length_low);

    }
  }

  if (print)
    c_printf("\n");
}
