#ifndef TOMOS_PAGING_H
#define TOMOS_PAGING_H

#include <stdint.h>

#define PG_PRESENT 0x01
#define PG_WRITE   0x02
#define PG_USER    0x04

#define PG_DIR_IDX(x) ((uint32_t)x/1024)
#define PG_TABLE_IDX(x) ((uint32_t)x%1024)

typedef uint32_t page_t;

typedef uint32_t page_directory_t;

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
//page_t get_page(uint32_t va, page_directory_t *dir);
//void alloc_frame(page_t *p, int kernel, int write);
int get_phys_addr(uint32_t va, uint32_t *pa);

#endif
