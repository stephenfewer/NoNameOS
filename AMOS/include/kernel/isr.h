#ifndef _KERNEL_ISR_H_
#define _KERNEL_ISR_H_

#define PIC_1		0x20
#define PIC_2		0xA0

#define EOI			0x20

struct REGISTERS
{
    unsigned int ds, es, fs, gs;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} PACKED;

typedef void (* ISR_HANDLER)( struct REGISTERS * );

void isr_dispatcher( struct REGISTERS * );

void isr_setHandler( int, ISR_HANDLER );

#endif

