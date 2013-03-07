#ifndef TOMOS_PAGING_H
#define TOMOS_PAGING_H

#include <stdint.h>

#define PG_PRESENT 0x01
#define PG_WRITE   0x02
#define PG_USER    0x04

// bits 31:22 are the page directory index
#define PG_DIR_OFFSET(x)       ((uint32_t) ((x >> 22) & 0x3FF))
// bits 21:12 are the page table index
#define PG_TABLE_OFFSET(x)     ((uint32_t) ((x >> 12) & 0x3FF))
// bits 11:0 are the page offset
#define PG_PAGE_OFFSET(x)      ((uint32_t) (x & 0xFFF))

typedef uint32_t page_t;

typedef uint32_t page_table_t;

typedef uint32_t page_directory_t;

page_directory_t *kpd;
page_directory_t *current_pd;

// functions
void pg_init(void); 
void pg_page_fault(uint32_t error);

/*
 * From Intel, Vol 3.
 * 
 * A PDE is selected using the physical address defined as
 * follows:
 * — Bits 39:32 are all 0.
 * — Bits 31:12 are from CR3.
 * — Bits 11:2 are bits 31:22 of the linear address.
 * — Bits 1:0 are 0.
 */

void switch_directory(page_directory_t *pd);

page_t *get_page(uint32_t va);
uint32_t get_phys_addr(uint32_t va);
void alloc_frame(page_t *pg, int kernel, int write);

#endif
