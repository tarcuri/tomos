#include "dev/console.h"
#include "tomsh.h"
#include "kernel/mm.h"
#include "kernel/heap.h"

#include "syscalls.h"

#include "dev/device.h"
#include "dev/ata.h"

#include <stdio.h>
#include <stdlib.h>

static void hdd_read(unsigned int lba, void *buf, unsigned int len);

// we need many string functions,
// strstr, strtok, etc
void command_loop()
{
  int val = atoi("56");
  c_printf("%d\n", val);
  strcpy(prompt, "tomsh $", 8);
  int scroll = 0;
  int lines_up = 0;

  while (1) {
    if (!scroll) {
      printf("%s ", prompt);
      fflush(0);
      //c_setcursor();
    }
    //c_printf("%x", *VIDEO_ADDR(0, curr_y));

    unsigned char c;
    int i = 0;
    int cmd_i = 0;

    for (cmd_i = 0; i < 512; ) {
      //read(1, &c, 1);
      c = c_getcode();

      if (scroll && ((c != 0x39) || (c != 0x33))) {
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
      case 0x33:	// page down
        if (lines_up) {
          lines_up -= c_win_scroll(1);
          scroll = 1;
        }
        break;
      case 0x39:	// page up
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


    if (strncmp(command_line, "dispheap", 8) == 0)
      dump_heap_index(k_heap);
    else if (strncmp(command_line, "getpid", 6) == 0)
      c_printf("PID: %d\n", getpid());
    else if (strncmp(command_line, "readhdd", 7) == 0) {
      void *tbuf = kmalloc(4096,0);
      //hdd_read(command_line[8], tbuf, command_line[10]);
      c_printf("read(%d, X, %d)\n", atoi(command_line[8]), atoi(command_line[10]));
      kfree(tbuf);
    } else if (!scroll && c_strlen(command_line))
      c_printf("> %s\n", command_line);
  }
}
static void hdd_read(unsigned int lba, void *buf, unsigned int len)
{
  device_t *hdd = ata_open();

  disk_request_t d;
  d.cmd = ATA_READ_MULTIPLE;
  d.lba = lba;
  d.buffer = buf;
  d.num_blocks = len;
  d.blocks_complete = 0;

  hdd->_ctrl(DISK_CMD_READ, (void *) &d);

  int i;
  for (i = 0; i < 512 * len; ++i)
    c_printf("%x", *(((unsigned char *)buf) + i));

  kfree(hdd);
}
