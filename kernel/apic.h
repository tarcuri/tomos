#ifndef APIC_H
#define APIC_H

//apic		= the linear address where you have mapped the APIC registers
 
#define APIC_APICID	0x20
#define APIC_APICVER	0x30
#define APIC_TASKPRIOR	0x80
#define APIC_EOI	0x0B0
#define APIC_LDR	0x0D0
#define APIC_DFR	0x0E0
#define APIC_SPURIOUS	0x0F0
#define APIC_ESR	0x280
#define APIC_ICRL	0x300
#define APIC_ICRH	0x310
#define APIC_LVT_TMR	0x320
#define APIC_LVT_PERF	0x340
#define APIC_LVT_LINT0	0x350
#define APIC_LVT_LINT1	0x360
#define APIC_LVT_ERR	0x370
#define APIC_TMRINITCNT	0x380
#define APIC_TMRCURRCNT	0x390
#define APIC_TMRDIV	0x3E0
#define APIC_LAST	0x38F
#define APIC_DISABLE	0x10000
#define APIC_SW_ENABLE	0x100
#define APIC_CPUFOCUS	0x200
#define APIC_NMI	(4<<8)
#define TMR_PERIODIC	0x20000
#define TMR_BASEDIV	(1<<20)

int check_apic(void);
 
#if 0
		;Interrupt Service Routines
isr_dummytmr:	mov			dword [apic+APIC_EOI], 0
		iret
isr_spurious:	iret
		;function to set a specific interrupt gate in IDT
		;al=interrupt
		;ebx=isr entry point
writegate:	...
		ret
#endif

#endif
