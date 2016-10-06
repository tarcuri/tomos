#include "apic.h"
#include "cpu.h"

int check_apic(void)
{
        uint32_t eax, edx;
        cpuid(1, &eax, &edx);
}
