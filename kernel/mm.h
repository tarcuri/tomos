#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

// kernel memory manager, allocates space for kernel data structures
#define MM_FRAME_SIZE			(0x1000)
#define MM_FRAME_ADDRESS(idx)		(mm_high_mem_base + (idx * MM_FRAME_SIZE))
#define MM_FRAME_ENABLE_BM(idx)		(mm_bit_map[ (idx/8) ] |= (1 << (idx % 8)))
#define MM_FRAME_DISABLE_BM(idx)	(mm_bit_map[ (idx/8) ] &= (0xff ^ (1 << (idx % 8))))

// define 'allocable' memory
// this needs to be heavily documented.
uint32_t _memory_ceiling;
extern uint32_t kernel_start;	// defined in linker script
extern uint32_t kernel_end;

uint32_t mm_highest_allocd;		// highest allocated memory address

uint32_t mm_high_mem_base;		// memory past kernel
uint32_t mm_high_mem_limit;		// memory limit (size pase base)
uint32_t mm_kernel_end;			// kernel end address
uint32_t mm_kernel_end_aligned;	// kernel end aligned for 4K boundary
uint32_t mm_kernel_size;		// kernel size in bytes

unsigned char *mm_bit_map;			// tracks allocated frames
uint32_t mm_bit_map_length;		// bitmap should be mm_total_frames/8 bytes long
uint32_t mm_allocated_frames;
uint32_t mm_last_allocated_frame;	// index of the most recently allocated frame
uint32_t mm_total_frames;			// total number of frames available

// functions
void mm_init(void *mbd, int print);

// allocate a 4KB aligned page frame
void *mm_alloc_frame(uint32_t idx);
void mm_set_frame(void *);

// perform grub multiboot initialization
void mm_grub_multiboot(void *mbd, int print);


// private static functions
static uint32_t mm_get_free_frame(void);

#endif
