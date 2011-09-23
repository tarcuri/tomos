#include "syscalls_asm.h"

#define SYSCALL(name)			\
	.global name			; \
name:	movl	$SYS_##name, %eax	; \
	int	$INT_VEC_SYSCALL	; \
	ret

SYSCALL(getpid)
