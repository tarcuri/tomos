#include "syscalls.h"

#include "dev/console.h"

#include "kernel/process.h"

void syscall_init()
{
  syscall_table[SYS_GETPID] = sys_getpid;
}

int sys_getpid(context *c, unsigned int *args)
{
  c->eax = current_proc->pid;
}

// do it like linux, use int 80h
void syscall_isr(int vector, int code)
{
  asm("cli");

  // from here call the actual system call
  int call_num = current_proc->context->eax;

  stack_t *pstack = (unsigned int *)(current_proc->context + 1) + 1;
  
  (*syscall_table[call_num])(current_proc->context, 0); 

/*
	// Invoke the handler.  The first argument is the process
	// context; the second is the address of the first user
	// syscall argument, which is  the second longword following
	// the context save area on the stack.

	(*_syscalls[num])( _current->context,
			   ((uint32_t *)(_current->context + 1)) + 1 );
*/

  asm("sti");
}
