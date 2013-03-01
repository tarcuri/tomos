#include "syscalls.h"
#include "x86.h"
#include "kernel/interrupt.h"

#include <errno.h>
#undef errno
extern int32_t errno;

#include "dev/console.h"

#include "kernel/process.h"

void syscall_init()
{
  syscall_table[SYS_exit    ] = sys_exit;
  syscall_table[SYS_close   ] = sys_close;
  syscall_table[SYS_open    ] = sys_open;
  syscall_table[SYS_read    ] = sys_read;
  syscall_table[SYS_write   ] = sys_write;
  syscall_table[SYS_lseek   ] = sys_lseek;
  syscall_table[SYS_link    ] = sys_link;
  syscall_table[SYS_unlink  ] = sys_unlink;
  syscall_table[SYS_getpid  ] = sys_getpid;
  syscall_table[SYS_fork    ] = sys_fork;
  syscall_table[SYS_kill    ] = sys_kill;
  syscall_table[SYS_execve  ] = sys_execve;
  syscall_table[SYS_sbrk    ] = sys_sbrk;
  syscall_table[SYS_stat    ] = sys_stat;
  syscall_table[SYS_fstat   ] = sys_fstat;
  syscall_table[SYS_isatty  ] = sys_isatty;
  syscall_table[SYS_times   ] = sys_times;
  syscall_table[SYS_wait    ] = sys_wait;

  _install_isr(INT_VEC_SYSCALL, syscall_isr);
}


// kernel system call implementations
int32_t sys_exit(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call exit\n");
  c->eax = -1;
}

int32_t sys_close(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call close\n");
  c->eax = -1;
}

int32_t sys_open(context_t *c, uint32_t *args)
{
  // this is going to implement VFS open()
  char *filename = (char *) args[0];
  int flags = (int) args[1];

  c_printf("> open(%s, %d)\n", filename, flags);

  // now well have a filename and flags, have vfs_open find the inode
  vfs_open(filename, flags);

  c->eax = -1;
}


int32_t sys_read(context_t *c, uint32_t *args)
{
  int32_t fd = args[0];
  unsigned char *buf = (unsigned char *) args[1];
  int32_t len = args[2];

  // how should this be done? can we wait for keypresses in a syscall?

  // need to clear bss
  // what?
  c_printf("read(%d, 0x%x, %d)\n", fd, buf, len);
//  for (i = 0; i < len; ++i)
//    buf[i] = c_getcode();

  int32_t i = vfs_read(fd, buf, len);

  c->eax = i;
}

int32_t sys_write(context_t *c, uint32_t *args)
{
  // use the console driver here
  int32_t file = args[0];
  char *p  = (char *) args[1];
  int32_t len  = args[2];

  //c_printf("write(%d, 0x%x, %d) ", file, p, len);
  c_write(p, len);
  c->eax = len;
}

int32_t sys_lseek(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call lseek\n");
  c->eax = 0;
}

int32_t sys_link(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call link\n");
  errno = EMLINK;
  c->eax = -1;
}

int32_t sys_unlink(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call unlink\n");
  errno = ENOENT;
  c->eax = -1;
}

int32_t sys_getpid(context_t *c, uint32_t *args)
{
  c->eax = current_proc->pid;
}

int32_t sys_fork(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call fork\n");
  errno = EAGAIN;
  c->eax = -1;
}

int32_t sys_kill(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call kill\n");
  errno = EINVAL;
  c->eax = -1;
}

int32_t sys_execve(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call execve\n");
  errno = ENOMEM;
  c->eax = -1;
}

// increases program data space (needed by malloc)
int32_t sys_sbrk(context_t *c, uint32_t *args)
{
  c_printf("sbrk(%d)\n", args[0]);
  
  c->eax = 0;
}

int32_t sys_stat(context_t *c, uint32_t *args)
{
  //c_printf("stat(%d, 0x%x)\n", args[0], args[1]);
  struct stat *st = (struct stat *) args[1];
  st->st_mode = S_IFCHR;
  c->eax = 0;

}

int32_t sys_fstat(context_t *c, uint32_t *args)
{
  //c_printf("fstat(%d,0x%x)\n", args[0], args[1]);
  struct stat *st = (struct stat *) args[1];
  st->st_mode = S_IFCHR;
  c->eax = 0;
}

int32_t sys_isatty(context_t *c, uint32_t *args)
{
  c_printf("isatty(%d)\n", args[0]);
  c->eax = 1;
}

int32_t sys_times(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call times\n");
  c->eax = -1;
}

int32_t sys_wait(context_t *c, uint32_t *args)
{
  c_printf("unimplemented call wait\n");
  errno = ECHILD;
  c->eax = -1;
}

// do it like linux, use int32_t 80h
void syscall_isr(int32_t vector, int32_t code)
{
  // from here call the actual system call
  int32_t call_num = current_proc->context->eax;

  uint32_t *args = ((uint32_t *)(current_proc->context + 1)) + 1;
  
  // Invoke the handler.  The first argument is the process
  // context; the second is the address of the first user
  // syscall argument, which is  the second longword following
  // the context save area on the stack.
  (*syscall_table[call_num])(current_proc->context, args); 

  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
