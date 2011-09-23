#ifndef SYSCALL_H
#define SYSCALL_H

#include "kernel/process.h"


#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#define NUM_SYSCALLS	10

#define INT_VEC_SYSCALL	0x80

// system call numbers
#define SYS_exit	1
#define SYS_close	2
#define SYS_open	3
#define SYS_read	4
#define SYS_write	5
#define SYS_lseek	6
#define SYS_link	7
#define SYS_unlink	8
#define SYS_getpid	9
#define SYS_fork	10
#define SYS_kill	11
#define SYS_execve	12
#define SYS_sbrk	13
#define SYS_stat	14
#define SYS_fstat	15
#define SYS_isatty	16
#define SYS_times	17
#define SYS_wait	18

// system call table
int (*syscall_table[NUM_SYSCALLS])();

// user level interface for system calls

// kernel implementation of system calls 
int sys_exit	(context_t *c, unsigned int *args);
int sys_close	(context_t *c, unsigned int *args);
int sys_open	(context_t *c, unsigned int *args);
int sys_read	(context_t *c, unsigned int *args);
int sys_write	(context_t *c, unsigned int *args);
int sys_lseek	(context_t *c, unsigned int *args);
int sys_link	(context_t *c, unsigned int *args);
int sys_unlink	(context_t *c, unsigned int *args);
int sys_getpid	(context_t *c, unsigned int *args);
int sys_fork	(context_t *c, unsigned int *args);
int sys_kill	(context_t *c, unsigned int *args);
int sys_execve	(context_t *c, unsigned int *args);
int sys_sbrk	(context_t *c, unsigned int *args);
int sys_stat	(context_t *c, unsigned int *args);
int sys_fstat	(context_t *c, unsigned int *args);
int sys_isatty	(context_t *c, unsigned int *args);
int sys_times	(context_t *c, unsigned int *args);
int sys_wait	(context_t *c, unsigned int *args);

void syscall_init(void);

void syscall_isr(int vector, int code);

#endif
