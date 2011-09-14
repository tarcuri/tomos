#include "support.h"

#include "intr.h"
#include "mm.h"
#include "pg.h"
#include "kheap.h"

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

  mm_init();
  pg_init();

  kheap_init();

  kb_init();

  //pci_init();
  ata_init();

  asm ("sti");
  main_loop();
}

void main_loop()
{
  c_printf("Welcome to tomos!\n");
  while (1)
  {
    unsigned char *buffer = (unsigned char *) kh_alloc(20, 0, &kernel_heap);

    if (buffer) {
      c_printf("buffer allocated 0x%x\n", buffer);

      char c = 'a';
      int i;
      for (i = 0; i < 19; ++i) {
        buffer[i] = c;
        c++;
      }
      buffer[19] = '\0';
    }
    c_printf("mem string: %s\n\n", buffer);

    heap_dump_words(&kernel_heap, 12);

    // for now try to block on input
    unsigned char c = c_getchar();
    if (c)
      c_putchar(c);
  }
}



