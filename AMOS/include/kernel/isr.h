#ifndef _KERNEL_ISR_H_
#define _KERNEL_ISR_H_

#include <sys/types.h>

#define PIC_1				0x20
#define PIC_2				0xA0

#define PIT_TIMER_0			0x40
#define PIT_TIMER_1			0x41
#define PIT_TIMER_2			0x42
#define PIT_COMMAND_REG		0x43

#define EOI					0x20

enum { 
	INT0 = 0,
	INT1,
	INT2,
	INT3,
	INT4,
	INT5,
	INT6,
	INT7,
	INT8,
	INT9,
	INT10,
	INT11,
	INT12,
	INT13,
	INT14,
	INT15,
	INT16,
	INT17,
	INT18,
	INT19,
	INT20,
	INT21,
	INT22,
	INT23,
	INT24,
	INT25,
	INT26,
	INT27,
	INT28,
	INT29,
	INT30,
	INT31,
	INT32,
};

enum { 
	IRQ0 = 32,
	IRQ1,
	IRQ2,
	IRQ3,
	IRQ4,
	IRQ5,
	IRQ6,
	IRQ7,
	IRQ8,
	IRQ9,
	IRQ10,
	IRQ11,
	IRQ12,
	IRQ13,
	IRQ14,
	IRQ15,
	IRQ16,
};

struct REGISTERS
{
    unsigned int ds, es, fs, gs;
    DWORD edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} PACKED;

typedef DWORD (* ISR_HANDLER)( struct REGISTERS * );

DWORD isr_dispatcher( struct REGISTERS * );

void isr_setHandler( int, ISR_HANDLER );

#endif

