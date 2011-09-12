#include "dev/clock.h"
#include "dev/console.h"
#include "x86.h"


void clock_init()
{
  clock_ticks = 0;

  _install_isr(INT_VEC_TIMER, timer_isr);

  c_printf("[clock]   clock timer initialized\n");
}

// this is for the PIT
static void timer_isr(int vector, int code)
{
  clock_ticks++;
  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
