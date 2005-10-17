#include <kernel/isr.h>
#include <kernel/idt.h>
#include <kernel/console.h>
#include <kernel/kernel.h>

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

DWORD isr_dispatcher( struct REGISTERS * reg )
{
	DWORD ret = NULL;
	ISR_HANDLER isr_handler = isr_handlers[ reg->int_no ];

	if( isr_handler != NULL )
	{
		ret = isr_handler( reg );
	}
	else
	{
		if( reg->int_no < 32 )
		{
			kprintf( "isr_dispatcher() - %s\n", isr_messages[ reg->int_no ] );
			while(TRUE);
		}
	}
	
	// if this was an IRQ we must signal an EOI to the PIC
	if( reg->int_no >= 40 && reg->int_no < 48 )
        outportb( PIC_2, EOI );
	else if( reg->int_no >= 32 && reg->int_no < 48 )
		outportb( PIC_1, EOI );
		
	return ret;
}

void isr_setHandler( int i, ISR_HANDLER isr_handler )
{
	isr_handlers[i] = isr_handler;
}

