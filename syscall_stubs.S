#include "syscalls_asm.h"

#define SYSCALL(name)			\
	.global name			; \
name:	movl	$SYS_##name, %eax	; \
	int	$INT_VEC_SYSCALL	; \
	ret

SYSCALL(exit)
SYSCALL(close)
SYSCALL(open)
SYSCALL(read)
SYSCALL(write)
SYSCALL(lseek)
SYSCALL(link)
SYSCALL(unlink)
SYSCALL(getpid)
SYSCALL(fork)
SYSCALL(kill)
SYSCALL(execve)
SYSCALL(sbrk)
SYSCALL(stat)
SYSCALL(fstat)
SYSCALL(isatty)
SYSCALL(times)
SYSCALL(wait)
