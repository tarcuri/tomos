#include "process.h"
#include "dev/console.h"

void proc_init()
{
  next_pid = 1;

  extern kernel_stack_base;

  // create a PCB for the kernel
  pcb_t *kernel_pcb = (pcb_t *) kmalloc(sizeof(pcb_t));

  kernel_pcb->context = (context_t *) kmalloc(sizeof(context_t));

  kernel_pcb->context->ds = 0x10;
  kernel_pcb->context->es = 0x10;
  kernel_pcb->context->fs = 0x10;
  kernel_pcb->context->gs = 0x10;
  kernel_pcb->context->ss = 0x18;

  kernel_pcb->stack = (stack_t *) kernel_stack_base;
  kernel_pcb->next = 0;
  kernel_pcb->prev = 0;

  kernel_pcb->pid  = next_pid++;
  kernel_pcb->ppid = 0;

  pcb_list = kernel_pcb;

  current_proc = kernel_pcb;

  c_printf("[proc]    kernel process intialized\n");
}
