#include "pg.h"
#include "mm.h"
#include "x86.h"

#include "cio.h"

// need a memory manager to allocate 4KB frames

void pg_init()
{
  // define the page directory on a 4KB boundary after the kernel

  // first define a page directory for the kernel itself
  pg_k_pdir_base   = mm_alloc_frame();
  pg_k_ptable_base = mm_alloc_frame();

  cio_printf("[pg]      kernel page directory initialized at 0x%x\n", pg_k_pdir_base);
  cio_printf("[pg]      kernel page table initialized at 0x%x\n", pg_k_ptable_base);

  // intialize (clear) kernel page directory
  unsigned int i;
  for (i = 0; i < 1024; ++i) {
    pg_k_pdir_base->tables[i] = (pde_t) (0 | PDE_WRITABLE);
  } 

  // identity map the page table to the first 4MB of physical memory
  for (i = 0; i < 1024; ++i) {
    pg_k_ptable_base->pages[i] = (i * 0x1000) | PTE_WRITABLE | PTE_PRESENT;
  }

  // allocate an initial page table for the kernel

  // store the table in the directory
  pg_k_pdir_base->tables[0] = (pde_t) ((unsigned int)pg_k_ptable_base | PDE_WRITABLE | PDE_PRESENT);

  cio_printf("[pg]      inialized %d bytes of memory for kernel paging\n", MM_FRAME_SIZE * 1024);

  // install the page fault handler
  _install_isr(INT_VEC_PAGE_FAULT, pg_page_fault);

  // now tell the CPU to enable paging
  pg_init_proc();

  cio_printf("[pg]      CPU paging initialized\n");
}

// initialize cr3, enable page bit in cr0
void pg_init_proc()
{
  asm volatile ("mov %0, %%cr3"     :: "r"(pg_k_pdir_base));

  unsigned int cr0;
  asm volatile ("mov %%cr0, %0"	    : "=r"(cr0));
  cr0 |= X86_CR0_PAGING;
  asm volatile ("mov %0, %%cr0"     :: "r"(cr0));
}

void pg_page_fault(int error)
{
  unsigned int fault_addr;
  asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

  cio_printf("\n[pg]      PAGE FAULT: 0x%x\n", fault_addr);

  if ((error & PTE_PRESENT)) {
    cio_printf("    present in memory\n");
  } else {
    cio_printf("    page not present in memory\n");
  }

  if (error & PTE_WRITABLE)
    cio_printf("    writable\n");

  if (error & PTE_USER_MODE)
    cio_printf("    user mode\n");

  if (error & PTE_RESERVED)
    cio_printf("    reserved\n");

  panic("PAGE FAULT");
}
