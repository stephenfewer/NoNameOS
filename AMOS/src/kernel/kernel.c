#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/debug.h>

BYTE * memset( BYTE * dest, BYTE val, int count )
{
    BYTE * temp = dest;
    for( ; count !=0 ; count-- )
    	*temp++ = val;
    return dest;
}

BYTE inportb( WORD port )
{
    BYTE rv;
    __asm__ __volatile__ ( "inb %1, %0" : "=a" (rv) : "dN" (port) );
    return rv;
}

void outportb( WORD port, BYTE data )
{
    __asm__ __volatile__ ( "outb %1, %0" : : "dN" (port), "a" (data) );
}

extern struct PAGE_DIRECTORY * pagedirectory;

extern struct GDT_POINTER	gdt_pointer;

int main( struct MULTIBOOT_INFO * m )
{
	console_init();
	
	kprintf( "AMOS %d.%d.%d\n", AMOS_MAJOR_VERSION, AMOS_MINOR_VERSION, AMOS_PATCH_VERSION );
	
	gdt_init();
	
	idt_init();

	irq_init();
	
	mm_init( m->mem_upper );
	
	sti();
	
    for(;;);

	return 0;
}

