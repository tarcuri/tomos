#include "multiboot.h"
#include "cio.h"
#include "intr.h"

// TODO: implement simple console input and test interrupts

#define GRUB_MAGIC_NUMBER	0x2BADB002

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// global data
struct global_desc_table *gdt;

// IDT - stores 8 byte interrupt 'gates'
unsigned int _idt[512];		// 8 bytes * 256 entries = 2048 bytes
void *idt = (void *) _idt;	// these need better names...

// ISR table 
void (*_isr_table[256])(int vector, int code);

// DT registerss
struct gdt_register *gdtr = &gdtr_register;
struct idt_register *idtr = &idtr_register;

// functions
void grub_mmap(void *mbd);

/*
** main kernel entry point
*/
void kernel( void* mbd, unsigned int magic, unsigned int other)
{
  // initialize IDT, interrupts
  idtr->limit = 0x800;
  idtr->address = (unsigned int) idt; 

  init_interrupts();

  // initial the console IO
  cio_init();

  cio_printf("other: 0x%x\n", other);
  cio_printf("idt:   0x%x\n", idt);

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
  asm("sgdt %0" : "=m"(gdtr));
  gdt = (struct global_desc_table *) gdtr->address;

  // now initialize the IDT and interrupts
  cio_printf("gdtr:  0x%x [0x%x]\n", gdtr->address, gdtr->limit);
  cio_printf("idtr:  0x%x [0x%x]\n", idtr->address, idtr->limit);
  cio_printf("kernel: 0x%x\n", kernel); 

  cio_printf("enabling processor interrupts\n");
  //asm("sti");

  // system modules
  cio_printf("\n\nModules:");
}

/* print the GRUB memory map */
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
    memory_map_t *mmap;
     
    cio_printf("mmap_addr = 0x%x, mmap_length = 0x%x\n",
              (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    for (mmap = (memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
          mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size))) {

      if (mmap->type == 0x01)
        cio_printf("  Conventional RAM\n");
      else
        cio_printf("  RESERVED\n");

      cio_printf("    size = 0x%x, base_addr = 0x%x%x, "
                 " length = 0x%x%x, type = 0x%x\n",
                 (unsigned long) mmap->size,
                 mmap->base_addr_low, mmap->base_addr_high,
                 mmap->length_low, mmap->length_high,
                 (unsigned) mmap->type);
    }
  }

  cio_printf("\n\n");
}
