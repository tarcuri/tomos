#include "support.h"

void panic(char *reason){
  asm( "cli" );
  cio_printf("\nPANIC: %s\nHalting...", reason);
  for(;;)
    ;
}

/*
** port I/O functions
*/
unsigned char __inb(unsigned int port)
{
  unsigned char val;
  asm volatile ("inb %%dx,%%al" :"=a"(val) :"d" (port));
  return val;
}

void __outb(unsigned int port, unsigned char val)
{
  asm volatile ("outb %%al,%%dx": :"d"(port),"a"(val));
}

unsigned short __inw(unsigned int port)
{
  unsigned short val;
  asm volatile ("inw %%dx,%%ax" :"=a"(val) :"d" (port));
  return val;
}

void __outw(unsigned int port, unsigned short val)
{
  asm volatile ("outw %%ax,%%dx": :"d"(port),"a"(val));
}


unsigned int __inl(unsigned int port)
{
  unsigned int val;
  asm volatile ("inl %%dx,%%eax" :"=a"(val) :"d" (port));
  return val;
}

void __outl(unsigned int port, unsigned int val)
{
  asm volatile ("outl %%eax,%%dx": :"d"(port),"a"(val));
}


