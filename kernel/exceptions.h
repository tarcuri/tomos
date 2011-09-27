#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#define EXCEPT_DIVIDE			0x00
#define EXCEPT_DEBUG			0x01
#define EXCEPT_NM_INTERRUPT		0x02
#define EXCEPT_BREAKPOINT		0x03
#define EXCEPT_OVERFLOW			0x04
#define EXCEPT_BOUNDS_CHECK		0x05
#define EXCEPT_INVALID_OPCODE		0x06
#define EXCEPT_COPROCESSOR		0x07
#define EXCEPT_DOUBLE_FAULT		0x08
#define EXCEPT_COPROC_SEGMENT		0x09
#define EXCEPT_INVALID_TSS		0x0A
#define EXCEPT_SEG_NOT_PRESENT		0x0B
#define EXCEPT_STACK			0x0C
#define EXCEPT_GENERAL_PROTECTION	0x0D
#define EXCEPT_PAGE_FAULT		0x0E
#define EXCEPT_RESERVED			0x0F
#define EXCEPT_COPROC_ERROR		0x10
#define EXCEPT_RESERVED			0x11
// to
#define EXCEPT_RESERVED			0x1F

#endif