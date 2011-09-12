#ifndef C_IO_H
#define C_IO_H

#define	VIDEO_BASE_ADDR		0xB8000
#define VIDEO_ADDR(x,y)		(unsigned short *)(VIDEO_BASE_ADDR + 2 * ((y) * SCREEN_X_SIZE + (x)))

#define SCREEN_MIN_X    0
#define SCREEN_MIN_Y    0
#define SCREEN_X_SIZE   80
#define SCREEN_Y_SIZE   25
#define SCREEN_MAX_X    ( SCREEN_X_SIZE - 1 )
#define SCREEN_MAX_Y    ( SCREEN_Y_SIZE - 1 )

unsigned int scroll_min_x;
unsigned int scroll_min_y;
unsigned int scroll_max_x;
unsigned int scroll_max_y;
unsigned int min_x;
unsigned int min_y;
unsigned int max_x;
unsigned int max_y;

unsigned int curr_x;
unsigned int curr_y;

// functions

void c_setscroll( unsigned int s_min_x, unsigned int s_min_y, unsigned int s_max_x, unsigned int s_max_y );

void c_moveto( unsigned int x, unsigned int y );

void c_clearscroll( void );

void cio_init(void);
void cio_clearscreen(void);

void cio_putchar(const char c);
void cio_putchar_at(unsigned int x, unsigned int y, unsigned int c);

unsigned int cio_strlen(const char *s);
unsigned int cio_print_str(const char *s);	// print null-terminated string to current position

// input
#define CIO_IN_BUFSIZE	1024
#define KEYBOARD_DATA   0x60
#define KEYBOARD_STATUS 0x64
#define READY           0x1
#define EOT             '\04'

static unsigned char _cio_scan_code[ 2 ][ 128 ] = {
        {
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
        }
};


unsigned char _cio_input_buf[CIO_IN_BUFSIZE];

void c_scroll( unsigned int lines );


// static private functions
void itoa(char *, int, int);
unsigned char _cio_getchar(void);
void _cio_proc_scancode(unsigned char);
void _cio_keyboard_isr(int vector, int code);
unsigned int _cio_strlen(const char *s);
void _cio_setcursor(void);
void _cio_putchar_at(unsigned int x, unsigned int y, unsigned int c);
unsigned int bound( unsigned int min, unsigned int value, unsigned int max );

#endif
