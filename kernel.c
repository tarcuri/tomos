#include "multiboot.h"
#include "support.h"

#include "intr.h"
#include "cio.h"
#include "mm.h"
#include "pg.h"

#include "x86.h"

// TODO: implement simple console input and test interrupts

#define GRUB_MAGIC_NUMBER	0x2BADB002

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// global data
struct global_desc_table *gdt;

unsigned long long _timer_ticks;

struct dt_register gdtr_register;
struct dt_register idtr_register;

unsigned int _memory_ceiling;

// functions
void main_loop(void);
void grub_mmap(void *mbd, int print);

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
  cio_printf("[kernel]  gdtr: base: %xh, limit: %xh\n", gdtr_register.base, gdtr_register.limit);

  // get it back
  struct dt_register my_idtr;
  asm("sidt %0" : "=m"(my_idtr): :"memory");
  cio_printf("[kernel]  idtr:   base: %xh, limit: %xh\n", my_idtr.base, my_idtr.limit);

  // GDT should be all set by now
  if (magic != GRUB_MAGIC_NUMBER) {
    cio_printf("invalid GRUB magic number 0x%x\n", magic);
    return;
  }
  
  grub_mmap(mbd, 0);

  mm_init();
  pg_init();

  unsigned int val = 10;
  cio_printf("value at 0x%x\n", &val);

  unsigned int *v = (unsigned int *)(0xF00FF00F);
  cio_printf("mem: %x\n", *v);

  asm ("sti");
  main_loop();
}

void main_loop()
{
  cio_printf("Welcome to tomos!\n");
  cio_printf("Press any key to allocate a new page\n");
  while (1)
  {
    // for now try to block on input
    unsigned char c = _cio_getchar();
    cio_putchar(c);

    cio_printf("char addr 0x%x\n", &c);

    void *page = mm_alloc_frame();

    cio_printf("\npage at 0x%x allocated\n", page);
  }
}


/* print the GRUB memory map */
void grub_mmap(void *mbd, int print)
{
  multiboot_info_t *mbi;
  mbi = (multiboot_info_t *) mbd; 

       /* Are mem_* valid? */
  if (print && CHECK_FLAG (mbi->flags, 0))
    cio_printf("mem_lower = %uKB, mem_upper = %uKB\n",
               (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
     
  /* Are mmap_* valid? */
  if (CHECK_FLAG (mbi->flags, 6))
  {
    memory_map_t *mmap;

    _memory_ceiling = 0;
     
    if (print)
      cio_printf("mmap_addr = 0x%x, mmap_length = 0x%x\n",
              (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
    for (mmap = (memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
          mmap = (memory_map_t *) ((unsigned long) mmap
                                     + mmap->size + sizeof (mmap->size))) {

      if (mmap->type == 0x01) {
        if (print)
          cio_printf(" Conventional RAM\n");
        if ((mmap->base_addr_low + mmap->length_low) > _memory_ceiling) {
          _memory_ceiling = mmap->base_addr_low + mmap->length_low;

	  // take off another 65k
          _memory_ceiling -= 0x10000;
        }
      } else if (print) {
        cio_printf(" RESERVED\n");
      }

      // how retarded, the grub displaymem function doesn't confusingly concatenate
      // both parts of the address (high/low)
      if (print)
        cio_printf("    base: 0x%x, len: 0x%x, type: 0x%x [%u bytes]\n",
                 (unsigned) mmap->base_addr_low,
                 (unsigned) mmap->length_low,
                 (unsigned) mmap->type,
                 (unsigned) mmap->length_low);

    }
  }

  if (print)
    cio_printf("\n");
}
