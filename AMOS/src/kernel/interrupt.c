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

#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/paging.h>
#include <kernel/pm/scheduler.h>
#include <lib/string.h>

struct INTERRUPT_TABLE_ENTRY interrupt_table[INTERRUPT_TABLE_ENTRYS];

INTERRUPT_HANDLER interrupt_handlers[INTERRUPT_TABLE_ENTRYS];

struct INTERRUPT_TABLE_POINTER interrupt_ptable;

INTERRUPT_SERVICE_ROUTINE interrupt_stubs[] =
{
    isr00, isr01, isr02, isr03, isr04, isr05, isr06, isr07, 
	isr08, isr09, isr10, isr11, isr12, isr13, isr14, isr15,
	isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
	isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
    isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39,
	isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47,
	isr48
};

char * interrupt_messages[] =
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

struct PROCESS_INFO * interrupt_dispatcher( struct PROCESS_INFO * process )
{
	struct PROCESS_INFO * newProcess;
	INTERRUPT_HANDLER handler;

	handler = interrupt_handlers[ process->kstack->intnumber ];

	if( handler != NULL )
	{
		newProcess = handler( process );
	}
	else
	{
		newProcess = process;
		// if its an exception we must do something by default if their is no appropriate handler
		if( process->kstack->intnumber <= INT31 )
		{
			if( process == NULL )
			{
				kernel_printf("An exception has occurred in an unknown process\n\t- %s\n", interrupt_messages[process->kstack->intnumber] );
				while(TRUE);
			} else {
				// if the process that caused the exception is the kernel, we must kernel panic
				if( process->id == KERNEL_PID )
					kernel_panic( process->kstack, interrupt_messages[process->kstack->intnumber] );
				else {
					kernel_printf("Exception: pid: %d %s\n", process->id, interrupt_messages[process->kstack->intnumber] );
					process_printStack( process->kstack );
					if( process_kill( process->id ) == SUCCESS )
						newProcess = scheduler_select( process );
				}
			}
			
		}
	}
	
	// if this was an IRQ we must signal an EOI to the PIC
	if( process->kstack->intnumber >= IRQ8 && process->kstack->intnumber <= IRQ15 )
        outportb( INTERRUPT_PIC_2, INTERRUPT_EOI );
	else if( process->kstack->intnumber >= IRQ0 && process->kstack->intnumber <= IRQ15 )
		outportb( INTERRUPT_PIC_1, INTERRUPT_EOI );
	
	return newProcess;
}

BOOL interrupt_setHandler( int index, INTERRUPT_HANDLER handler )
{
	if( index < INTERRUPT_TABLE_ENTRYS && index >= 0 )
	{
		interrupt_handlers[index] = handler;
		return TRUE;
	}	
	return FALSE;
}

void interrupt_setTableEntry( BYTE index, INTERRUPT_SERVICE_ROUTINE routine, BYTE privilege, BYTE present )
{
	interrupt_table[index].base_high = ((DWORD)routine & 0xFFFF0000) >> 16;
	
	interrupt_table[index].base_low  = ((DWORD)routine & 0xFFFF);

	interrupt_table[index].present = present;
	
	if( privilege == USER )
		interrupt_table[index].DPL = RING3;
	else
		interrupt_table[index].DPL = RING0;
	
	// this is slightly innacurate, we need to set the D bit here to 1 for the size of the gate (32bit in our case)
	// while also setting two other bits, so we shortcut and just set it to 14decimal which is 01110 binary
	interrupt_table[index].size = 14;
		
	interrupt_table[index].reserved = 0;
	
	interrupt_table[index].selector = KERNEL_CODE_SEL;
}

void interrupt_remapPIC( void )
{
    outportb( 0x20, 0x11 );
    outportb( 0xA0, 0x11 );

    outportb( 0x21, 0x20 );
    outportb( 0xA1, 0x28 );

    outportb( 0x21, 0x04 );
    outportb( 0xA1, 0x02 );

    outportb( 0x21, 0x01 );
    outportb( 0xA1, 0x01 );

    outportb( 0x21, 0x00 );
    outportb( 0xA1, 0x00 );
}

BOOL interrupt_enable( int index, INTERRUPT_HANDLER handler, BYTE privilege )
{
	if( index < INTERRUPT_TABLE_ENTRYS && index >= 0 )
	{
		interrupt_setTableEntry( index, interrupt_stubs[index], privilege, TRUE );
		if( handler != NULL )
			return interrupt_setHandler( index, handler );
		return TRUE;
	}
	return FALSE;
}

BOOL interrupt_disable( int index )
{
	if( index < INTERRUPT_TABLE_ENTRYS && index >= 0 )
	{
		INTERRUPT_SERVICE_ROUTINE stub;

		if( index >= IRQ0 && index < IRQ8 )
			stub = disable_irqA;
		else if( index >= IRQ8 && index <= IRQ15 )
			stub = disable_irqB;
		else if( index == INT8 || (index >= INT10 && index <= INT14) )
			stub = disable_intB;
		else
			stub = disable_intA;
		interrupt_setTableEntry( index, stub, SUPERVISOR, TRUE );
		// return success
		return TRUE;
	}
	// return fail
	return FALSE;
}

int interrupt_init( void )
{
	int index;
	// patch in the values for the IDT pointer
    interrupt_ptable.limit = ( sizeof(struct INTERRUPT_TABLE_ENTRY) * INTERRUPT_TABLE_ENTRYS ) - 1;
    interrupt_ptable.base = (DWORD)&interrupt_table;
	// clear the interrupt descriptor table
    memset( (void *)&interrupt_table, 0x00, sizeof(struct INTERRUPT_TABLE_ENTRY) * INTERRUPT_TABLE_ENTRYS );
	// initially we clear all are interrupt handlers and disable all interrupts
	for( index=0 ; index<INTERRUPT_TABLE_ENTRYS ; index++ )
	{
		interrupt_handlers[index] = NULL;
		interrupt_disable( index );
	}
	// enable the first 32 interrupts but dont set a handler
	for( index=INT0 ; index<=INT31 ; index++ )
		interrupt_enable( index, NULL, SUPERVISOR );
	// remap the Programable Interrupt Controller
	interrupt_remapPIC();
	// load the interrupt descriptor table (interrupt_ptable pointer to a linear address of the interrupt_table)
	ASM( "lidt (%0)" : : "r" ( &interrupt_ptable) );
	return SUCCESS;
}
