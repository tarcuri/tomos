#include "dev/console.h"
#include "tomsh.h"
#include "mm.h"

// we need many string functions,
// strstr, strtok, etc
void command_loop()
{
  strcpy(prompt, "tomsh $", 8);

  while (1) {
    c_printf("%s ", prompt);

    unsigned char c;
    int i = 0;
    int cmd_i = 0;

    for (cmd_i = 0; i < 512; ) {
      c = c_getcode();

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
      default:
        c_putchar(c);
        command_line[cmd_i++] = c;
      };

      if (c == '\n')
        break;
    }

    c_printf("> %s\n", command_line);
  }
}