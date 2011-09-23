#ifndef SYSCALL_H
#define SYSCALL_H

#include "kernel/process.h"

#define NUM_SYSCALLS	10

#define INT_VEC_SYSCALL	0x80

// system call numbers
//#define SYS_EXIT	1
#define SYS_getpid	1

int (*syscall_table[NUM_SYSCALLS])();

int getpid();

int sys_getpid(context_t *c, unsigned int *args);

void syscall_init(void);

void syscall_isr(int vector, int code);

#endif
