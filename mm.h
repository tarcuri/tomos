#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// kernel memory manager, allocates space for kernel data structures
#define MM_FRAME_SIZE		(0x1000)
#define MM_FRAME_ADDRESS(idx)	(mm_high_mem_base + (idx * MM_FRAME_SIZE))

// define 'allocable' memory
extern unsigned int _memory_ceiling;

unsigned int mm_high_mem_base;		// memory past kernel
unsigned int mm_high_mem_limit;		// memory limit (size pase base)
unsigned int mm_kernel_end;		// kernel end address
unsigned int mm_kernel_end_aligned;	// kernel end aligned for 4K boundary
unsigned int mm_kernel_size;		// kernel size in bytes

unsigned int mm_total_frames;		// total number of frames available

// management functions
void mm_init(void);

void *mm_alloc_frame(void);

// private static functions
static unsigned int mm_get_free_frame(void);

#endif
