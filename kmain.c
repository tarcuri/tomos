#include "cio.h"

#include "multiboot.h"

#define GRUB_MAGIC_NUMBER	0x2BADB002

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

typedef struct global_desc_entry {
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_mid;
  unsigned char access_byte;
  unsigned char granularity;
  unsigned char base_high;
} global_desc_entry_t;

typedef struct global_desc_table {
  global_desc_entry_t null_desc;

  global_desc_entry_t code_seg;		// 0x08
  global_desc_entry_t data_seg;		// 0x10
  global_desc_entry_t stack_seg;	// 0x18
} global_desc_table_t;

typedef struct dt_register {
  unsigned short limit;
  unsigned int address;
} dtr_t;

// global data
global_desc_table_t *gdt;

void grub_mmap(void *mbd)
{
  multiboot_info_t *mbi;

  mbi = (multiboot_info_t *) mbd;
  cio_printf("flags: 0x%x\n", (unsigned) mbi->flags);

       /* Are mem_* valid? */
  if (CHECK_FLAG (mbi->flags, 0))
    cio_printf("mem_lower = %uKB, mem_upper = %uKB\n",
               (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
     
  /* Is boot_device valid? */
  if (CHECK_FLAG (mbi->flags, 1))
    cio_printf("boot_device = 0x%x\n", (unsigned) mbi->boot_device);

  /* Are mmap_* valid? */
  if (CHECK_FLAG (mbi->flags, 6))
  {
    multiboot_memory_map_t *mmap;
     
    cio_printf("mmap_addr = 0x%x, mmap_length = 0x%x\n",
              (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
          mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size)))
      cio_printf(" size = 0x%x, base_addr = 0x%x%x,"
                 " length = 0x%x%x, type = 0x%x\n",
                 (unsigned) mmap->size,
                 mmap->addr >> 32,
                 mmap->addr & 0xffffffff,
                 mmap->len >> 32,
                 mmap->len & 0xffffffff,
                 (unsigned) mmap->type);
  }

  cio_printf("\n\n");
}

void kmain( void* mbd, unsigned int magic, void *gdt_addr)
{
  // initial the console IO
  cio_init();

  if (magic != GRUB_MAGIC_NUMBER)
  {
    /* Something went not according to specs. Print an error */
    /* message and halt, but do *not* rely on the multiboot */
    /* data structure. */
    cio_printf("invalid GRUB magic number 0x%x\n", magic);
    return;
  }
  
  // GRUB multiboot mmap
  grub_mmap(mbd);

  // reference to the GDT
  gdt = (global_desc_table_t *) gdt_addr;

  cio_printf("gdt:   0x%x\n", gdt_addr);
  cio_printf("kmain: 0x%x\n", kmain); 

  // initialize interrupts, system modules
  cio_printf("\n\nModules:");
}
