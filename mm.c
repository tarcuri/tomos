#include "mm.h"
#include "dev/console.h"
#include "multiboot.h"
#include "support.h"

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// kernel allocation routines

void mm_init()
{
  extern unsigned int kernel_start;	// defined in linker script
  extern unsigned int kernel_end;

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

void memset(void *dst, unsigned char value, unsigned int n)
{
  unsigned int i;
  unsigned char *p = dst;

  for (i = 0; i < n; ++i)
    *p++ = value;
}

// memory areas should not overlap, else use memmove
void memcpy(void *dst, void *src, unsigned int n)
{
  unsigned char *d = (unsigned char *) dst;
  unsigned char *s = (unsigned char *) src;

  int i;
  for (i = 0; i < n; ++i)
    *d++ = *s++;
}

void *memmove(void *dst, void *src, unsigned int n)
{
  unsigned char buf[n];
  unsigned char *d = (unsigned char *) dst;
  unsigned char *s = (unsigned char *) src;

  // copy src to buf, then buf to dst
  int i;
  for (i = 0; i < n; ++i) {
    buf[i] = s[i];
    d[i] = buf[i];
  }

  return dst;
}

char *strcpy(char *dst, const char *src, unsigned int n)
{
  char *p = dst;

  int i;
  for (i = 0; i < n; ++i) {
    if (*src == '\0') {
      *p = '\0';
      break;
    } else {
      *p++ = *src++;
    }
  }

  return dst;
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
