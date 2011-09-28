#ifndef STACK_H
#define STACK_H

#include <stdint.h>

#define STACK_SIZE	0x1000		// 64K stacks

typedef uint32_t stack_t[STACK_SIZE];

void stack_dump();

#endif
