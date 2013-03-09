#include "support.h"
#include "x86.h"

#include "kernel/process.h"

#include <stdint.h>

void panic(char *reason){
  asm( "cli" );
  c_printf("\nPANIC: %s\nHalting...", reason);

  for(;;)
    ;
}

uint32_t get_eflags()
{
  uint32_t eflags;
  asm volatile ("pushf; popl %0" : "=g"(eflags));
  return eflags;
}

void print_eflags()
{
   uint32_t eflags = get_eflags();
   c_printf("\nEFLAGS(0x%x): ", eflags);
   if (eflags & X86_EFLAGS_CF)
     c_printf(" CF");
   if (eflags & X86_EFLAGS_PF)
     c_printf(" PF");
   if (eflags & X86_EFLAGS_AF)
     c_printf(" AF");
   if (eflags & X86_EFLAGS_ZF)
     c_printf(" ZF");
   if (eflags & X86_EFLAGS_SF)
     c_printf(" SF");
   if (eflags & X86_EFLAGS_TF)
     c_printf(" TF");
   if (eflags & X86_EFLAGS_IF)
     c_printf(" IF");

   c_printf("\n");
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


