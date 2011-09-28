#ifndef STACK_H
#define STACK_H

#include "kernel.h"

#define STACK_SIZE	0x1000		// 64K stacks

typedef unsigned int stack_t[STACK_SIZE];

extern unsigned int *kernel_esp;

void stack_dump(stack_t *);

#endif
