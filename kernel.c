#include "x86.h"
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
#include "dev/disk.h"

#include "tomsh.h"

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

  _install_isr(0x27, de_handler);
 
  mm_grub_multiboot(mbd, 0);

  // merge all memory init into the mm module
  mm_init();
  pg_init();

  heap_init();

  kb_init();

  pci_init();
  //ata_init();

  asm ("sti");

  pci_dev_t ich6_ide;
  ich6_ide.bus = 0;
  ich6_ide.slot = 31;
  ich6_ide.func = 1;
  pci_probe_device_config(&ich6_ide, 1);
  c_getcode();

  pci_dev_t *dev= pci_list_head;
  while (dev) {
    pci_probe_device_config(dev, 1);
    c_printf("%d s\n", (unsigned int)clock_ticks/TIMER_DEFAULT_TICKS_PER_SECOND);
    c_getcode();

    dev = dev->next;
  }


  c_printf("\n");

  disk_request_t dr;
  dr.cmd = DISK_CMD_READ;
  dr.status = DISK_STATUS_READ_PENDING;
  dr.lba = 0;
  dr.num_blocks = 4;
  dr.blocks_complete = 0;

  dr.buffer = (void *) kmalloc(DISK_BLOCK_SIZE * 16, 0);

  ata_identify_device();
  c_printf("reading sectors...\n");
  ata_read_sectors(&dr);

  c_printf("DONE\n");

/*
  int i;
  char *buf = (char *) dr.buffer;
  for (i = 0; i < DISK_BLOCK_SIZE; ++i) {
    c_printf("%x", buf[i]);
    if (i && (i % 20 == 0))
      c_printf("\n");
  }
*/
  command_loop();
}
