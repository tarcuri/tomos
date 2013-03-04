#ifndef TOMOS_PAGING_H
#define TOMOS_PAGING_H

#include <stdint.h>

#define PG_PRESENT 0x01
#define PG_WRITE   0x02
#define PG_USER    0x04

typedef uint32_t page_t;

typedef uint32_t page_directory_t;

// functions
void pg_init(void);
void pg_page_fault(uint32_t error);

//page_t *get_page(uint32_t addr, int make, page_directory_t *dir);
//void alloc_frame(page_t *p, int kernel, int write);

#endif
