#include "cio.h"
#include "x86.h"

#include "dev/keyboard.h"

#include  "support.h"




// input
void cio_init( void ){
        /*
        ** Screen dimensions
        */
        min_x  = SCREEN_MIN_X;
        min_y  = SCREEN_MIN_Y;
        max_x  = SCREEN_MAX_X;
        max_y  = SCREEN_MAX_Y;

        /*
        ** Scrolling region
        */
        scroll_min_x = SCREEN_MIN_X;
        scroll_min_y = SCREEN_MIN_Y;
        scroll_max_x = SCREEN_MAX_X;
        scroll_max_y = SCREEN_MAX_Y;

        /*
        ** Initial cursor location
        */
        curr_y = min_y;
        curr_x = min_x;
        _cio_setcursor();

        cio_clearscreen();

        /*
        ** Set up the interrupt handler for the keyboard
        */
}
void c_setscroll( unsigned int s_min_x, unsigned int s_min_y, unsigned int s_max_x, unsigned int s_max_y ){
	scroll_min_x = bound( min_x, s_min_x, max_x );
	scroll_min_y = bound( min_y, s_min_y, max_y );
	scroll_max_x = bound( scroll_min_x, s_max_x, max_x );
	scroll_max_y = bound( scroll_min_y, s_max_y, max_y );
	curr_x = scroll_min_x;
	curr_y = scroll_min_y;
	_cio_setcursor();
}

void c_moveto( unsigned int x, unsigned int y ){
	curr_x = bound( scroll_min_x, x + scroll_min_x, scroll_max_x );
	curr_y = bound( scroll_min_y, y + scroll_min_y, scroll_max_y );
	_cio_setcursor();
}

void c_clearscroll( void ){
	unsigned int	nchars = scroll_max_x - scroll_min_x + 1;
	unsigned int	l;
	unsigned int	c;

	for( l = scroll_min_y; l <= scroll_max_y; l += 1 ){
		unsigned short *to = VIDEO_ADDR( scroll_min_x, l );

		for( c = 0; c < nchars; c += 1 ){
			*to++ = ' ' | 0x0700;
		}
	}
}

void cio_clearscreen( void )
{
        unsigned short *to = VIDEO_ADDR( min_x, min_y );
        unsigned int    nchars = ( max_y - min_y + 1 ) * ( max_x - min_x + 1 );

        while( nchars > 0 ){
                *to++ = ' ' | 0x0700;
                nchars -= 1;
        }
}

void cio_putchar_at(unsigned int x, unsigned int y, unsigned int c)
{
  if( ( c & 0x7f ) == '\n' ){
    unsigned int    limit;

    /*
    ** If we're in the scroll region, don't let this loop
    ** leave it.  If we're not in the scroll region, don't
    ** let this loop enter it.
    */
    if( x > scroll_max_x ){
      limit = max_x;
    } else if( x >= scroll_min_x ){
      limit = scroll_max_x;
    } else {
      limit = scroll_min_x - 1;
    }

    while( x <= limit ){
      _cio_putchar_at( x, y, ' ' );
      x += 1;
    }
  } else {
    _cio_putchar_at( x, y, c );
  }
}


unsigned int cio_strlen(const char *s)
{
  return _cio_strlen(s);
}


void cio_printf(const char *format, ...)
{
  char **arg = (char **) &format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
      if (c != '%') {
        cio_putchar(c);
      } else {
        char *p;

        c = *format++;
        switch (c) {
        case 'd':
        case 'u':
        case 'x':
          itoa (buf, c, *((int *) arg++));
          p = buf;
          goto string;
          break;
        case 's':
          p = *arg++;
          if (! p)
            p = "(null)";

        string:
          while (*p)
            cio_putchar(*p++);
          break;

        default:
          cio_putchar(*((int *) arg++));
          break;
        }
      }
    }
}

unsigned int cio_print_str(const char *s)
{
  const char *c = s;
  unsigned int x = curr_x, y = curr_y;

  while (*c) {
    cio_putchar(*c);
    c++;
  }

  return (unsigned int) c - (unsigned int) s;
}

void cio_putchar(const char c)
{
  cio_putchar_at(curr_x, curr_y, (unsigned int) c);
	if( curr_y > scroll_max_y ){
		c_scroll( curr_y - scroll_max_y );
		curr_y = scroll_max_y;
	}

  // now increment the position
  if (c == '\n') {
    curr_x = 0;
    curr_y++;
  } else if ((curr_x + 1) >= max_x) {
    // increment column
    curr_x = 0;
    curr_y++;
  } else {
    curr_x++;
  }

  _cio_setcursor();
}

//
// static private functions
//
void itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;

      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);

  /* Terminate BUF. */
  *p = 0;

  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

unsigned int bound( unsigned int min, unsigned int value, unsigned int max ){
	if( value < min )
		value = min;
	if( value > max )
		value = max;
	return value;
}


void _cio_setcursor( void )
{
        unsigned addr;
        unsigned int    y = curr_y;

        if( y > scroll_max_y ){
                y = scroll_max_y;
        }

        addr = (unsigned)( y * SCREEN_X_SIZE + curr_x );

        __outb( 0x3d4, 0xe );
        __outb( 0x3d5, ( addr >> 8 ) & 0xff );
        __outb( 0x3d4, 0xf );
        __outb( 0x3d5, addr & 0xff );
}


unsigned int _cio_strlen(const char *s)
{
  unsigned int len = 0;

  const char *p = s;
  while (*p++ != '\0')
    len++;

  return len;
}

void _cio_putchar_at( unsigned int x, unsigned int y, unsigned int c )
{
	if( curr_y > scroll_max_y ){
		c_scroll( curr_y - scroll_max_y );
		curr_y = scroll_max_y;
	}
  /* If x or y is too big or small, don't do any output. */
  if( x <= max_x && y <= max_y ){
    unsigned short *addr = VIDEO_ADDR( x, y );

    if( c > 0xff ) {
      /* Use the given attributes */
      *addr = (unsigned short)c;
    } else {
      /* Use attributes 0000 0111 (white on black) */
      *addr = (unsigned short)c | 0x0700;
    }
  }
}

// input

unsigned char *_cio_next_char = _cio_input_buf;
unsigned char *_cio_next_space = _cio_input_buf;

unsigned char _cio_getchar()
{
  unsigned char c;

  while ((c = kb_get_code()) == 0)
	;

  switch (c){
  case 0x1B:	// escape
    c = 0;
    break;

  case 0x08:	// escape
    //clear the last character
    if (curr_x == 0) {
      curr_x = max_x;
      curr_y -= 1;
    } else {
      curr_x -= 1;
    }

    unsigned short *last = VIDEO_ADDR(curr_x, curr_y);
    *last = ' ' | 0x0700;

    _cio_setcursor();
    c = 0;
    break;

  default:
    break;
  }

  // still need to deal with escape, ctrl, etc...
  return c;
}
void c_scroll( unsigned int lines ){
	unsigned short *from;
	unsigned short *to;
	int	nchars = scroll_max_x - scroll_min_x + 1;
	int	line, c;

	/*
	** If # of lines is the whole scrolling region or more, just clear.
	*/
	if( lines > scroll_max_y - scroll_min_y ){
		c_clearscroll();
		curr_x = scroll_min_x;
		curr_y = scroll_min_y;
		_cio_setcursor();
		return;
	}

	/*
	** Must copy it line by line.
	*/
	for( line = scroll_min_y; line <= scroll_max_y - lines; line += 1 ){
		from = VIDEO_ADDR( scroll_min_x, line + lines );
		to = VIDEO_ADDR( scroll_min_x, line );
		for( c = 0; c < nchars; c += 1 ){
			*to++ = *from++;
		}
	}

	for( ; line <= scroll_max_y; line += 1 ){
		to = VIDEO_ADDR( scroll_min_x, line );
		for( c = 0; c < nchars; c += 1 ){
			*to++ = ' ' | 0x0700;
		}
	}
}

void _cio_proc_scancode(unsigned char code)
{
  static unsigned char shift = 0;
  static unsigned char ctrl_mask = 0xff;

  switch (code) {
  case 0x2a:
  case 0x36:	/* shift */
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
    if ((code & 0x80) == 0) { 	// explain this...(whats with the high order bits?)
      code = _cio_scan_code[shift][(int)code];
      if (code != '\377') {
        *_cio_next_space = code & ctrl_mask;
        _cio_next_space = _cio_input_buf + (((_cio_next_space - _cio_input_buf)+1) % CIO_IN_BUFSIZE);
      }
    }
  }
}


void _cio_keyboard_isr(int vector, int code)
{
  //cio_printf("KEYBOARD ISR CALLED %x (%d)\n", vector, code);

  // process the character
  _cio_proc_scancode( __inb(KEYBOARD_DATA) );
  __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}
