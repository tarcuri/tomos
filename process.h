#ifndef PROCESS_H
#define PROCESS_H

// adapted from RIT CS project
// order must be consistent with isr_stubs.S
typedef struct context {
  unsigned int ss;
  unsigned int gs;
  unsigned int fs;
  unsigned int es;
  unsigned int ds;
  unsigned int edi;
  unsigned int esi;
  unsigned int ebp;
  unsigned int esp;
  unsigned int ebx;
  unsigned int edx;
  unsigned int ecx;
  unsigned int eax;
  unsigned int vector;
  unsigned int code;
  unsigned int eip;
  unsigned int cs;
  unsigned int eflags;
} context_t;

// modeled after RIT/Linux PCBs
typedef struct process_control_block
{
  context_t	*context;	// must be first?

  // TODO: include a notion of current disk requests
  // TODO: include a notion of the current process (which receives IO, etc)

  // for now, the PCB itself will be a list node
  struct proccess_control_block *next;
  struct proccess_control_block *prev;

  unsigned short	pid;
  unsigned short	ppid;

  unsigned char		status;		// can use a bit here to identify privledge level
} pcb_t;


// global data
pcb_t *pcb_list;
unsigned short next_pid;

void proc_init()
{
  next_pid = 0;

  // create a PCB for the kernel
  pcb_t kernel_pcb;
  kernel_pcb.next = 0;
  kernel_pcb.prev = 0;

  pcb_list = kernel_pcb;
}

#endif
