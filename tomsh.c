#include "dev/console.h"
#include "dev/keyboard.h"
#include "tomsh.h"
#include "kernel/mm.h"
#include "kernel/heap.h"

#include "syscalls.h"

//#include "dev/device.h"
//#include "dev/ata.h"

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
  strncpy(prompt, "tomsh $", 8);
  int scroll = 0;
  int lines_up = 0;

  while (1) {
    if (!scroll) {
      c_printf("%s ", prompt);
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


    if (strncmp(command_line, "dispheap", 8) == 0)
      dump_heap_index(k_heap);
    else if (strncmp(command_line, "getpid", 6) == 0)
      c_printf("PID: %d\n", getpid());
/*    else if (strncmp(command_line, "ls ", 3) == 0)
      ls_dir(atoi(&command_line[3]));*/
    else if (!scroll && strlen(command_line))
      c_printf("> %s\n", command_line);
  }
}

/*
void ls_dir(int ino)
{
  dir_t *dir = opendir(ino);	// open the root directory

  // list its contents
  char name[256];
  dirent_t *ent = readdir(dir);
  while (ent != NULL) {
    memcpy((void *) name, (void *) ent->name, ent->name_len);
    name[ent->name_len] = '\0';
    printf("  %04d: %s\n", ent->inode, name);
    ent = readdir(dir);
  }

  closedir(dir);
}


void hdd_read(unsigned int lba, void *buf, unsigned int len)
{
  device_t *hdd = ata_open();

  disk_request_t d;
  d.cmd = DISK_CMD_READ;
  d.lba = lba;
  d.buffer = buf;
  d.num_blocks = len;
  d.blocks_complete = 0;

  // call the ata driver
  hdd->_ctrl(DISK_CMD_READ, (void *) &d);

  int i;
  for (i = 0; i < 512 * len; ++i)
    printf("%x", *(((unsigned char *)buf) + i));

  printf("\n");

  kfree(hdd);
}
*/
