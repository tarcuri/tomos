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

// functions

void cio_init(void);
void cio_clearscreen(void);

void cio_putchar(const char c);
void cio_putchar_at(unsigned int x, unsigned int y, unsigned int c);

unsigned int cio_strlen(const char *s);
unsigned int cio_print_str(const char *s);	// print null-terminated string to current position

// static private functions

static unsigned int _cio_strlen(const char *s);
static void _cio_setcursor(void);
static void _cio_putchar_at(unsigned int x, unsigned int y, unsigned int c);

#endif
