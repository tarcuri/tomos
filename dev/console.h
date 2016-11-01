#ifndef CONSOLE_H
#define CONSOLE_H

#define	VIDEO_BASE_ADDR		0xB8000
#define VIDEO_ADDR(x,y)		(unsigned short *)(VIDEO_BASE_ADDR + 2 * ((y) * SCREEN_X_SIZE + (x)))

#define SCREEN_MIN_X    0
#define SCREEN_MIN_Y    0
#define SCREEN_X_SIZE   80
#define SCREEN_Y_SIZE   25
#define SCREEN_MAX_X    ( SCREEN_X_SIZE - 1 )
#define SCREEN_MAX_Y    ( SCREEN_Y_SIZE - 1 )

#define NUM_WINDOW_BUFFERS	32	// 124K for screen buffers
#define BUFFER_ROWS		(NUM_WINDOW_BUFFERS * SCREEN_Y_SIZE)

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

int view_offset;


// functions
void		c_init(void);

/**
 * Use the idea of a scrolling window that is a view over a screen buffer.
 * The screen buffer is an array of MAX_Y * NUM_WINDOW_BUFFERS screen rows.
 * The window is the size of the console. If the window scrolls partially or
 * completely past a buffer boundary, the window offset is moved up or down the
 * buffer, and the old rows are referenced from the other (previous) end of the buffer.
 */
unsigned short screen_buffer[BUFFER_ROWS][SCREEN_X_SIZE];
#define WIN(x,y)	screen_buffer[win_offset + y][x]

// The window offset will always be a valid index into the screen buffer,
// if the window overlaps a boundary, the overlapping rows are referenced from
// the other end of the buffer.
unsigned int win_offset;
unsigned int win_scroll_ceiling;	// can never scroll up past the ceiling
unsigned int win_scroll_floor;		// overwrite data when pushing this down to the ceiling
unsigned int win_dist_ceiling;		// distance from window offset to ceiling
unsigned int win_dist_floor;		// distance from window bottom to floor 
unsigned int win_dist_floor_to_ceiling;	// distance from floor to ceiling


/**
 * Scroll the window up/down the screen buffer, partially overlapping a boundary if necessary.
 * If the scroll overlaps a boundary, move the window offset to the other end of the buffer
 * and reference the old rows from the old end of the buffer.
 */
int c_win_scroll(int lines);

/**
 * Write a value to the screen buffer at a given offset. The offset references an area
 * the size of the screen. This function abstracts the buffer boundarys that come into play.
 */
void c_write_window(unsigned int x, unsigned int y, unsigned short c);

/**
 * Clear the window in the screen buffer referenced by win_offest, then re-draw the screen.
 */
//void c_clear_window(void);

/**
 * Draw the current window in the screen buffer to the display.
 */
void c_draw(void);

// other
unsigned int	c_strlen(const char *s);
unsigned int	c_print_str(const char *s);
void		itoa(char *, int, int);
char *		c_getstr(char *s, unsigned int n);
char *		c_getline(char *s, unsigned int n);

void		c_moveto(unsigned int x, unsigned int y);
void		c_scroll( unsigned int lines );
void		c_setscroll( unsigned int min_x, unsigned int min_y, unsigned int max_x, unsigned int max_y );
void		c_clearscreen(void);
void		c_clearscroll( void );

unsigned int	c_bound( unsigned int min, unsigned int value, unsigned int max );
void		c_setcursor(void);

void		c_write(const char *str, int len);
void		c_putchar(const char c);
void		c_putchar_at(unsigned int x, unsigned int y, unsigned int c);

unsigned short	c_getcode(void);

// input
#define CIO_IN_BUFSIZE	1024
#define EOT             '\04'

static unsigned char c_scan_code[ 2 ][ 128 ] = {
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


#endif
