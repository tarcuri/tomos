#include "keyboard.h"
#include "support.h"
#include "x86.h"

#include "dev/console.h"
#include "kheap.h"

// eventually, this will know the pid of the current process, and its state
void kb_init()
{
  // allocate space for the kernels input buffer
  // other processes will get their own

  // duh, using the same frame as the heap
  kb_active_buffer = (unsigned char *) kmalloc(KEYBOARD_BUFSIZE, 0, &kernel_heap);

  //kb_active_buffer = (unsigned char *) mm_alloc_frame();

  kb_write_idx = 0;
  kb_read_idx = 0;

  _install_isr(INT_VEC_KEYBOARD, kb_handler);

  c_printf("[kb]      keyboard driver initialized\n");
}

int kb_read(unsigned char *buf, int n)
{
  n = (n > KEYBOARD_BUFSIZE) ? (KEYBOARD_BUFSIZE) : (n);

  int i;
  for (i = 0; i < n; ++i)
    *buf++ = kb_get_code();

  return n;
}

unsigned char kb_get_code()
{
  unsigned char c = 0;

  // always assume we haven't overflowed
  if (kb_read_idx != kb_write_idx) {
    c = kb_active_buffer[kb_read_idx++];
    kb_read_idx %= KEYBOARD_BUFSIZE;
  }

  return c;
}

static void kb_insert_code(unsigned char code)
{
  kb_active_buffer[kb_write_idx++] = code;
  kb_write_idx %= KEYBOARD_BUFSIZE;
}

static void kb_scancode(unsigned char code)
{
  static unsigned char shift = 0;
  static unsigned char ctrl_mask = 0xff;

  switch (code) {
  case KEYBOARD_L_SHIFT:
  case KEYBOARD_R_SHIFT:
    shift = 1;
    break;

  case 0xaa:
  case 0xb6:
    shift = 0;
    break;

  case 0x1d:
    ctrl_mask = 0x1f;
    break;

  default:
    if ((code & 0x80) == 0) { 	// all 'normal' keys are 0x7F and below
      code = kb_scancode_table[shift][(int)code];
      kb_insert_code(code);
    }
  }
}


static void kb_handler(int vector, int code)
{
  // process the character
  kb_scancode( __inb(KEYBOARD_DATA) );
  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
