#include "dev/console.h"
#include "dev/keyboard.h"
#include "support.h"
#include "x86.h"

void c_init()
{
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
  win_scroll_floor = SCREEN_Y_SIZE;
  win_dist_ceiling = 0;
  win_dist_floor = 0;
  win_dist_floor_to_ceiling = BUFFER_ROWS - SCREEN_Y_SIZE;

  // without the foreground color set the cursor will be black and black
  int x, y;
  for (y = 0; y < BUFFER_ROWS; ++y)
    for (x = 0; x < SCREEN_X_SIZE; ++x)
      screen_buffer[y][x] = (unsigned short) ' ' | 0x0700;

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

void c_write(const char *str, int len)
{
  int i;
  for (i = 0; i < len; ++i)
    c_putchar(*str++);
}

void c_putchar(const char c)
{
  if (c == '\t')
    c_printf("    ");
  else
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
    c_win_scroll(curr_y - scroll_max_y);
    curr_y = scroll_max_y;
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
int c_win_scroll(int lines)
{
  int new_offset;

  if (lines < 0) {
    //
    // scroll UP
    int num_lines = lines * -1;
    if (num_lines > win_dist_ceiling)
      num_lines = win_dist_ceiling;

    // now scroll up num_lines
    new_offset = win_offset - num_lines;
    if (new_offset < 0)
      new_offset = BUFFER_ROWS + new_offset;

    win_dist_ceiling -= num_lines;
    lines = num_lines;	// to return how many we scrolled
  } else {
    //
    // scroll DOWN

    new_offset = win_offset + lines;
    if (new_offset >= BUFFER_ROWS)
      new_offset -= BUFFER_ROWS;

    // reset the floor
    if (lines > win_dist_floor) {
      int floor_push = (new_offset + SCREEN_Y_SIZE) - win_scroll_floor;
      win_scroll_floor = new_offset + SCREEN_Y_SIZE;
      win_dist_floor = 0;
      // push ceiling down
      if (floor_push > win_dist_floor_to_ceiling) {
        win_scroll_ceiling = win_scroll_floor;
        win_dist_floor_to_ceiling = 0;
      }
    } else {
      win_dist_floor -= lines;
    }

    win_dist_ceiling += lines;
  }
  win_offset = new_offset;
  c_draw();

  return lines;
}

void c_write_window(unsigned int x, unsigned int y, unsigned short c)
{
  if ((win_offset + y) >= BUFFER_ROWS) {
    int overlap = ((win_offset + y) - BUFFER_ROWS);
    *VIDEO_ADDR(x,y) = screen_buffer[overlap][x] = c;
  } else {
    *VIDEO_ADDR(x,y) = screen_buffer[win_offset + y][x] = c;
  }
}

void c_draw()
{
  static int lines_overwritten = 0;
  int offset = win_offset;

  int rows_to_split = 0;
  int rows_to_print = SCREEN_Y_SIZE;
  if ((win_offset + SCREEN_Y_SIZE) > BUFFER_ROWS) {
    rows_to_split = (SCREEN_Y_SIZE - (BUFFER_ROWS - win_offset));
    rows_to_print = SCREEN_Y_SIZE - rows_to_split;
  }

  int lines_split = 0;
  int lines_copied = 0;
  int overwrite; 	// flag to know when weve overitten a line before
  int split;		// similar
  int copied;
  int y, x;
  for (y = 0; y < SCREEN_Y_SIZE; ++y) {
    for (x = 0, copied =0, overwrite = 0, split = 0; x < SCREEN_X_SIZE; ++x) {
      if (y < rows_to_print) {
        *VIDEO_ADDR(x,y) = screen_buffer[ win_offset + y ][x];
      } else {
        if (lines_copied < lines_overwritten) {
          *VIDEO_ADDR(x,y) = screen_buffer[ y - rows_to_print ][x];
          if (!copied)
            copied = 1;
        } else {
          // clear everything else
          *VIDEO_ADDR(x,y) = screen_buffer[ y - rows_to_print ][x] = ' ' | 0x0700;
          if (!overwrite)
            overwrite = 1;
        }
      }
    } // the line

    if (copied)
      lines_copied++;
    if (overwrite)
      lines_overwritten++;
  }
  c_setcursor();
}

// this is all we need by design, let the shell handle the characters
uint16_t c_getcode()
{
  return kb_wait_code();
}


