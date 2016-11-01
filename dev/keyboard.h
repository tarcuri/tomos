#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEYBOARD_ESCAPE		0x01
#define KEYBOARD_BACKSPACE	0x0E


#define KEYBOARD_DATA   0x60
#define KEYBOARD_STATUS 0x64
#define EOT             '\04'

// define an active input handle
// eventually the globally active process will be associated with this
// to do so we need to reference the current process...

// buffers will always be circular, 4KB pages
#define KEYBOARD_BUFSIZE	(0x2000)

#define KB_ESCAPE_SC		0xE0

#define KSC_LCTRL		0x1D
#define KSC_LSHIFT		0x2A
#define KSC_RSHIFT		0x36

// escape scancodes
#define KSC_ESC_ENTER		0x1C
#define KSC_ESC_RCTRL		0x1D
#define KSC_ESC_KP_FSLASH	0x35
#define KSC_ESC_PRT_SCREEN	0x37
#define KSC_ESC_RALT		0x38
#define KSC_ESC_BREAK		0x46
#define KSC_ESC_HOME		0x47
#define KSC_ESC_UP		0x48
#define KSC_ESC_PAGE_UP		0x49
#define KSC_ESC_LEFT		0x4B
#define KSC_ESC_RIGHT		0x4C
#define KSC_ESC_END		0x4F
#define KSC_ESC_DOWN		0x50
#define KSC_ESC_PAGE_DOWN	0x51
#define KSC_ESC_INSERT		0x52
#define KSC_ESC_DELETE		0x53

// tomos key codes
#define KCODE_ENTER		(KSC_ESC_ENTER + 0x100)
#define KCODE_RCTRL		(KSC_ESC_RCTRL + 0x100)
#define KCODE_KP_FSLASH		(KSC_ESC_KP_FSLASH + 0x100)
#define KCODE_PRT_SCREEN	(KSC_ESC_PRT_SCREEN + 0x100)
#define KCODE_RALT		(KSC_ESC_RALT + 0x100)
#define KCODE_BREAK		(KSC_ESC_BREAK + 0x100)
#define KCODE_HOME		(KSC_ESC_HOME + 0x100)
#define KCODE_UP		(KSC_ESC_UP + 0x100)
#define KCODE_PAGE_UP		(KSC_ESC_PAGE_UP + 0x100)
#define KCODE_LEFT		(KSC_ESC_LEFT + 0x100)
#define KCODE_RIGHT		(KSC_ESC_RIGHT + 0x100)
#define KCODE_END		(KSC_ESC_END + 0x100)
#define KCODE_DOWN		(KSC_ESC_DOWN + 0x100)
#define KCODE_PAGE_DOWN		(KSC_ESC_PAGE_DOWN + 0x100)
#define KCODE_INSERT		(KSC_ESC_INSERT + 0x100)
#define KCODE_DELETE		(KSC_ESC_DELETE + 0x100)

// active buffer to store keycodes and/or scancodes
uint16_t *kb_active_buffer;

int kb_write_idx;
int kb_read_idx;

static uint16_t kcode_table[3][128] = {
  {
// DEFAULT
/* 00-07 */     '\377', '\033', '1',    '2',    '3',    '4',    '5',    '6',
/* 08-0f */     '7',    '8',    '9',    '0',    '-',    '=',    '\b',   '\t',
/* 10-17 */     'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
/* 18-1f */     'o',    'p',    '[',    ']',    '\n',   '\377', 'a',    's',
/* 20-27 */     'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
/* 28-2f */     '\'',   '`',    '\377', '\\',   'z',    'x',    'c',    'v',
/* 30-37 */     'b',    'n',    'm',    ',',    '.',    '/',    '\377', '*',
/* 38-3f */     '\377', ' ',    '\377', '\377', '\377', '\377', '\377', '\377',
/* 40-47 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '7',
/* 48-4f */     '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
/* 50-57 */     '2',    '3',    '0',    '.',    '\377', '\377', '\377', '\377',
/* 58-5f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 60-67 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 68-6f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 70-77 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 78-7f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377'
  },
// SHIFT
  {
/* 00-07 */     '\377', '\033', '!',    '@',    '#',    '$',    '%',    '^',
/* 08-0f */     '&',    '*',    '(',    ')',    '_',    '+',    '\b',   '\t',
/* 10-17 */     'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
/* 18-1f */     'O',    'P',    '{',    '}',    '\n',   '\377', 'A',    'S',
/* 20-27 */     'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
/* 28-2f */     '"',    '~',    '\377', '|',    'Z',    'X',    'C',    'V',
/* 30-37 */     'B',    'N',    'M',    '<',    '>',    '?',    '\377', '*',
/* 38-3f */     '\377', ' ',    '\377', '\377', '\377', '\377', '\377', '\377',
/* 40-47 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '7',
/* 48-4f */     '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
/* 50-57 */     '2',    '3',    '0',    '.',    '\377', '\377', '\377', '\377',
/* 58-5f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 60-67 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 68-6f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 70-77 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 78-7f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377'
  },
// ESCAPE
  {
/* 00-07 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 08-0f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 10-17 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 18-1b */     '\377', '\377', '\377', '\377', 
/* 1C    */	KCODE_ENTER,
/* 1D,E,F*/	KCODE_RCTRL, '\377', '\377',
/* 20-27 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 28-2f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 30-34 */     '\377', '\377', '\377', '\377', '\377', 
/* 35-37 */     KCODE_KP_FSLASH, '\377', KCODE_PRT_SCREEN,
/* 38-3f */     KCODE_RALT,'\377','\377','\377','\377', '\377', '\377', '\377',
/* 40-45 */     '\377', '\377', '\377', '\377', '\377', '\377', 
/* 46-49 */	KCODE_BREAK, KCODE_HOME, KCODE_UP, KCODE_PAGE_UP,
/* 48-4a */     '\377',    '\377',    '\377',
/* 4b-4f */	KCODE_LEFT, KCODE_RIGHT, KCODE_END,
/* 50-53 */     KCODE_DOWN, KCODE_PAGE_DOWN, KCODE_INSERT, KCODE_DELETE,
/* 54-57 */	'\377', '\377', '\377', '\377',
/* 58-5f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 60-67 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 68-6f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 70-77 */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377',
/* 78-7f */     '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377'
  }
};

uint16_t kb_wait_code();
uint16_t kb_get_code();

static void kb_insert_code(uint16_t code);
static void kb_scancode(uint8_t code);
static void kb_handler(int vector, int code);

#endif
