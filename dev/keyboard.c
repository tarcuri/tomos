#include "keyboard.h"
#include "support.h"
#include "x86.h"

#include "dev/console.h"
#include "kernel/heap.h"

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
  kb_active_buffer = (uint8_t *) kmalloc(KEYBOARD_BUFSIZE, 0);

  //kb_active_buffer = (uint8_t *) mm_alloc_frame();

  kb_write_idx = 0;
  kb_read_idx = 0;

  _install_isr(INT_VEC_KEYBOARD, kb_handler);

  c_printf("[kb]      keyboard driver initialized\n");
}

int kb_read(uint8_t *buf, int n)
{
  n = (n > KEYBOARD_BUFSIZE) ? (KEYBOARD_BUFSIZE) : (n);

  int i;
  for (i = 0; i < n; ++i)
    *buf++ = kb_get_code();

  return n;
}

uint8_t kb_wait_code()
{
  uint16_t c = 0;

  // always assume we haven't overflowed
  while (kb_read_idx == kb_write_idx)
	;

  c = kb_active_buffer[kb_read_idx++];
  kb_read_idx %= KEYBOARD_BUFSIZE;

  return c;
}

uint8_t kb_get_code()
{
  uint8_t c = 0;

  // always assume we haven't overflowed
  if (kb_read_idx != kb_write_idx) {
    c = kb_active_buffer[kb_read_idx++];
    kb_read_idx %= KEYBOARD_BUFSIZE;
  }

  return c;
}

static void kb_insert_code(uint8_t code)
{
  kb_active_buffer[kb_write_idx++] = code;
  kb_write_idx %= KEYBOARD_BUFSIZE;
}

static void kb_scancode(uint8_t code)
{
  static uint8_t shift = 0;
  static uint8_t ctrl_mask = 0xff;
  static uint8_t escaped = 0;
  c_printf(" C: [%x] ", code);

  if (escaped) {
    code += 0x100;
    escaped = 0;
  }

  switch (code) {
  case KEYBOARD_L_SHIFT:
  case KEYBOARD_R_SHIFT:
    shift = 1;
    break;

  case 0xAA:
    shift = 0;
    break;

  case 0x1D:
    ctrl_mask = 0x1F;
    break;

  case 0xE0:	// escape codes
    escaped = 1;
    break;

  default:
    if (code & 0x80) {
      // ignore break code
    } else {
      code = kb_scancode_table[shift][(int)code];
      kb_insert_code(code);
    }
  }
}


static void kb_handler(int vector, int code)
{
  kb_scancode( __inb(KEYBOARD_DATA) );
  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
