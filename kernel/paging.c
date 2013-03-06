#include "paging.h"
#include "mm.h"
#include "heap.h"
#include "x86.h"

#include <stdlib.h>

void pg_init()
{
  _install_isr(INT_VEC_PAGE_FAULT, pg_page_fault);

  // first create a kernel page directory
  kpd = (page_directory_t *) mm_place_kalloc(0x1000, 1);
  memset(kpd, 0, 0x1000);
  uint32_t physical_addr = (uint32_t) kpd;

  // create the base page table
  kpd[0] = mm_place_kalloc(0x1000, 1) | PG_PRESENT | PG_WRITE;
  uint32_t *pt = (uint32_t *) (kpd[0] & 0xFFFFF000);

  // identity map the first 4 MB
  uint32_t i;
  for (i = 0; i < 1024; i++) {
    pt[i] = i*0x1000 | PG_PRESENT | PG_WRITE;
  }

  c_printf("id mapped up to: 0x%x\n", pt[1023]);

  // assign the second-last table and zero it (JamesM)
  kpd[1022] = mm_place_kalloc(0x1000, 1) | PG_PRESENT | PG_WRITE;
  pt = (uint32_t *) (kpd[1022] & 0xFFFFF000);
  memset(pt, 0, 0x1000);

  // The last entry of the second-last table is the directory itself.
  pt[1023] = (uint32_t) kpd | PG_PRESENT | PG_WRITE;

  // The last table loops back on the directory itself.
  kpd[1023] = (uint32_t) kpd | PG_PRESENT | PG_WRITE;

  // now enable the heap
  k_heap_loc = (mm_highest_allocd + 0x1000) & 0xFFFFF000;
  heap_init();

  switch_directory(kpd);
  c_printf("heap loc: 0x%x\n", k_heap_loc);
  c_printf("phys: 0x%x\n", get_page(k_heap_loc, kpd));
  // now get the virtual address of the page directory
}

page_directory_t *switch_directory(page_directory_t *pd)
{
  asm volatile ("movl %0, %%cr3"    : : "r"(pd));

  uint32_t cr0;
  asm volatile ("movl %%cr0, %0" : "=r"(cr0));
  cr0 |= X86_CR0_PAGING;
  asm volatile ("movl %0, %%cr0" : : "r"(cr0));

  page_directory_t *old_pd = 0;
  asm volatile ("movl %%cr3, %0" : "=r"(old_pd));
  return old_pd;
}

page_t get_page(uint32_t va, page_directory_t *dir)
{
  page_table_t *pt;
  uint32_t virtual_pg = va / 0x1000;
  uint32_t pt_idx = PG_DIR_IDX(virtual_pg);

  if (dir[pt_idx] == 0) {
    uint32_t phys;
    dir[pt_idx] = kmalloc_p(0x1000, 1, &phys);
    dir[pt_idx] = phys | PG_PRESENT | PG_WRITE;
    pt = dir[pd_idx] & 0xFFFFF000;
  } else {
    pt = dir[pt_idx];
  }

  return (page_t) &pt[virtual_pg % 1024];
}

int get_phys_addr(uint32_t va, uint32_t *pa)
{
  uint32_t virtual_pg = va / 0x1000;
  uint32_t pt_idx = PG_DIR_IDX(virtual_pg);

  if (kpd[pt_idx] == 0)
    return 0;   // not allocated yet

  uint32_t *pt = kpd[pt_idx];
  if (pt[virtual_pg] != 0) {
    if (pa) *pa = pt[virtual_pg] & 0xFFFFF000;
    return 1;
  }
}

void pg_page_fault(uint32_t error)
{
  c_printf("page fault: %d\n", error);

  panic("PAGE FAULT");
}
