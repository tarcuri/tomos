#include "process.h"
#include "kernel.h"
#include "scheduler.h"
#include "dev/console.h"

#include <unistd.h>     // for _exit call

void proc_init()
{
  next_pid = 1;

  // create a PCB for the kernel
  pcb_t *kernel_pcb = (pcb_t *) kmalloc(sizeof(pcb_t));

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
  kernel_pcb->context->cs = GDT_LINEAR;
  kernel_pcb->context->ds = GDT_CODE;
  kernel_pcb->context->es = GDT_CODE;
  kernel_pcb->context->fs = GDT_CODE;
  kernel_pcb->context->gs = GDT_CODE;
  kernel_pcb->context->ss = GDT_DATA;

  //c_printf("KERNEL STACK TOP   : 0x%x\n", kernel_pcb->stack);
  //c_printf("KERNEL STACK BOTTOM: 0x%x\n", kernel_pcb->stack + 1);
  //c_printf("KERNEL STACK SIZE  : %x bytes\n", (kernel_pcb->stack + 1) - (kernel_pcb->stack));
  kernel_pcb->context->ebp = (uint32_t) ret;
  kernel_esp = (uint32_t *) kernel_pcb->context;

  kernel_pcb->context->eflags = 0x2 | 0x200;

  // kernel entry point
  kernel_pcb->context->eip = (uint32_t) kmain;

  current_proc = kernel_pcb;
  
  kernel_pcb->status = READY;
  
  ready_queue = NULL;

  c_printf("[proc]    kernel process intialized\n");
}

int create_process(uint16_t owner_uid, int (*proc)(void))
{
        context_t *context;
        uint32_t *ret;

        pcb_t *pcb = (pcb_t *) kmalloc(sizeof(pcb_t));
        if (pcb == 0) {
                printf("create_process: couldn't allocate a new pcb\n");
                return -1;
        }

        memset(pcb, '\0', sizeof(pcb_t));

        pcb->stack = (stack_t *) kmalloc(sizeof(stack_t));
        if (pcb->stack == 0) {
                printf("create_process: couldn't allocate a stack\n");
                return -1;
        }
        memset(pcb->stack, '\0', sizeof(stack_t));

        pcb->uid = owner_uid;
        pcb->pid = next_pid++;
        pcb->ppid = 1;  // kernel parent process

        // insert into list
        pcb_t *list = pcb_list;
        while (list->next)
                list = list->next;

        list->next = pcb;
        pcb->prev  = list;

        // place a (fake) return address to cause the user process to "return"
        // to the exit system call; however, we probably need our own exit call
        // leaves a longword at the bottom of stack containing 0
        ret = ((uint32_t *)(pcb->stack + 1)) - 2;
        *ret = (uint32_t) _exit;

        pcb->context = ((context_t *) ret) - 1;

        /* 
         * need to review how I configured the GDT so that this is different from
         * the sp2 code, which uses different selectors.
         */
        pcb->context->ss = GDT_DATA;
        pcb->context->gs = GDT_CODE;
        pcb->context->fs = GDT_CODE;
        pcb->context->es = GDT_CODE;
        pcb->context->ds = GDT_CODE;
        pcb->context->cs = GDT_LINEAR;
        
        pcb->context->ebp = (uint32_t) ret;

        // sp2 defined 0x2 | 0x200 as DEFAULT_EFLAGS
        pcb->context->eflags = 0x2 | 0x200;

        pcb->context->eip = (uint32_t) proc;
  
        pcb->status = READY;

        schedule(pcb);

        return pcb->pid;
}
