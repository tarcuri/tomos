#include "dev/console.h"
#include "dev/keyboard.h"
#include "tomsh.h"
#include "kernel/mm.h"
#include "kernel/heap.h"
#include "kernel/user.h"
#include "kernel/timer.h"
#include "kernel/kernel.h"

#include "syscalls.h"

//#include "dev/device.h"
#include "dev/ata.h"
#include "fs/ext2.h"

//#include "fs/vfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char tp_output[16][128];

int test_proc_1(void)
{
        struct timer *t = (struct timer *) kmalloc(sizeof(struct timer));

        for (;;) {
                t->delay = 50;
                start_timer(t);
                snprintf(&tp_output[current_proc->pid - 2], 128,
                                "test_proc_1 (%d) = %d\n",
                                current_proc->pid, get_time());
                remove_timer(t);
        }

        return 0;
}

int test_proc_2(void)
{
        for (;;) {
                snprintf(&tp_output[current_proc->pid - 2], 128,
                                "test_proc_2 (%d) = %d\n",
                                current_proc->pid, get_time());
        }

        return 0;
}

int test_proc_3(void)
{
        int i;

        for (i = 0; i < 1000; ++i) {
                snprintf(&tp_output[current_proc->pid - 2], 128,
                                "test_proc_3 (%d) = %d\n",
                                current_proc->pid, get_time());
        }

        return 0;
}


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
      case 0x1B:  // escape
        // nothing yet
        break; 
      case '\b':  // backspace
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
    } else if (strncmp(command_line, "print_superblock", 16) == 0) {
      print_superblock_info();
    } else if (strncmp(command_line, "ls", 2) == 0) {
      int ino = 2;
      if (strlen(command_line) > 3)
        ino = atoi(&command_line[3]);
      c_printf("ls %d\n", ino);
      ls_dir(ino);
    } else if (strncmp(command_line, "gettime", 7) == 0) {
      printf("%d\n", get_time()); 
    } else if (strncmp(command_line, "cat ", 4) == 0) {
      cat_file(atoi(&command_line[4]));
    } else if (strncmp(command_line, "users", 5) == 0) {
      struct user *u;
      if (!user_list) {
        printf("no users in system\n");
      } else {
        for (u = user_list; u; u = u->next) {
          printf("%d - %s [%s]\n", u->uid, u->name, u->realname);
        }
      }
    } else if (strncmp(command_line, "adduser ", 8) == 0) {
      strtok(command_line, " ");
      char *name = strtok(NULL, " ");
      char *realname = strtok(NULL, "\"");
      if (name && realname) {
        if (add_user(name, realname) == USER_EXISTS) {
          printf("user %s already exists\n", name);
        } else {
          printf("added user: %s [%s]\n", name, realname);
        }
      }
    } else if (strncmp(command_line, "run", 3) == 0) {
      strtok(command_line, " ");
      char *proc_owner = strtok(NULL,  " ");
      char *proc_name = strtok(NULL, " ");
      char *proc_prio = strtok(NULL, " ");
      uint16_t prio = LOW;
      if (proc_prio) {
        int p = atoi(proc_prio);
        if (p <= HIGH && p >= LOW) {
          prio = p;
        } 
      }

      if (prio == LOW) {
        c_printf("[LOW]    ");
      } else if (prio == MEDIUM) {
        c_printf("[MEDIUM] ");
      } else if (prio == HIGH) {
        c_printf("[HIGH]   ");
      } else {
        c_printf("INVALID");
      }

      if (proc_owner && proc_name) {
        if (strncmp(proc_name, "test_proc_1", 11) == 0) {
          uint16_t uid;
          if (!get_uid(proc_owner, &uid)) {
            int pid = create_process(uid, "test_proc_1", test_proc_1, prio);
            printf("spawned %s (%d)\n", proc_name, pid);
          } else {
            printf("couldn't lookup uid for %s\n", proc_owner);
          }
        } else if (strncmp(proc_name, "test_proc_2", 11) == 0) {
          uint16_t uid;
          if (!get_uid(proc_owner, &uid)) {
            int pid = create_process(uid, "test_proc_2", test_proc_2, prio);
            printf("spawned %s (%d)\n", proc_name, pid);
          } else {
            printf("couldn't lookup uid for %s\n", proc_owner);
          }
        } else if (strncmp(proc_name, "test_proc_3", 11) == 0) {
          uint16_t uid;
          if (!get_uid(proc_owner, &uid)) {
            int pid = create_process(uid, "test_proc_3", test_proc_3, prio);
            printf("spawned %s (%d)\n", proc_name, pid);
          } else {
            printf("couldn't lookup uid for %s\n", proc_owner);
          }
        }

      }
    } else if (strncmp(command_line, "kill ", 5) == 0) {
      strtok(command_line, " ");
      char *pid_str = strtok(NULL, " ");
      if (!pid_str)
        continue;
      uint16_t pid;
      if (sscanf(pid_str, "%d", &pid)) {
        kill_process(pid);
      }
    } else if (strncmp(command_line, "ps", 2) == 0) {
      strtok(command_line, " ");
      char *option = strtok(NULL, " ");
      int all = 0;
      if (option && strncmp(option, "a", 1) == 0)
              all = 1;
      pcb_t *p;
      printf("%3s\t%3s\t%5s\t%5s\t%5s\t%s\n",
                      "PID", "UID", "USER", "TIME", "PRIO", "COMMAND");
      for (p = get_pcb_list(); p; p = p->next) {
              if (p->pid < 2 && !all)
                      continue;
              struct user *u = get_user(p->uid);
              printf("%2d\t %2d\t %5s\t%5d\t%4d\t %s\n",
                              p->pid, p->uid, u->name,
                              p->time_slices, p->prio, p->cmd);
      }
    } else if (strncmp(command_line, "output", 6) == 0) {
      strtok(command_line, " ");
      char *pid_str = strtok(NULL, " ");
      uint16_t pid;
      if (pid_str && sscanf(pid_str, "%d", &pid) && pid > 2) {
        printf("%s\n", tp_output[pid - 2]);
      } else {
        printf("invalid/missing pid\n");
      }
    } else if (strncmp(command_line, "log", 3) == 0) {
      asm volatile("cli");
      print_log();
      asm volatile("sti");
    } else if (strncmp(command_line, "l3", 2) == 0) {
      strtok(command_line,  " ");
      char *user = strtok(NULL, " ");
      uint16_t uid = 0;
      if (user && !get_uid(user, &uid)) {
        create_process(uid, "test_proc_2", test_proc_2, 0);
        create_process(uid, "test_proc_2", test_proc_2, 1);
        create_process(uid, "test_proc_2", test_proc_2, 2);
      }
    } else if (strncmp(command_line, "help", 4) == 0) {
      c_printf("tomsh commands:\n");
      c_printf("\tdispheap\n");
      c_printf("\tgetpid\n");
      c_printf("\tusers\n");
      c_printf("\tgettime\n");
      c_printf("\tidhdd\n");
      c_printf("\ttestata\n");
      c_printf("\tprint_superblock\n");
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
