#ifndef PROCESS_H
#define PROCESS_H

#include "stack.h"

#include <stdint.h>

enum process_status {READY, SLEEP, KB_WAIT, IO_WAIT, TERMINATE};

// adapted from RIT CS project
// order must be consistent with isr_stubs.S
typedef struct context {
  uint32_t ss;
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t vector;
  uint32_t code;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
} context_t;

// modeled after RIT/Linux PCBs
typedef struct process_control_block
{
  context_t	*context;	// must be first so that we can easily r/w from asm

  // need a stack structure
  stack_t	*stack;

  // list of open files

  // TODO: include a notion of current disk requests
  // TODO: include a notion of the current process (which receives IO, etc)

  // for now, the PCB itself will be a list node
  struct process_control_block *next;
  struct process_control_block *prev;

  char cmd[64];

  // process id
  uint16_t	pid;
  // parent process id
  uint16_t	ppid;
  // process owner user id
  uint16_t  uid;
  // process state
  uint16_t  status;

  uint32_t time_slices;
} __attribute__((__packed__)) pcb_t;


// global data
pcb_t *pcb_list;
uint16_t next_pid;

uint32_t *kernel_esp;

pcb_t *current_proc;

void proc_init(void);

int create_process(uint16_t owner_uid, char *cmd, int (*proc)(void));
void kill_process(uint16_t pid);
pcb_t *get_pcb_list(void);

#endif
