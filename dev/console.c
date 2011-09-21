#include "dev/console.h"
#include "dev/keyboard.h"
#include "support.h"
#include "x86.h"

void c_init( void ){
  // Screen dimensions
  min_x  = SCREEN_MIN_X;
  min_y  = SCREEN_MIN_Y;
  max_x  = SCREEN_MAX_X;
  max_y  = SCREEN_MAX_Y;

  // Scrolling region
  scroll_min_x = SCREEN_MIN_X;
  scroll_min_y = SCREEN_MIN_Y;
  scroll_max_x = SCREEN_MAX_X;
  scroll_max_y = SCREEN_MAX_Y;

  // Initial cursor location
  curr_y = min_y;
  curr_x = min_x;
  c_setcursor();

  // configure the scroll window
  win_offset = 0;
  win_scroll_ceiling = 0;

  c_clearscreen();
}

//
// i/o functions: to be replaced with libc
//
void c_printf(const char *format, ...)
{
  char **arg = (char **) &format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
      if (c != '%') {
        c_putchar(c);
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
            c_putchar(*p++);
          break;

        default:
          c_putchar(*((int *) arg++));
          break;
        }
      }
    }
}

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

unsigned int c_strlen(const char *s)
{
  unsigned int len = 0;

  const char *p = s;
  while (*p++ != '\0')
    len++;

  return len;
}

char *c_getstr(char *s, unsigned int n)
{
  char c;
  int i;

  for (i = 0; i < n; ++i) {
    c = c_getcode();
    if (c) {
      s[i] = c;
    } else {
      s[i] = '\0';
      break;
    }
  }

  return s;
}

char *c_getline(char *s, unsigned int n)
{
  char c;
  int i;

  for (i = 0; i < n; ++i) {
    c = c_getcode();
      
    if (!c || (c == '\n')) {
      s[i] = '\0';
      break;
    }

    s[i] = c;
  }

  return s;
}

//
// Console functions
//
void c_moveto(unsigned int x, unsigned int y)
{
  curr_x = c_bound( scroll_min_x, x + scroll_min_x, scroll_max_x );
  curr_y = c_bound( scroll_min_y, y + scroll_min_y, scroll_max_y );
  c_setcursor();
}

void c_scroll(unsigned int lines)
{
  unsigned short *from;
  unsigned short *to;
  int	nchars = scroll_max_x - scroll_min_x + 1;
  int	line, c;

  // If # of lines is the whole scrolling region or more, just clear.
  if( lines > scroll_max_y - scroll_min_y ){
    c_clearscroll();
    curr_x = scroll_min_x;
    curr_y = scroll_min_y;
    c_setcursor();
    return;
  }

  // Must copy it line by line.
  for( line = scroll_min_y; line <= scroll_max_y - lines; line += 1 ){
    from = VIDEO_ADDR( scroll_min_x, line + lines );
    to = VIDEO_ADDR( scroll_min_x, line );
    for(c = 0; c < nchars; c += 1)
      *to++ = *from++;
  }

  for( ; line <= scroll_max_y; line += 1){
    to = VIDEO_ADDR(scroll_min_x, line);
    for(c = 0; c < nchars; c += 1)
      *to++ = ' ' | 0x0700;
  }
}

void c_setscroll(unsigned int s_min_x, unsigned int s_min_y, unsigned int s_max_x, unsigned int s_max_y)
{
  scroll_min_x = c_bound( min_x, s_min_x, max_x );
  scroll_min_y = c_bound( min_y, s_min_y, max_y );
  scroll_max_x = c_bound( scroll_min_x, s_max_x, max_x );
  scroll_max_y = c_bound( scroll_min_y, s_max_y, max_y );
  curr_x = scroll_min_x;
  curr_y = scroll_min_y;
  c_setcursor();
}

void c_clearscreen(void)
{
   unsigned short *to = VIDEO_ADDR( min_x, min_y );
   unsigned int    nchars = (max_y - min_y + 1) * (max_x - min_x + 1);

   while( nchars > 0 ){
     *to++ = ' ' | 0x0700;
     nchars -= 1;
   }
}

void c_clearscroll(void)
{
  unsigned int	nchars = scroll_max_x - scroll_min_x + 1;
  unsigned int	l;
  unsigned int	c;

  for(l = scroll_min_y; l <= scroll_max_y; l += 1){
    unsigned short *to = VIDEO_ADDR(scroll_min_x, l);

    for(c = 0; c < nchars; c += 1)
      *to++ = ' ' | 0x0700;
  }
}

// this should be a macro
unsigned int c_bound(unsigned int min, unsigned int value, unsigned int max)
{
  if(value < min)
  	value = min;
  if(value > max)
  	value = max;
  return value;
}

void c_setcursor( void )
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

void c_putchar(const char c)
{
  c_putchar_at(curr_x, curr_y, (unsigned int) c);
}

void c_putchar_at(unsigned int x, unsigned int y, unsigned int c)
{
  int echo = 1;

  switch (c) {
  case '\n':
    curr_y++;
    curr_x = 0;

    echo = 0;
    c_setcursor();
    break;
  case '\b':
    // erase the previous character
    if (x) {
      curr_x = x - 2;
      curr_y = y;
      c_setcursor();
      c_putchar_at(x - 1, y, ' ');
      return;
    } else {
      // need to backspace to previous line
    }
    break;
  };

  if( curr_y > scroll_max_y ){
    //c_scroll( curr_y - scroll_max_y );
    c_win_scroll(curr_y - scroll_max_y);
    curr_y = scroll_max_y;
    //echo = 0;
  }

  /* If x or y is too big or small, don't do any output. */
  if(echo && x <= max_x && y <= max_y){
    if(c > 0xff) {
      /* Use the given attributes */
      c_write_window(x, y, (unsigned short) c);
    } else {
      /* Use attributes 0000 0111 (white on black) */
      c_write_window(x, y, (unsigned short) c | 0x0700);
    }

    curr_x++;
    if (curr_x > max_x) {
      curr_x = 0;
      curr_y++;
    }
    c_setcursor();
  }
}

// window functions

// we can always scroll down, clearing data if needed
// only can scroll up until we reach the ceiling
void c_win_scroll(int lines)
{
  int target = win_offset + lines;

  if (lines < 0) {		// scroll up
    if (win_offset == win_scroll_ceiling) {
      // do nothing
    } else if (win_offset > win_scroll_ceiling) {
      win_offset = (target > win_scroll_ceiling) ? (target) : (win_scroll_ceiling);
    } else {
      // ceiling is 'below' the window
      if (target < 0) { 	// past top of buffer
        win_offset = ((BUFFER_ROWS + target) >= win_scroll_ceiling)
                         ? (BUFFER_ROWS + target) : (win_scroll_ceiling);
      } else {
        win_offset = target;
      }
    }
  } else {			// scroll down
    // always scroll down, moving the ceiling if neccessary
    int win_bottom = target + SCREEN_MAX_Y;

    if (win_offset >= win_scroll_ceiling) {
      // ceiling is 'above'
      if (win_bottom >= BUFFER_ROWS) {
        if (target >= BUFFER_ROWS) {
          target = target - BUFFER_ROWS;
          win_bottom = target + SCREEN_MAX_Y;
          if (win_bottom > win_scroll_ceiling)
            win_scroll_ceiling = win_bottom;
        } else {
          int overlap = SCREEN_MAX_Y - (BUFFER_ROWS - target);
          if (overlap > win_scroll_ceiling)
            win_scroll_ceiling = overlap;
        }
      }
      win_offset = target;
    } else {
      // ceiling is 'below'
      if (win_bottom < BUFFER_ROWS) {
        if (win_bottom > win_scroll_ceiling)
          win_scroll_ceiling = win_bottom;
      } else {
        if (target >= BUFFER_ROWS) {
          target = target - BUFFER_ROWS;
          win_bottom = target + SCREEN_MAX_Y;
          if (win_bottom > win_scroll_ceiling)
            win_scroll_ceiling = win_bottom;
        } else {
          win_scroll_ceiling = SCREEN_MAX_Y - (BUFFER_ROWS - target);
        }
      } // do we hit the bottom?
    } // is ceiling above or below?

    // clear lines rows from the bottom of the new window
    int x, y;
    for (y = SCREEN_MAX_Y; y > (SCREEN_MAX_Y - lines); --y)
      for (x = 0; x < SCREEN_X_SIZE; ++x)
        *VIDEO_ADDR(x, y) = ' ' | 0x0700;
    
  } // up or down?

  //c_printf("offset: %d\n", win_offset);
  c_draw();
}

void c_write_window(unsigned int x, unsigned int y, unsigned short c)
{
  if ((win_offset + y) > BUFFER_ROWS) {		// need to write at the top of the buffer
    int overlap = ((win_offset + y) - BUFFER_ROWS) - 1;
    screen_buffer[0 + overlap][x] = c;
  } else {
    screen_buffer[win_offset + y][x] = c;
  }

  // update the screen
  *VIDEO_ADDR(x,y) = WIN(x,y);
}

void c_draw()
{
  int y, x;
  for (y = 0; y < SCREEN_Y_SIZE; ++y)
    for (x = 0; x < SCREEN_X_SIZE; ++x)
      *VIDEO_ADDR(x,y) = WIN(x,y);

  c_setcursor();
}

// this is all we need by design, let the shell handle the characters
unsigned char c_getcode()
{
  return kb_wait_code();
}


