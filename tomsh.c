#include "dev/console.h"
#include "dev/keyboard.h"
#include "tomsh.h"
#include "kernel/mm.h"
#include "kernel/heap.h"

#include "syscalls.h"

//#include "dev/device.h"
#include "dev/ata.h"
#include "fs/ext2.h"

//#include "fs/vfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//void ls_dir(int ino);
//void hdd_read(unsigned int lba, void *buf, unsigned int len);

// we need many string functions,
// strstr, strtok, etc
void command_loop()
{
  printf("entered the command loop\n");
  strncpy(prompt, "tomsh $", 8);
  int scroll = 0;
  int lines_up = 0;

  while (1) {
    if (!scroll) {
      printf("%s ", prompt);
      fflush(stdout);
    }

    uint16_t c;
    int cmd_i = 0;

    while (cmd_i < 512) {
      //read(1, &c, 1);
      c = c_getcode();

      if (scroll && (c != KCODE_PAGE_DOWN) && (c != KCODE_PAGE_UP)) {
        c_win_scroll(lines_up);
        lines_up = 0;
        scroll = 0;
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
      case KCODE_PAGE_DOWN:
        if (lines_up) {
          lines_up -= c_win_scroll(1);
          scroll = 1;
        }
        break;
      case KCODE_PAGE_UP:
        lines_up += c_win_scroll(-1);
        scroll = 1;
        break;
      default:
        c_putchar(c);
        command_line[cmd_i++] = c;
      };

      if (c == '\n') {
        scroll = 0;
        break;
      }
    }


    if (strncmp(command_line, "dispheap", 8) == 0) {
      dump_heap_index(k_heap);
    } else if (strncmp(command_line, "getpid", 6) == 0) {
      c_printf("PID: %d\n", getpid());
    } else if (strncmp(command_line, "idhdd", 5) == 0) {
      ata_identify_device();
    } else if (strncmp(command_line, "test_ata", 8) == 0) {
      test_ata();
    } else if (strncmp(command_line, "ls ", 3) == 0) {
      ls_dir(atoi(&command_line[3]));
    } else if (strncmp(command_line, "cat ", 4) == 0) {
      cat_file(atoi(&command_line[4]));
		} else if (strncmp(command_line, "help", 4) == 0) {
		  c_printf("tomsh commands:\n");
			c_printf("\tdispheap\n");
			c_printf("\tgetpid\n");
			c_printf("\tidhdd\n");
			c_printf("\ttestata\n");
			c_printf("\tls\n");
			c_printf("\tcat\n");
    } else if (!scroll && strlen(command_line)) {
      c_printf("> %s\n", command_line);
		} else {
			c_printf("type \"help\" for a list of commands.\n");
		}

    memcpy(command_line, 0, 512);
  }
}
