#include "kernel/kernel.h"

#include "x86.h"
#include "support.h"

#include "kernel/interrupt.h"
#include "kernel/mm.h"
#include "kernel/paging.h"
#include "kernel/heap.h"
#include "kernel/process.h"
#include "kernel/user.h"
#include "kernel/queue.h"
#include "kernel/timer.h"

#include "syscalls.h"

#include "kernel/stack.h"

#include "dev/device.h"
#include "dev/keyboard.h"
#include "dev/console.h"
#include "dev/pci.h"
#include "dev/ata.h"
#include "dev/disk.h"

#include "fs/ext2.h"
#include "fs/vfs.h"

#include "tomsh.h"

#include <stdio.h>
#include <stdlib.h>

#define KLOG_MAX        1048576/2

uint16_t next_uid;

int klog_idx;
char klog[KLOG_MAX];

void test_ata();

// initial kernel entry point
void kernel(void* mbd, uint32_t magic, uint32_t other)
{
  klog_idx = 0;

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

  // we must return to kmain immediately after proc_init
  proc_init();

  // at this point proc should have initlialized a pcb for the kernel,
  // when we return loader.S should jmp to isr_restore which will inialize a
  // new context and stack for the kernel and jump down to main
}

void kmain()
{
  // hardware and devices
  kb_init();
  pci_init();
  ata_init();

  asm volatile ("sti");

  syscall_init();
  vfs_init();

  next_uid = 1;
  add_user("root", "root");

  init_scheduler_queues();

  timer_init();

  create_process(0, "tomsh", command_loop);

  // this is the idle process loop
  while (1) {
    asm volatile ("hlt");
  }
}

void syslog(char *msg)
{
        char log_msg[1024];
        int msg_len;

        if (msg && *msg) {
                msg_len = strlen(msg);

                if (msg[msg_len - 1] != '\n') {
                        snprintf(log_msg, 1024, "%8d: %s\n", get_time(), msg);
                } else {
                        snprintf(log_msg, 1024, "%8d: %s", get_time(), msg);
                }

                if (strlen(log_msg) < KLOG_MAX - klog_idx) {
                        strncpy(&klog[klog_idx], log_msg, strlen(log_msg));
                        klog_idx += strlen(log_msg);
                } else {
                        c_printf("kernel log full\n");
                }
        }
}

void print_log(void)
{
        c_printf("%s", klog);
}

void test_ata()
{
  disk_request_t dr;

  dr.cmd = DISK_CMD_READ;
  dr.lba = 88;
  dr.num_blocks = 0;
  dr.blocks_complete = 0;
  dr.buffer = kmalloc(512*256);

  //ata_read_multiple(dr);
  device_t *hd = ata_open();
  hd->_ctrl(DISK_CMD_READ, &dr);
  printf("read %d blocks\n", dr.blocks_complete);

  kfree(dr.buffer);
}
