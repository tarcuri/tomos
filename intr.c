#include "intr.h"
#include "x86.h"


extern void *idt;	
extern struct dt_register *idtr;

/*
**  IDT stuff
*/

void init_interrupts()
{
  // now load the IDTR
  asm("lidt (%0)" : : "m"(idtr));

  init_idt();
  init_pic();
}

static void init_idt()
{
  int i;
  extern void (*_isr_stubs[256])(void);

  for (i = 0; i < 256; ++i) {
    set_idt_entry(i, _isr_stubs[i]);
    _install_isr(i, du_handler);
  }

  _install_isr(INT_VEC_KEYBOARD, de_handler);
  _install_isr(INT_VEC_TIMER, de_handler);
}

// taken from sp2
static void init_pic(void){
   /*
   ** ICW1
   */
   __outb( PIC_MASTER_CMD_PORT, PIC_ICW1BASE | PIC_NEEDICW4 );
   __outb( PIC_SLAVE_CMD_PORT, PIC_ICW1BASE | PIC_NEEDICW4 );

   /*
   ** ICW2: master offset of 20 in the IDT, slave offset of 28
   */
   __outb( PIC_MASTER_IMR_PORT, 0x20 );
   __outb( PIC_SLAVE_IMR_PORT, 0x28 );

   /*
   ** ICW3: slave attached to line 2 of master, bit mask is 00000100
   **       slave id is 2
   */
   __outb( PIC_MASTER_IMR_PORT, PIC_MASTER_SLAVE_LINE );
   __outb( PIC_SLAVE_IMR_PORT, PIC_SLAVE_ID );

   /*
   ** ICW4
   */
   __outb( PIC_MASTER_IMR_PORT, PIC_86MODE );
   __outb( PIC_SLAVE_IMR_PORT, PIC_86MODE );

   /*
   ** OCW1: allow interrupts on all lines
   */
   __outb( PIC_MASTER_IMR_PORT, 0x00 );
   __outb( PIC_SLAVE_IMR_PORT, 0x00 );
}


void set_idt_entry(int entry, void (*handler)(void))
{
  struct idt_entry *g = (struct idt_entry *)idt + entry;

  g->offset_15_0 = (int)handler & 0xffff;
  g->selector = 0x0010;
  g->type_attr = IDT_PRESENT | IDT_DPL_0 | IDT_INT32_GATE;
  g->offset_31_16 = (int)handler >> 16 & 0xffff;
}


/* default, unexpected handler */
static void du_handler( int vector, int code)
{
        cio_printf( "\nVector=0x%x, code=%d\n", vector, code );
        panic( "Unhandled interrupt" );
}

/* default, expected handler */
static void de_handler(int vector, int code)
{
  if( vector >= 0x20 && vector < 0x30 ){
    __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
    if( vector > 0x28 )
      __outb( PIC_SLAVE_CMD_PORT, PIC_EOI );
  } else {
    panic("Unexpected \"expected\" interrupt!");
  }
}


void (*_install_isr(int vector, void (*old)(int vector, int code)))(int vector, int code)
{
  extern void (*_isr_table[256])(int vector, int code);
  void (*new)(int vector, int code);

  old = _isr_table[vector];
  _isr_table[vector] = new;
  return old;
}
