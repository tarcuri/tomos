#ifndef INTR_H
#define INTR_H

#include <stdint.h>

// idt entry gate
struct idt_entry {
  uint16_t offset_15_0;
  uint16_t selector;
  uint16_t type_attr;
  uint16_t offset_31_16;
} __attribute__ ((__packed__));


// the IDT itself - 8 bytes x 256 entires - 2048 bytes (0x800)
struct idt_entry _idt[256];

// ISR table 
void (*_isr_table[256])(int32_t vector, int32_t code);

void init_interrupts(void);
static void init_pic(void);
static void init_idt(void);

void set_idt_entry(int32_t entry, void (*handler)(void));

void du_handler(int32_t vector, int32_t code);
void de_handler(int32_t vector, int32_t code);

void (*_install_isr(int32_t vector, void (*old)(int32_t vector, int32_t code)))(int32_t vector, int32_t code);

#endif
