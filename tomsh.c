#include "dev/console.h"
#include "tomsh.h"
#include "kernel/mm.h"
#include "kernel/heap.h"

// we need many string functions,
// strstr, strtok, etc
void command_loop()
{
  strcpy(prompt, "tomsh $", 8);
  int scroll = 0;

  char *buffer;
  while (1) {
    if (!scroll) {
      c_printf("%s ", prompt);
      //c_setcursor();
    }

    unsigned char c;
    int i = 0;
    int cmd_i = 0;

    for (cmd_i = 0; i < 512; ) {
      c = c_getcode();

      if (scroll && (c != 0x39)) {
        scroll = 0;
        c_setcursor();
      }

      switch (c) {
      case '\n':
        command_line[cmd_i++] = '\0';
        c_putchar('\n');
        break;
      case 0x1B:	// escape
        // nothing yet
        break; 
      case '\b':	// backspace
        // none for now
        if (cmd_i) {
          c_putchar(c);
          cmd_i--;
        }
        break;
      case 0x33:	// page down
        //c_win_scroll(1);
        //scroll = 1;
        break;
      case 0x39:	// page up
        c_win_scroll(-1);
        scroll = 1;
        break;
      default:
        c_putchar(c);
        command_line[cmd_i++] = c;
      };

      if (c == '\n')
        break;
    }

    if (strncmp(command_line, "dispheap", 8) == 0)
      dump_heap_index(k_heap);
    else if (!scroll)
      c_printf("> %s\n", command_line);
  }
}
