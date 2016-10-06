#ifndef CPU_H
#define CPU_H

#include <stdint.h>

static void cpuid(int code, uint32_t *a, uint32_t *d);

#endif
