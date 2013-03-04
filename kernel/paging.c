#include "paging.h"
#include "mm.h"
#include "x86.h"

#include <stdlib.h>

page_directory_t *k_page_directory = 0;

void pg_init()
{
  _install_isr(INT_VEC_PAGE_FAULT, pg_page_fault);

  // first create a kernel page directory
  k_page_directory = (page_directory_t *) mm_place_kalloc(0x1000, 1);
  memset(k_page_directory, 0, 0x1000);
  uint32_t physical_addr = (uint32_t) k_page_directory;

  // map in the heap?

  // create the base page table
  k_page_directory[0] = mm_place_kalloc(0x1000, 1) | PG_PRESENT | PG_WRITE;
  uint32_t *pt = (uint32_t *) (k_page_directory[0] & 0xFFFFF000);

  // identity map the first 4 MB
  uint32_t i;
  for (i = 0; i < 1024; i++) {
    pt[i] = i*0x1000 | PG_PRESENT | PG_WRITE;
  }

  // assign the second-last table and zero it (JamesM)
  k_page_directory[1022] = mm_place_kalloc(0x1000, 1) | PG_PRESENT | PG_WRITE;
  pt = (uint32_t *) (k_page_directory[1022] & 0xFFFFF000);
  memset(pt, 0, 0x1000);

  // The last entry of the second-last table is the directory itself.
  pt[1023] = (uint32_t) k_page_directory | PG_PRESENT | PG_WRITE;

  // The last table loops back on the directory itself.
  k_page_directory[1023] = (uint32_t) k_page_directory | PG_PRESENT | PG_WRITE;

  asm volatile ("movl %0, %%cr3"    : : "r"(k_page_directory));

  uint32_t cr0;
  asm volatile ("movl %%cr0, %0" : "=r"(cr0));
  cr0 |= X86_CR0_PAGING;
  asm volatile ("movl %0, %%cr0" : : "r"(cr0));

  // now get the virtual address of the page directory
  asm volatile ("movl %%cr3, %0" : "=r"(k_page_directory));
}

int get_phys_addr(uint32_t va, uint32_t *pa)
{
  uint32_t virtual_pg = va / 0x1000;
  uint32_t pt_idx = PG_DIR_IDX(virtual_pg);

  if (k_page_directory[pt_idx] == 0)
    return 0;   // not allocated yet

  uint32_t *pt = k_page_directory[pt_idx];
  if (pt[virtual_pg] != 0) {
    if (pa) *pa = pt[virtual_pg] & 0xFFFFF000;
    return 1;
  }
}

void pg_page_fault(uint32_t error)
{

}
