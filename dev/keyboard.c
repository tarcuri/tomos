#include "keyboard.h"
#include "support.h"
#include "x86.h"

#include "dev/console.h"
#include "kernel/heap.h"
#include "kernel/scheduler.h"

// eventually, this will know the pid of the current process, and its state
void kb_init()
{
  // get scanecode set
  __outb(KEYBOARD_DATA, 0xF0);
  if (__inb(KEYBOARD_DATA) == 0xFA) 	// ACK
    __outb(KEYBOARD_DATA, 0x0);
  uint8_t scancode_set = __inb(KEYBOARD_DATA);
  c_printf("[kb]      keyboard using scancode set 0x%x\n", scancode_set);

  // allocate space for the kernels input buffer
  // other processes will get their own

  // duh, using the same frame as the heap
  uint32_t phys;
  kb_active_buffer = (uint16_t *) kmalloc_p(KEYBOARD_BUFSIZE, 0, &phys);

  kb_write_idx = 0;
  kb_read_idx = 0;

  _install_isr(INT_VEC_KEYBOARD, kb_handler);

  c_printf("[kb]      keyboard driver initialized\n");
}

uint16_t kb_wait_code()
{
  uint16_t c = 0;

  // always assume we haven't overflowed
  current_proc->status = KB_WAIT;
  // dispatch
  asm volatile ("int  %0" : : "N" (INT_VEC_TIMER) : "cc", "memory");

  // need a better way to do this, after all we're
  // (trying) to write an OS here.
  while (kb_read_idx == kb_write_idx)
	;

  c = kb_active_buffer[kb_read_idx++];
  kb_read_idx %= KEYBOARD_BUFSIZE;

  return c;
}

uint16_t kb_get_code()
{
  uint16_t c = 0;

  // always assume we haven't overflowed
  if (kb_read_idx != kb_write_idx) {
    c = kb_active_buffer[kb_read_idx++];
    kb_read_idx %= KEYBOARD_BUFSIZE;
  }

  return c;
}

static void kb_insert_code(uint16_t code)
{
  kb_active_buffer[kb_write_idx++] = code;
  kb_write_idx %= KEYBOARD_BUFSIZE;
}

static void kb_scancode(uint8_t code)
{
  static uint8_t shift = 0;
  static uint8_t ctrl_mask = 0xff;
  static uint8_t escaped = 0;

  uint16_t kcode = code;

  if (escaped)
    kcode += 0x100;

  switch (code) {
  case KSC_LSHIFT:
  case KSC_RSHIFT:
    shift = 1;
    break;

  case 0xAA:            // left shift released
  case 0xB6:            // right shift released
    shift = 0;
    break;

  case KSC_LCTRL:	// need to handle escape RCTRLs
    ctrl_mask = 0x1F;
    break;

  case KB_ESCAPE_SC:
    escaped = 2;	// use to index kcode_table
    break;

  default:
    // for now, escape trumps shift
    if (kcode & 0x80) {
      if (escaped)
        escaped = 0;
    } else if (escaped) {
      kcode = kcode_table[escaped][code];
      kb_insert_code(kcode);

      escaped = 0;
    } else {
      kcode = kcode_table[shift][code];
      kb_insert_code(kcode);
    }
  }
}


static void kb_handler(int vector, int code)
{
  kb_scancode( __inb(KEYBOARD_DATA) );
  set_kb_ready();
  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
