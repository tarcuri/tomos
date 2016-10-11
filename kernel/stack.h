#ifndef STACK_H
#define STACK_H

#include <stdint.h>

#define STACK_SIZE	0x2000		// 64K stacks

#define GDT_LINEAR      0x08            // all memory, R/W
#define GDT_CODE        0x10            // all memory, R/E
#define GDT_DATA        0x18            // all memory, R/W
#define GDT_STACK       0x20            // all memory, R/W

typedef uint32_t stack_t[STACK_SIZE];

void stack_dump();

#endif
