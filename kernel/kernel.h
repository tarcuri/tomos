#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>


#define GRUB_MAGIC_NUMBER	0x2BADB002

// global descriptor table entry
struct global_desc_entry {
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_mid;
  unsigned char access_byte;
  unsigned char granularity;
  unsigned char base_high;
} __attribute__ ((__packed__));

// my hard-coded gdt
struct global_desc_table {
  struct global_desc_entry null_desc;

  struct global_desc_entry code_seg;	// 0x08
  struct global_desc_entry data_seg;	// 0x10
  struct global_desc_entry stack_seg;	// 0x18
} __attribute__ ((__packed__));

// gdtr/idtr register
struct dt_register {
  unsigned short limit;
  unsigned int   base;
} __attribute__ ((__packed__));


// global data
struct global_desc_table *gdt;

unsigned long long kernel_timer_ticks;

unsigned int kernel_stack_base;

struct dt_register gdtr_register;
struct dt_register idtr_register;

// functions
void kmain(void);

void kernel(void *mbd, uint32_t magic, uint32_t other);

#endif
