/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/isr.h>
#include <kernel/idt.h>
#include <kernel/kprintf.h>
#include <kernel/kernel.h>
#include <kernel/tasking/task.h>

ISR_HANDLER	isr_handlers[IDT_ENTRYS];

char * isr_messages[] =
{
    "Divide By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

DWORD isr_dispatcher( struct TASK_STACK * taskstack )
{
	kernel_lock();
	
	DWORD ret = (DWORD)NULL;
	ISR_HANDLER isr_handler = isr_handlers[ taskstack->intnumber ];

	if( isr_handler != NULL )
	{
		ret = isr_handler( taskstack );
	}
	else
	{
		if( taskstack->intnumber < 32 )
		{
			kprintf( "isr_dispatcher() - %s\n", isr_messages[ taskstack->intnumber ] );
			while(TRUE);
		}
	}
	
	// if this was an IRQ we must signal an EOI to the PIC
	if( taskstack->intnumber >= 40 && taskstack->intnumber < 48 )
        outportb( PIC_2, EOI );
	else if( taskstack->intnumber >= 32 && taskstack->intnumber < 48 )
		outportb( PIC_1, EOI );
	
	kernel_unlock();
	
	return ret;
}

void isr_setHandler( int i, ISR_HANDLER isr_handler )
{
	isr_handlers[i] = isr_handler;
}

