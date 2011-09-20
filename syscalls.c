#include "syscalls.h"

#include "dev/console.h"

int sys_getpid()
{
  return 1;  // just the kernel, for now
}

void syscall_init()
{
  syscall_table[SYS_GETPID] = sys_getpid;
}

// do it like linux, use int 80h
void syscall_isr(int vector, int code)
{
  asm("cli");

  // from here call the actual system call
  
  

  asm("sti");
}
