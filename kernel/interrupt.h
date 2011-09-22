#ifndef INTR_H
#define INTR_H

// idt entry gate
struct idt_entry {
  unsigned short offset_15_0;
  unsigned short selector;
  unsigned short type_attr;
  unsigned short offset_31_16;
} __attribute__ ((__packed__));


// the IDT itself - 8 bytes x 256 entires - 2048 bytes (0x800)
struct idt_entry _idt[256];

// ISR table 
void (*_isr_table[256])(int vector, int code);

void init_interrupts(void);
static void init_pic(void);
static void init_idt(void);

void set_idt_entry(int entry, void (*handler)(void));

void du_handler( int vector, int code);
void de_handler( int vector, int code);

void (*_install_isr(int vector, void (*old)(int vector, int code)))(int vector, int code);

#endif
