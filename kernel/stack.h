#ifndef STACK_H
#define STACK_H

#define STACK_SIZE	0x4000		// 16K stacks

typedef unsigned int stack_t[STACK_SIZE];

extern unsigned int *kernel_esp;

#endif
