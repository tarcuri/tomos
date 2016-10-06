#ifndef APIC_H
#define APIC_H

//apic		= the linear address where you have mapped the APIC registers
 
APIC_APICID	20h
APIC_APICVER	30h
APIC_TASKPRIOR	80h
APIC_EOI	0B0h
APIC_LDR	0D0h
APIC_DFR	0E0h
APIC_SPURIOUS	0F0h
APIC_ESR	280h
APIC_ICRL	300h
APIC_ICRH	310h
APIC_LVT_TMR	320h
APIC_LVT_PERF	340h
APIC_LVT_LINT0	350h
APIC_LVT_LINT1	360h
APIC_LVT_ERR	370h
APIC_TMRINITCNT	380h
APIC_TMRCURRCNT	390h
APIC_TMRDIV	3E0h
APIC_LAST	38Fh
APIC_DISABLE	10000h
APIC_SW_ENABLE	100h
APIC_CPUFOCUS	200h
APIC_NMI	(4<<8)
TMR_PERIODIC	20000h
TMR_BASEDIV	(1<<20)

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
