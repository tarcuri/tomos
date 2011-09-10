#include "pg.h"
#include "mm.h"

#include "cio.h"

// need a memory manager to allocate 4KB frames

void pg_init()
{
  // define the page directory on a 4KB boundary after the kernel

  // first define a page directory for the kernel itself
  pg_directory = (page_directory_t *) mm_alloc_frame();

  cio_printf("[pg]      kernel page directory initialized at 0x%x\n", pg_directory);
}
