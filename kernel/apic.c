#include "apic.h"
#include "cpu.h"

#define IA32_APIC_BASE_MSR              0x1B
#define IA32_APIC_BASE_MSR_BSP          0x100
#define IA32_APIC_BASE_MSR_ENABLE       0x800

int check_apic(void)
{
        uint32_t eax, edx;
        cpuid(1, &eax, &edx);

        return edx & CPUID_FLAG_APIC;
}

/* set physical address for local APIC registers */
void cpu_set_apic_base(uintptr_t apic)
{
        uint32_t edx = 0;
        uint32_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
 
#ifdef __PHYSICAL_MEMORY_EXTENSION__
        edx = (apic >> 32) & 0x0f;
#endif
 
        cpu_set_msr(IA32_APIC_BASE_MSR, eax, edx);
}

/*
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpu_get_apic_base(void)
{
   uint32_t eax, edx;
   cpu_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);
 
#ifdef __PHYSICAL_MEMORY_EXTENSION__
   return (eax & 0xfffff000) | ((edx & 0x0f) << 32);
#else
   return (eax & 0xfffff000);
#endif
}

void enable_apic(void)
{
        /* HW enable local APIC if it wasn't already */
        cpu_set_apic_base(cpu_get_apic_base());

        /* Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts */
        // write_reg(0xF0, read_register(0xF0) | 0x100);
}
