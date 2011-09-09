#include "multiboot.h"
#include "cio.h"
#include "intr.h"
#include "x86.h"
#include "support.h"

// TODO: implement simple console input and test interrupts

#define GRUB_MAGIC_NUMBER	0x2BADB002

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// global data
struct global_desc_table *gdt;

unsigned long long _timer_ticks;

struct dt_register gdtr_register;
struct dt_register idtr_register;


// functions
void grub_mmap(void *mbd);

void _timer_isr(int vector, int code)
{
  _timer_ticks++;
  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}

/*
** main kernel entry point
*/
void kernel( void* mbd, unsigned int magic, unsigned int other)
{
  // initialize IDT, interrupts
  extern void (*_isr_stubs[256])(void);

  init_interrupts();


  // initial the console IO
  cio_init();
  cio_printf("kernel: %xh\n\n", kernel); 

  _install_isr(INT_VEC_TIMER, _timer_isr);

  //_isr_table[0x21](0x21, 5);

  // reference to the GDT
  asm("sgdt %0" : "=m"(gdtr_register): :"memory");
  cio_printf("gdtr:   base: %xh, limit: %xh\n", gdtr_register.base, gdtr_register.limit);

  // get it back
  struct dt_register my_idtr;
  asm("sidt %0" : "=m"(my_idtr): :"memory");
  cio_printf("idtr:   base: %xh, limit: %xh\n", my_idtr.base, my_idtr.limit);

  // GDT should be all set by now
  if (magic != GRUB_MAGIC_NUMBER) {
    cio_printf("invalid GRUB magic number 0x%x\n", magic);
    return;
  }
  
  // GRUB multiboot mmap
  //grub_mmap(mbd);

  // is the IDT properly aligned?

  cio_printf("\nenabling processor interrupts\n");

  asm volatile ("sti;int $0x21");
//  asm volatile ("int $0x80");

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
