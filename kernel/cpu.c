#include "cpu.h"

inline void cpuid(int code, uint32_t* a, uint32_t* d)
{
        asm volatile ("cpuid"
                        : "=a"(*a), "=d"(*d)
                        : "0"(code)
                        : "ebx", "ecx");
}

int cpu_has_msr(void)
{
        uint32_t a, d; // eax, edx
        cpuid(1, &a, &d);

        return d & CPUID_FLAG_MSR;
}

void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
        asm volatile ("rdmsr"
                        : "=a"(*lo), "=d"(*hi)
                        : "c"(msr));
}

void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi)
{
        asm volatile ("wrmsr"
                        :
                        : "a"(lo), "d"(hi), "c"(msr));
}
