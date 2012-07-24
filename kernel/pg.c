#include "pg.h"
#include "mm.h"
#include "x86.h"

#include "dev/console.h"

// need a memory manager to allocate 4KB frames

void pg_init()
{
  // Before paging is enabled, we are going to statically allocate a few
  // page frames for the paging directory. These will never need to be deallocated.

  // first define a page directory for the kernel itself
  kernel_pg_directory   = mm_alloc_frame();	// TODO: page directory is larger than 4KB
  memset(kernel_pg_directory, 0, sizeof(page_directory_t));
  kernel_pg_directory->physical_addr = kernel_pg_directory->tables_phys;	// at this point, we're all physical

  c_printf("[pg]      kernel page directory initialized at 0x%x\n", kernel_pg_directory);
  //c_printf("[pg]      kernel page table initialized at 0x%x\n", kernel_pg_table);

  // Map some pages in the kernel heap. This will create page_table_t's
  unsigned int i;
  for (i = HEAP_BASE_ADDRESS; i < HEAP_BASE_ADDRESS + HEAP_INITIAL_SIZE; i += 0x1000)
    pg_get_page(i, 1, kernel_pg_directory);
  

  // We need to identity map (phys addir = virt addr) from 0x0 to end of used memory (JamesM)
  i = 0;
  while (i < mm_highest_allocd + 0x1000) {
  }


  // install the page fault handler
  _install_isr(INT_VEC_PAGE_FAULT, pg_page_fault);

  // now tell the CPU to enable paging
  pg_switch_directory();

  c_printf("[pg]      CPU paging initialized\n");
}

// initialize cr3, enable page bit in cr0
void pg_switch_directory(page_directory_t *dir)
{
  asm volatile ("mov %0, %%cr3"     :: "r"(kernel_pg_directory));

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
		return &dir->tables[table_idx]->pages[address % 1024];
	} else if (make) {
		uint32_t t;
		// won't work - we'd need the pre-heap placement allocation like JamesM's
		// can we just alloc a page using the (physical) memory manager (mm)?
		if (k_heap) {
			dir->tables[table_idx] = (page_table_t*) kmalloc(sizeof(page_table_t), 1, &t);
		} else {
			dir->tables[table_idx] = (page_table_t*) mm_alloc_frame();	// 4KB aligned frame
			t = dir->tables[table_idx];
		}
        memset(dir->tables[table_idx], 0, 0x1000);
		dir->tables_phys[table_idx] = t | PTE_PRESENT | PTE_WRITABLE;
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
  }

  if (error & PTE_WRITABLE)
    c_printf("    writable\n");

  if (error & PTE_USER_MODE)
    c_printf("    user mode\n");

  if (error & PTE_RESERVED)
    c_printf("    reserved\n");

  panic("PAGE FAULT");
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
  if (page->frame)
    return;

  // grab the next free page index
  unsigned int idx = mm_get_free_frame();

  mm_set_frame(idx);
  page->present = 1;
  page->rw = (is_writeable==1) ? 1 : 0;
  page->user = (is_kernel==1) ? 1 : 0;
  page->frame = MM_FRAME_ADDRESS(idx);
}

void pg_free_frame(page_t *page)
{
  if (!page->frame_addr) {
    return;
  } else {
    mm_clear_frame(page->frame_addr);
  }
}
