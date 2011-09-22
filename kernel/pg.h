#ifndef PAGING_H
#define PAGING_H

// now what?

// page table entry
#define PTE_PRESENT	0x00000001	// bit 0 identifys if a page is present in memory
#define PTE_WRITABLE	0x00000002	// bit 1 marks the page as r/w
#define PTE_USER_MODE	0x00000004	// bit 2 set if user mode (clear for kernel)
#define PTE_RESERVED	0x00000198	// bits 3,4,7,8 are reserved by the CPU
#define PTE_ACCESSED	0x00000020	// bit 5 set if page has been accessed
#define PTE_DIRTY	0x00000030	// bit 6 set if page has been written
#define PTE_AVAILABLE	0x00000E00	// bits 9,10,11 are available to the kernel
#define PTE_FRAME_ADDR	0xFFFFF000	// high 20 bits are the frame address

// page directory entry

#define PDE_PRESENT	0x00000001	// bit 0 identifys if a page is present in memory
#define PDE_WRITABLE	0x00000002	// bit 1 marks the page as r/w
#define PDE_SUPERVISOR	0x00000004	// bit 2 set if user mode (clear for kernel)
#define PDE_PWT		0x00000008	// bit 3 set to enable page-level write through (see Intel)
#define PDE_PCD		0x00000010	// bit 4 set to disable page-level cache
#define PDE_ACCESSED	0x00000020	// bit 5 set if page has been accessed
#define PDE_AVAILABLE	0x00000F40	// bits 11,10,9,8,6 available for kernel
#define PDE_TABLE_ADDR	0xFFFFF000	// high 20 bits are frame address

#define KERNEL_END	

typedef unsigned int pte_t;
typedef unsigned int pde_t;

typedef struct page_directory
{
  pde_t	tables[1024];
} page_directory_t;

typedef struct page_table
{
  pte_t pages[1024];
} page_table_t;

page_directory_t *pg_k_pdir_base;

page_table_t *pg_k_ptable_base;


// functions
void pg_init(void);
void pg_init_proc(void);

void pg_page_fault(int error);


#endif