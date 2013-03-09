#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdint.h>

void panic(char *reason);

uint32_t get_eflags(void);
void print_eflags(void);

unsigned char __inb(unsigned int port);
unsigned short __inw(unsigned int port);
unsigned int __inl(unsigned int port);

void __outb(unsigned int port, unsigned char value);
void __outw(unsigned int port, unsigned short value);
void __outl(unsigned int port, unsigned int value);

/* get EFLAGS register */
//int sp_get_flags(void);

//void sp_pause(void);

#endif
