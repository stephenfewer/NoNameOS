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

#include <kernel/idt.h>
#include <kernel/kernel.h>
#include <kernel/isr.h>
#include <kernel/lib/string.h>

struct IDT_ENTRY   idt_table[IDT_ENTRYS];

struct IDT_POINTER idt_pointer;

ISR isr_stubs[] =
{
    isr00, isr01, isr02, isr03, isr04, isr05, isr06, isr07, 
	isr08, isr09, isr10, isr11, isr12, isr13, isr14, isr15,
	isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
	isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

void idt_setEntry( BYTE i, ISR isr_routine, WORD selector, BYTE flags )
{
	idt_table[i].base_high = ((DWORD)isr_routine & 0xFFFF0000) >> 16;

	idt_table[i].base_low  = ((DWORD)isr_routine & 0xFFFF);

	idt_table[i].flags = flags;

	idt_table[i].reserved = 0;

	idt_table[i].selector = selector;
}

void idt_init()
{
	int i;

    idt_pointer.limit = ( sizeof(struct IDT_ENTRY) * IDT_ENTRYS ) - 1;
    idt_pointer.base = (unsigned int)&idt_table;

    memset( (void *)&idt_table, 0x00, sizeof(struct IDT_ENTRY) * IDT_ENTRYS );

	for( i=INT0 ; i<INT32 ; i++ )
	{
		idt_setEntry(  i, isr_stubs[i], 0x08, 0x8E );
		isr_setHandler( i, NULL );
	}

	ASM( "lidt (%0)" : : "r" ( &idt_pointer) );
}

