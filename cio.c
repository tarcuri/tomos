#include "cio.h"

#include  "support.h"

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
        //_install_isr( INT_VEC_KEYBOARD, __c_keyboard_isr );
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

static void
itoa (char *buf, int base, int d)
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


static void _cio_setcursor( void )
{
        unsigned addr;
        unsigned int    y = curr_y;

        if( y > scroll_max_y ){
                y = scroll_max_y;
        }

        addr = (unsigned)( y * SCREEN_X_SIZE + curr_x );

        sp_outb( 0x3d4, 0xe );
        sp_outb( 0x3d5, ( addr >> 8 ) & 0xff );
        sp_outb( 0x3d4, 0xf );
        sp_outb( 0x3d5, addr & 0xff );
}


static unsigned int _cio_strlen(const char *s)
{
  unsigned int len = 0;

  const char *p = s;
  while (*p++ != '\0')
    len++;

  return len;
}

static void _cio_putchar_at( unsigned int x, unsigned int y, unsigned int c )
{
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

