#include "paging.h"
#include "mm.h"
#include "heap.h"
#include "x86.h"

#include <stdlib.h>

void pg_init()
{
  _install_isr(INT_VEC_PAGE_FAULT, page_fault);
  _install_isr(INT_VEC_GENERAL_PROTECTION, protection_fault);

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
  c_printf("[pg]      paging initialized\n");
}

void switch_directory(page_directory_t *pd)
{
  current_pd = pd;
  asm volatile ("movl %0, %%cr3"    : : "r"(pd));

  uint32_t cr0;
  asm volatile ("movl %%cr0, %0" : "=r"(cr0));
  cr0 |= X86_CR0_PAGING;
  asm volatile ("movl %0, %%cr0" : : "r"(cr0));
}

// this returns the page referenced by va,
// not the actuall address offset within that page
page_t *get_page(uint32_t va)
{
  page_table_t *pt;
  page_directory_t *pd;
  uint32_t pde, pte;

  pde = (uint32_t) current_pd[PG_DIR_OFFSET(va)];
  if (!pde) {
    pd = (page_directory_t *) kmalloc_a(0x1000, 1);
    current_pd[PG_DIR_OFFSET(va)] = (page_directory_t) ((uint32_t)pd | PG_PRESENT | PG_WRITE);
    pde = (uint32_t) current_pd[PG_DIR_OFFSET(va)];
  }
  pt = (page_table_t *) (pde & 0xFFFFF000);
  pt[PG_TABLE_OFFSET(va)] = (uint32_t)pt[PG_TABLE_OFFSET(va)] | PG_PRESENT | PG_WRITE;

  return (page_t *) &pt[PG_TABLE_OFFSET(va)];
}

uint32_t get_phys_addr(uint32_t va)
{
  page_t *pg = get_page(va);
  return ((uint32_t) *pg | PG_PAGE_OFFSET(va));
}

void alloc_frame(page_t *pg, int kernel, int write)
{
  uint32_t frame = ((uint32_t)*pg) & 0xFFFFF000;
  if (frame) {
    // already allocated
    return;
  } else {
    frame = mm_get_free_frame();
    mm_set_frame(frame);
    frame = MM_FRAME_ADDRESS(frame) | (kernel ? 0 : PG_USER)
                                    | (write ? PG_WRITE : 0)
                                    | PG_PRESENT;
    *pg = (page_t) frame;
    return;
  }
}

void page_fault(uint32_t error)
{
  unsigned int fault_addr;
  asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

  uint32_t present = (error & PG_PRESENT) ? 1 : 0;
  uint32_t supervisor = (error & PG_USER) ? 0 : 1;
  uint32_t write = (error & PG_WRITE) ? 1 : 0;

  c_printf(" PAGE FAULT AT 0x%x - ", fault_addr);
  if (!present) {
    if (write)
      c_printf("WRITE ");
    c_printf("NOT PRESENT\n");
    alloc_frame(get_page(fault_addr), 1, 1);
    return;
  }

  panic("PAGE FAULT");
}

void protection_fault(uint32_t error)
{
  // faulting address should be the return address from the stack
  uint32_t fault_addr, cs;

  asm volatile ("movl   4(%%esp), %0" : "=r"(fault_addr));
  asm volatile ("movw   8(%%esp), %0" : "=r"(cs));

  c_printf(" GENERAL PROTECTION FAULT: %x\n", error);
  c_printf(" FAULTING ADDRESS: %x:%x\n", cs, fault_addr);

  panic("GPF");
}
