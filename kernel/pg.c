#include "pg.h"
#include "mm.h"
#include "x86.h"

#include "dev/console.h"

// need a memory manager to allocate 4KB frames

void pg_init()
{
  // Before paging is enabled, we are going to statically allocate a few
  // page frames for the paging directory. These will never need to be deallocated.
  kernel_pg_directory   = (page_directory_t *)mm_place_kalloc(sizeof(page_directory_t), 1); 
  memset(kernel_pg_directory, 0, sizeof(page_directory_t));
  kernel_pg_directory->physical_addr = (uint32_t) &kernel_pg_directory->tables_phys;

  c_printf("[pg]      kernel page directory initialized at 0x%x\n", kernel_pg_directory);

  // need to map in the kernel heap
  uint32_t i = 0;
  k_heap_loc = mm_highest_allocd;
  k_heap_loc &= 0xFFFFF000;
  k_heap_loc += 0x1000;
  for (i = k_heap_loc; i < k_heap_loc + HEAP_INITIAL_SIZE; i += 0x1000)
    pg_get_page(i, 1, kernel_pg_directory);

  // We need to identity map (phys addir = virt addr) from 0x0 to end of used memory (JamesM)
  // After this we can't increase the placement address, need to enable heap.
  uint32_t pre_allocd_mem = mm_highest_allocd;
  c_printf("about to map %d frames\n", pre_allocd_mem / 0x1000);
  for (i = 0; i < 1024; i++) {
    kernel_pg_directory->tables[0]->pages[i].frame = i*0x1000;
    kernel_pg_directory->tables[0]->pages[i].present = 1;
    kernel_pg_directory->tables[0]->pages[i].rw = 1;
  }
  /*
  i = 0;
  while (i < pre_allocd_mem+0x1000) {
	pg_alloc_frame(pg_get_page(i, 1, kernel_pg_directory), 0, 0);
    i += 0x1000;
  }
  */

  // allocate the heap pages mapped in earlier
  for (i = k_heap_loc; i < k_heap_loc + HEAP_INITIAL_SIZE; i += 0x1000) {
	pg_alloc_frame(pg_get_page(i, 1, kernel_pg_directory), 0, 0);
    i += 0x1000;
  }

  // install the page fault handler
  _install_isr(INT_VEC_PAGE_FAULT, pg_page_fault);

  // now tell the CPU to enable paging
  pg_switch_directory(kernel_pg_directory);

  c_printf("[pg]      CPU paging initialized\n");

  // now initialize the heap
  heap_init();
}

// initialize cr3, enable page bit in cr0
void pg_switch_directory(page_directory_t *dir)
{
  asm volatile ("mov %0, %%cr3"     :: "r"(&dir->tables_phys));

  unsigned int cr0;
  asm volatile ("mov %%cr0, %0"	    : "=r"(cr0));
  cr0 |= X86_CR0_PAGING;
  asm volatile ("mov %0, %%cr0"     :: "r"(cr0));
}

// modeled after JamesM's paging
page_t *pg_get_page(uint32_t address, int make, page_directory_t *dir)
{
	address /= 0x1000;	// turn address into an index?

    uint32_t table_idx = address / 1024;	// page table containing this address

    if (dir->tables[table_idx]) {	// if this table is already assigned
        if (first_page == 2)
          c_printf("@ page table already assigned\n");
		return &dir->tables[table_idx]->pages[address % 1024];
	} else if (make) {
		uint32_t t;
		// won't work - we'd need the pre-heap placement allocation like JamesM's
		// can we just alloc a page using the (physical) memory manager (mm)?
        c_printf("table_idx: %d\n", table_idx);
		if (k_heap) {
            c_printf("entering kmalloc_p\n");
			dir->tables[table_idx] = (page_table_t*) kmalloc_p(sizeof(page_table_t), 1, &t);
            c_printf("new page table[%d] = 0x%x\n", table_idx, dir->tables[table_idx]);
		} else {
            c_printf("entering place_kalloc\n");
			dir->tables[table_idx] = (page_table_t*) mm_place_kalloc(sizeof(page_table_t), 1);
			t = dir->tables[table_idx];
		}
        memset(dir->tables[table_idx], 0, 0x1000);
        // i though we want to clear the supervisor bit for kernel mode?
		dir->tables_phys[table_idx] = t | PDE_PRESENT | PDE_WRITABLE | PDE_SUPERVISOR;
		return &dir->tables[table_idx]->pages[address % 1024];
    }
	return 0;
}

void pg_page_fault(int error)
{
  unsigned int fault_addr;
  asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

  c_printf("\n[pg]      PAGE FAULT: 0x%x\n", fault_addr);

  if ((error & PTE_PRESENT)) {
    c_printf("    present in memory\n");
  } else {
    c_printf("    page not present in memory\n");
    // map in this page
    uint32_t pde = fault_addr >> 22;
    uint32_t pte = fault_addr >> 12 & 0x03FF;
    uint32_t pde_o = fault_addr / 0x1000;
    uint32_t pte_o = pde_o / 1024;
    c_printf("phys: 0x%x 0x%x\n", kernel_pg_directory->tables_phys[pte_o], &kernel_pg_directory->tables_phys[pte_o]);
    c_printf("phys: 0x%x 0x%x\n", pde, &pde);
    if (pde_o & PDE_PRESENT)
      c_printf("PDE other PRESENT\n");
    if (pte_o & PTE_PRESENT)
      c_printf("PTE other present\n");
    c_printf("%x : %x\n", pde, pde_o);
    if (pde & PDE_PRESENT) {
      // PTE_PRESENT will be cleared
      c_printf("PDE_PRESENT - 0x%x\n", pte);
      if (pte & PTE_PRESENT == 0)
        c_printf("PTE NOT PRESENT\n");
      pg_alloc_frame(pg_get_page(fault_addr, 0, kernel_pg_directory), 1, 1);
    } else {
      c_printf("PDE NOT PRESENT - setting up page table\n");
      pg_get_page(fault_addr, 1, kernel_pg_directory);
      asm volatile ("iret");
    }
  }

  if (error & PTE_WRITABLE)
    c_printf("    writable\n");

  if (error & PTE_USER_MODE)
    c_printf("    user mode\n");

  if (error & PTE_RESERVED)
    c_printf("    reserved\n");

  //panic("PAGE FAULT");
  return;
}

page_directory_t* pg_clone_directory()
{

}

page_table_t* pg_clone_table()
{

}

// page frame management
// allocate a single page frame
void pg_alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
  if (first_page == 2)
    c_printf("@ pg_alloc_frame 0x%x\n", page->frame);
  if (page->frame)
    return;

  // grab the next free page index
  unsigned int idx = mm_get_free_frame();

  mm_set_frame(idx);
  page->present = 1;
  page->rw = (is_writeable==1) ? 1 : 0;
  page->user = (is_kernel==1) ? 0 : 1;
  page->frame = idx;
  if (first_page == 2)
    c_printf("@ pg_alloc_frame DONE\n", page->frame);
}

void pg_free_frame(page_t *page)
{
  if (!page->frame) {
    return;
  } else {
    mm_clear_frame(page->frame);
  }
}


