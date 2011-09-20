#ifndef SYSCALL_H
#define SYSCALL_H

#define NUM_SYSCALLS	10

// system call numbers
//#define SYS_EXIT	1
#define SYS_GETPID	1

int (*syscall_table[NUM_SYSCALLS])();


/*
void _exit();
int close(int file);
char **environ;
int execve(char *name, char **argv, char **env);
int fork();
int fstat(int file, struct stat *st);
int getpid();
int isatty(int file);
int kill(int pid, int sig);
int link(char *old, char *new);
int lseek(int file, int ptr, int dir);
int open(const char *name, int flags, ...);
int read(int file, char *ptr, int len);
caddr_t sbrk(int incr);
int stat(const char *file, struct stat *st);
clock_t times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
int write(int file, char *ptr, int len);
int gettimeofday(struct timeval *p, struct timezone *z);
*/

int sys_getpid(void);

void syscall_init(void);

void syscall_isr(void);

#endif
