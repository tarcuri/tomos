#include "process.h"
#include "kernel.h"
#include "dev/console.h"

void proc_init()
{
  next_pid = 1;

  // create a PCB for the kernel
  pcb_t *kernel_pcb = (pcb_t *) kmalloc_a(sizeof(pcb_t));

  kernel_pcb->stack = (stack_t *) kmalloc(sizeof(stack_t));

  kernel_pcb->next = 0;
  kernel_pcb->prev = 0;

  kernel_pcb->pid  = next_pid++;
  kernel_pcb->ppid = 0;

  // stick it in the list...or not?
  pcb_list = kernel_pcb;

  // here is the trick pary:
  // context points to the stack, and after we jmp to isr_restore,
  // the context pointer will be stored in esp, so that when we begin
  // to restore the registers, they are popped off the new stack with
  // initialized values below

  // start things at the bottom of the stack structure since x86 stacks grow down
  // dummy return address at bottom of stack followed by a zero dword
  uint32_t *ret = ((uint32_t *)(kernel_pcb->stack + 1)) - 2;
  *ret = (uint32_t) kmain;

  // point the context just above the dummy return address
  kernel_pcb->context = ((context_t *)ret) - 1;

  // now store the segment registers on the stack to be later popped
  // by isr_restore
  kernel_pcb->context->cs = 0x08;
  kernel_pcb->context->ds = 0x10;
  kernel_pcb->context->es = 0x10;
  kernel_pcb->context->fs = 0x10;
  kernel_pcb->context->gs = 0x10;
  kernel_pcb->context->ss = 0x18;

  //c_printf("KERNEL STACK TOP   : 0x%x\n", kernel_pcb->stack);
  //c_printf("KERNEL STACK BOTTOM: 0x%x\n", kernel_pcb->stack + 1);
  //c_printf("KERNEL STACK SIZE  : %x bytes\n", (kernel_pcb->stack + 1) - (kernel_pcb->stack));
  kernel_pcb->context->ebp = (uint32_t) ret;
  kernel_esp = (uint32_t *) kernel_pcb->context;

  c_printf("ESP initialized at 0x%x\n", kernel_esp);

  //uint32_t eflags;
  //asm volatile ("pushfl; popl %0" : "=r"(eflags));
  //c_printf("current eflags: 0x%x\n", eflags);
  c_printf("original EFLAGS: %x\n", get_eflags());
  kernel_pcb->context->eflags = 0x2 | 0x200;

  // kernel entry point
  kernel_pcb->context->eip = (uint32_t) kmain;

  current_proc = kernel_pcb;

  //c_printf("[proc]    kernel process intialized\n");
  //c_printf("stack: 0x%x -> 0x%x\n", kernel_pcb->stack, (uint32_t)kernel_pcb->stack + (4*STACK_SIZE));
}
