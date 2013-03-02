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
#include "fs/vfs.h"

#include "tomsh.h"

#include <stdio.h>
#include <stdlib.h>

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

  // merge all memory init into the mm module
  k_heap = 0;
  mm_init(mbd, 1);
  pg_init(); // pg.c enables the heap

  // test page fault
  //uint32_t *ptr = (uint32_t *)0xAB000000;
  //uint32_t do_page_fault = *ptr;

  // test heap
  //void *mem = halloc(20, 0, k_heap);
  //c_printf("mem addr: 0x%x\n", mem);


  // system calls reference processes, and processes require heap.
  proc_init();

  // hardware and devices
  kb_init();
  pci_init();
  ata_init();

  syscall_init();

  //c_printf("waiting\n");
  //while (1) {}

  //asm volatile ("sti");
  //c_printf("EFLAGS: %x\n", get_eflags());
  //c_getcode();
  //c_printf("\nSystem initialization complete!\n");

  // at this point proc should have initlialized a pcb for the kernel,
  // when we return loader.S should jmp to isr_restore which will inialize a
  // new context and stack for the kernel and jump down to main
  //kmain();
}

void kmain()
{
  //c_printf("ANYTHING?\n");
  //stack_dump(current_proc);
  // TODO: interrupts fire for an ATA command only if printf()  is called first...
  //c_printf("Press any key to continue...\n");
  //putchar('?');
  //getchar();

  void *m = malloc(10);
  c_printf("malloc'd: 0x%x\n", m);
  *(int *)m = 10;
  c_printf("m_val: %d\n", *(int *)m);

  printf("pid: %d\n", getpid());
  printf("hello newlib\n");
  //sys_sbrk(10);
  //printf("hello\n");
  //c_printf("EFLAGS: %x\n", get_eflags());

  //ext2_init();
  command_loop();
}
