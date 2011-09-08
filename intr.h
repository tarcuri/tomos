#ifndef INTR_H
#define INTR_H

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
struct gdt_register {
  unsigned short limit;
  unsigned int address;
} __attribute__ ((__packed__)) gdtr_register;

struct idt_register {
  unsigned short limit;
  unsigned int address;
} __attribute__((__packed__)) idtr_register;

// idt entry
struct idt_entry {
  unsigned short offset_15_0;
  unsigned short selector;
  unsigned short type_attr;
  unsigned short offset_31_16;
} __attribute__ ((__packed__));

void init_interrupts(void);
static void init_pic(void);
static void init_idt(void);

void set_idt_entry(int entry, void (*handler)(void));

static void du_handler( int vector, int code);
static void de_handler( int vector, int code);

void (*_install_isr(int vector, void (*old)(int vector, int code)))(int vector, int code);

#endif
