#include "stack.h"
#include "dev/console.h"

#include "process.h"

#include <stdint.h>

void stack_dump(pcb_t *proc)
{
  uint32_t *esp;
  asm volatile ("movl	%%esp, %0" : "=r"(esp));
  c_printf("%d esp: 0x%x\n", proc->pid, esp);

  uint32_t stack_end = ((uint32_t) (proc->stack + 1));
//  c_printf("ESP index: 0x%x\n", ((uint32_t) esp + 2));

  int first = 1;
  while (esp < stack_end) {
    if (first) {
      c_printf("esp ->    0x%x\n", *esp);
      first = 0;
    } else {
      c_printf("          0x%x\n", *esp);
    }

    esp++;
  }
}
