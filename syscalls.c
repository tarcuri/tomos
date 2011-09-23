#include "syscalls.h"
#include "x86.h"
#include "kernel/interrupt.h"

#include "dev/console.h"

#include "kernel/process.h"

void syscall_init()
{
  syscall_table[SYS_getpid] = sys_getpid;
  _install_isr(INT_VEC_SYSCALL, syscall_isr);
}

int sys_getpid(context_t *c, unsigned int *args)
{
  c->eax = current_proc->pid;
}

// do it like linux, use int 80h
void syscall_isr(int vector, int code)
{
  // from here call the actual system call
  int call_num = current_proc->context->eax;

  unsigned int *args = ((unsigned int *)(current_proc->context + 1)) + 1;
  
  // Invoke the handler.  The first argument is the process
  // context; the second is the address of the first user
  // syscall argument, which is  the second longword following
  // the context save area on the stack.
  (*syscall_table[call_num])(current_proc->context, args); 

  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
