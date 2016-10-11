#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define CPUID_FLAG_MSR          (1 << 5)
#define CPUID_FLAG_APIC         (1 << 9)

void cpuid(int code, uint32_t *a, uint32_t *d);
int cpu_has_msr(void);
void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi);
void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi);

#endif
