/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

//#include <syscalls.h>   // from tomos
#include <syscalls.h>


/*
void _exit()
{
  unsigned int __attribute__((used)) vec_syscall = 0x80;
  __asm__ ("movl %0, %%eax ;"
           "int $vec_syscall ;"
           "ret ;"
           :
           : "r" (0x01));
}
int close(int file)
{
  unsigned int __attribute__((used)) vec_syscall = 0x80;
  __asm__ ("movl %0, %%eax ;"
           "int $vec_syscall ;"
           "ret ;"
           :
           : "r" (0x02));
}
int open(const char *name, int flags, ...)
{
  __asm__ ("movl $SYS_open, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int read(int file, char *ptr, int len)
{
  __asm__ ("movl $SYS_read, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int write(int file, char *ptr, int len)
{
  __asm__ ("movl $SYS_write, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int lseek(int file, int ptr, int dir)
{
  __asm__ ("movl $SYS_lseek, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int link(char *old, char *new)
{
  __asm__ ("movl $SYS_link, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int unlink(char *name)
{
  __asm__ ("movl $SYS_unlink, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int getpid()
{
  __asm__ ("movl $SYS_getpid, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int fork()
{
  __asm__ ("movl $SYS_fork, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int kill(int pid, int sig)
{
  __asm__ ("movl $SYS_kill, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int execve(char *name, char **argv, char **env)
{
  __asm__ ("movl $SYS_execve, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
caddr_t sbrk(int incr)
{
  __asm__ ("movl $SYS_sbrk, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int stat(const char *file, struct stat *st)
{
  __asm__ ("movl $SYS_stat, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int fstat(int file, struct stat *st)
{
  __asm__ ("movl $SYS_fstat, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int isatty(int file)
{
  __asm__ ("movl $SYS_isatty, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
clock_t times(struct tms *buf)
{
  __asm__ ("movl $SYS_times, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
int wait(int *status)
{
  __asm__ ("movl $SYS_wait, %eax ;"
           "int $INT_VEC_SYSCALL ;"
           "ret ;" );
}
*/
