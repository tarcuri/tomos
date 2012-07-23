#include "pg.h"
#include "mm.h"
#include "x86.h"

#include "dev/console.h"

// need a memory manager to allocate 4KB frames

void pg_init()
{
  // define the page directory on a 4KB boundary after the kernel

  // first define a page directory for the kernel itself
  kernel_pg_directory   = mm_alloc_frame();
  kernel_pg_table = mm_alloc_frame();

  //c_printf("[pg]      kernel page directory initialized at 0x%x\n", kernel_pg_directory);
  //c_printf("[pg]      kernel page table initialized at 0x%x\n", kernel_pg_table);

  // intialize (clear) kernel page directory
  unsigned int i;
  for (i = 0; i < 1024; ++i) {
    kernel_pg_directory->tables[i] = (pde_t) (0 | PDE_WRITABLE);
  } 

  // identity map the page table to the first 4MB of physical memory
  for (i = 0; i < 1024; ++i) {
    kernel_pg_table->pages[i] = (i * 0x1000) | PTE_WRITABLE | PTE_PRESENT;
  }

  // allocate an initial page table for the kernel

  // store the table in the directory
  kernel_pg_directory->tables[0] = (pde_t) ((unsigned int)kernel_pg_table | PDE_WRITABLE | PDE_PRESENT);

  //c_printf("[pg]      inialized %d bytes of memory for kernel paging\n", MM_FRAME_SIZE * 1024);

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
pte_t *pg_get_page(uint32_t address, int make, page_directory_t *dir)
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
