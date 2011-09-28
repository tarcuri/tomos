#include "stack.h"
#include "dev/console.h"

#include <stdint.h>

void stack_dump(stack_t *stack)
{
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 0, *stack[(STACK_SIZE-1) - 0], 12, *stack[(STACK_SIZE-1) - 12], 24, *stack[(STACK_SIZE-1) - 24]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 1, *stack[(STACK_SIZE-1) - 1], 13, *stack[(STACK_SIZE-1) - 13], 25, *stack[(STACK_SIZE-1) - 25]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 2, *stack[(STACK_SIZE-1) - 2], 14, *stack[(STACK_SIZE-1) - 14], 26, *stack[(STACK_SIZE-1) - 26]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 3, *stack[(STACK_SIZE-1) - 3], 15, *stack[(STACK_SIZE-1) - 15], 27, *stack[(STACK_SIZE-1) - 27]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 4, *stack[(STACK_SIZE-1) - 4], 16, *stack[(STACK_SIZE-1) - 16], 28, *stack[(STACK_SIZE-1) - 28]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 5, *stack[(STACK_SIZE-1) - 5], 17, *stack[(STACK_SIZE-1) - 17], 29, *stack[(STACK_SIZE-1) - 29]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 6, *stack[(STACK_SIZE-1) - 6], 18, *stack[(STACK_SIZE-1) - 18], 30, *stack[(STACK_SIZE-1) - 30]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 7, *stack[(STACK_SIZE-1) - 7], 19, *stack[(STACK_SIZE-1) - 19], 31, *stack[(STACK_SIZE-1) - 31]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 8, *stack[(STACK_SIZE-1) - 8], 20, *stack[(STACK_SIZE-1) - 20], 32, *stack[(STACK_SIZE-1) - 32]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 9, *stack[(STACK_SIZE-1) - 9], 21, *stack[(STACK_SIZE-1) - 21], 33, *stack[(STACK_SIZE-1) - 33]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 10, *stack[(STACK_SIZE-1) - 10], 22, *stack[(STACK_SIZE-1) - 22], 34, *stack[(STACK_SIZE-1) - 34]);
  c_printf("%d: 0x%x, %d 0x%x, %d 0x%x\n", 11, *stack[(STACK_SIZE-1) - 11], 23, *stack[(STACK_SIZE-1) - 23], 35, *stack[(STACK_SIZE-1) - 35]);
}
