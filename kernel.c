#include "support.h"

#include "intr.h"
#include "mm.h"
#include "pg.h"
#include "heap.h"

#include "dev/keyboard.h"
#include "dev/console.h"
#include "dev/clock.h"
#include "dev/pci.h"
#include "dev/ata.h"

#include "x86.h"

#define GRUB_MAGIC_NUMBER	0x2BADB002

// global data
struct global_desc_table *gdt;

unsigned long long kernel_timer_ticks;

struct dt_register gdtr_register;
struct dt_register idtr_register;

unsigned int _memory_ceiling;

// functions
void main_loop(void);

// main kernel entry point
void kernel( void* mbd, unsigned int magic, unsigned int other)
{
  // initialize IDT, interrupts
  extern void (*_isr_stubs[256])(void);
  init_interrupts();

  // initial the console IO
  c_init();

  // reference to the GDT
  asm("sgdt %0" : "=m"(gdtr_register): :"memory");
  c_printf("[kernel]  gdtr: base: %xh, limit: %xh\n", gdtr_register.base, gdtr_register.limit);

  // get it back
  struct dt_register my_idtr;
  asm("sidt %0" : "=m"(my_idtr): :"memory");
  c_printf("[kernel]  idtr:   base: %xh, limit: %xh\n", my_idtr.base, my_idtr.limit);

  // GDT should be all set by now
  if (magic != GRUB_MAGIC_NUMBER) {
    c_printf("invalid GRUB magic number 0x%x\n", magic);
    return;
  }

  clock_init();
 
  mm_grub_multiboot(mbd, 0);

  // merge all memory init into the mm module
  mm_init();
  pg_init();

  heap_init();

  kb_init();

  //pci_init();
  ata_init();

  asm ("sti");

  main_loop();
}

void main_loop()
{
  c_printf("Welcome to tomos!\n");

  unsigned int last;
  while (1)
  {
    unsigned char c = c_getchar();
    if (c)
      c_putchar(c);

    switch(c){
    case '4':
      last = (unsigned int) kmalloc(4, 0, k_heap);
      break;
    case '8':
      last = (unsigned int) kmalloc(8, 0, k_heap);
      break;
    case '1':
      last = (unsigned int) kmalloc(16, 0, k_heap);
      break;
    case 'f':
      kfree((void *) last, k_heap);
      break;
    case 'd':
      dump_heap_index(k_heap);
    };
  }
}



