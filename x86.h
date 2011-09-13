#ifndef X86_H
#define X86_H


// cr0
#define X86_CR0_PAGING		0x80000000

/*
** IDT Descriptors
** 
** IA-32 V3, page 5-13.
**
** All have a segment selector in bytes 2 and 3; Task Gate descriptors
** have bytes 0, 1, 4, 6, and 7 reserved; others have bytes 0, 1, 6,
** and 7 devoted to the 16 bits of the Offset, with the low nybble of
** byte 4 reserved.
*/
#define IDT_PRESENT             0x8000
#define IDT_DPL_MASK            0x6000
#       define          IDT_DPL_0       0x0000
#       define          IDT_DPL_1       0x2000
#       define          IDT_DPL_2       0x4000
#       define          IDT_DPL_3       0x6000
#define IDT_GATE_TYPE           0x0f00
#       define          IDT_TASK_GATE   0x0500
#       define          IDT_INT16_GATE  0x0600
#       define          IDT_INT32_GATE  0x0e00
#       define          IDT_TRAP16_GATE 0x0700
#       define          IDT_TRAP32_GATE 0x0f00

/*
** Interrupt vectors
*/
#define INT_VEC_DIVIDE_ERROR            0x00
#define INT_VEC_DEBUG_EXCEPTION         0x01
#define INT_VEC_NMI_INTERRUPT           0x02
#define INT_VEC_BREAKPOINT              0x03
#define INT_VEC_INTO_DETECTED_OVERFLOW  0x04
#define INT_VEC_BOUND_RANGE_EXCEEDED    0x05
#define INT_VEC_INVALID_OPCODE          0x06
#define INT_VEC_DEVICE_NOT_AVAILABLE    0x07
#define INT_VEC_DOUBLE_EXCEPTION        0x08
#define INT_VEC_COPROCESSOR_OVERRUN     0x09
#define INT_VEC_INVALID_TSS             0x0a
#define INT_VEC_SEGMENT_NOT_PRESENT     0x0b
#define INT_VEC_STACK_FAULT             0x0c
#define INT_VEC_GENERAL_PROTECTION      0x0d
#define INT_VEC_PAGE_FAULT              0x0e

#define INT_VEC_COPROCESSOR_ERROR       0x10
#define INT_VEC_ALIGNMENT_CHECK         0x11
#define INT_VEC_MACHINE_CHECK           0x12
#define INT_VEC_SIMD_FP_EXCEPTION       0x13

#define INT_VEC_TIMER                   0x20		// IRQ0
#define INT_VEC_KEYBOARD                0x21

#define INT_VEC_SERIAL_PORT_2           0x23
#define INT_VEC_SERIAL_PORT_1           0x24
#define INT_VEC_PARALLEL_PORT           0x25
#define INT_VEC_FLOPPY_DISK             0x26
#define INT_VEC_MYSTERY                 0x27		// IRQ7

#define INT_VEC_PRI_IDE			0x2E		// IRQ14
#define INT_VEC_SEC_IDE			0x2F

#define INT_VEC_MOUSE                   0x2C


/*
** Hardware timer (Intel 8254 Programmable Interval Timer)
*/
#define TIMER_DEFAULT_TICKS_PER_SECOND  18      /* default ticks per second */
#define TIMER_DEFAULT_MS_PER_TICK       (1000/TIMER_DEFAULT_TICKS_PER_SECOND)
#define TIMER_FREQUENCY                 1193182 /* clock cycles/sec  */
#define TIMER_BASE_PORT                 0x40    /* I/O port for the timer */
#define TIMER_0_PORT                    ( TIMER_BASE_PORT )
#define TIMER_1_PORT                    ( TIMER_BASE_PORT + 1 )
#define TIMER_2_PORT                    ( TIMER_BASE_PORT + 2 )
#define TIMER_CONTROL_PORT              ( TIMER_BASE_PORT + 3 )

/* Timer 0 settings */
#define TIMER_0_SELECT                  0xc0    /* select timer 0 */
#define TIMER_0_LOAD                    0x30    /* load LSB, then MSB */
#define TIMER_0_NDIV                    0x04    /* divide-by-N counter */
#define TIMER_0_SQUARE                  0x06    /* square-wave mode */
#define TIMER_0_ENDSIGNAL               0x00    /* assert OUT at end of count */

/* Timer 1 settings */
#define TIMER_1_SELECT                  0x40    /* select timer 1 */
#define TIMER_1_READ                    0x30    /* read/load LSB then MSB */
#define TIMER_1_RATE                    0x06    /* square-wave, for USART */

// TODO: redo this PIC stuff...someday

/* Definitions for 8259 Programmable Interrupt Controller */

#define PIC_NEEDICW4    0x01            /* ICW4 needed */
#define PIC_ICW1BASE    0x10            /* base for ICW1 */
#define PIC_LTIM        0x08            /* level-triggered mode */
#define PIC_86MODE      0x01            /* MCS 86 mode */
#define PIC_AUTOEOI     0x02            /* do auto eoi's */ 
#define PIC_SLAVEBUF    0x08            /* put slave in buffered mode */
#define PIC_MASTERBUF   0x0C            /* put master in buffered mode */
#define PIC_SPFMODE     0x10            /* special fully nested mode */
#define PIC_READISR     0x0B            /* Read the ISR */
#define PIC_READIRR     0x0A            /* Read the IRR */
#define PIC_EOI         0x20            /* Non-specific EOI command */
#define PIC_SEOI        0x60            /* specific EOI command */
#define PIC_SEOI_LVL7   (PIC_SEOI | 0x7)        /* specific EOI for level 7 */
                
/*      
 * Interrupt configuration information specific to a particular computer.
 * These constants are used to initialize tables in modules/pic/space.c.
 * NOTE: The master pic must always be pic zero.
 */

#define NPIC            2               /* 2 PICs */
/*
** Port addresses for the command port and interrupt mask register port
** for both teh master and slave PICs.
*/
#define PIC_MASTER_CMD_PORT     0x20    /* master command */
#define PIC_MASTER_IMR_PORT     0x21    /* master intr mask register */
#define PIC_SLAVE_CMD_PORT      0xA0    /* slave command */
#define PIC_SLAVE_IMR_PORT      0xA1    /* slave intr mask register */
#define PIC_MASTER_SLAVE_LINE   0x04    /* bit mask: slave id */
#define PIC_SLAVE_ID            0x02    /* integer: slave id */
#define PIC_BUFFERED            0       /* PICs not in buffered mode */


#endif
