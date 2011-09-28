#include "kernel/kernel.h"

#include "x86.h"
#include "support.h"

#include "kernel/interrupt.h"
#include "kernel/mm.h"
#include "kernel/pg.h"
#include "kernel/heap.h"
#include "kernel/process.h"

#include "syscalls.h"

#include "kernel/stack.h"

#include "dev/device.h"
#include "dev/keyboard.h"
#include "dev/console.h"
#include "dev/clock.h"
#include "dev/pci.h"
#include "dev/ata.h"
#include "dev/disk.h"

#include "fs/ext2.h"

#include "tomsh.h"

#include <stdio.h>

// initial kernel entry point
void kernel(void* mbd, uint32_t magic, uint32_t other)
{
  // initialize IDT, interrupts
  extern void (*_isr_stubs[256])(void);
  init_interrupts();

  // initial the console IO
  c_init();

  // reference to the GDT
  asm("sgdt %0" : "=m"(gdtr_register): :"memory");
  //c_printf("[kernel]  gdtr: base: %xh, limit: %xh\n", gdtr_register.base, gdtr_register.limit);

  // get it back
  struct dt_register my_idtr;
  asm("sidt %0" : "=m"(my_idtr): :"memory");
  //c_printf("[kernel]  idtr:   base: %xh, limit: %xh\n", my_idtr.base, my_idtr.limit);

  // GDT should be all set by now
  if (magic != GRUB_MAGIC_NUMBER) {
    c_printf("invalid GRUB magic number 0x%x\n", magic);
    return;
  }

  //clock_init();

  _install_isr(0x27, du_handler);
 
  mm_grub_multiboot(mbd, 0);

  // merge all memory init into the mm module
  mm_init();
  pg_init();
  heap_init();

  // hardware and devices
  kb_init();
  pci_init();

  // TODO: still get int 82h if interrupts aren't already enabled
  proc_init();
  syscall_init();
  ata_init();

  //asm volatile ("sti");
  //c_printf("EFLAGS: %x\n", get_eflags());
  //stack_dump(current_proc->stack);
  //c_getcode();
  //c_printf("\nSystem initialization complete!\n");
  // at this point proc should have initlialized a pcb for the kernel,
  // when we return loader.S should jmp to isr_restore which will inialize a
  // new context and stack for the kernel and jump down to main
  //kmain();
}

void kmain()
{
  // TODO: interrupts fire for an ATA command only if printf()  is called first...
  //printf("Press any key to continue...\n");
  //putchar('?');
  //getchar();

  c_printf("EFLAGS: %x\n", get_eflags());

  ext2_init();
  command_loop();
}
