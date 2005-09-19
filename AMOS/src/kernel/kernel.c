#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>

BYTE * memset( BYTE * dest, BYTE val, int count )
{
    BYTE * temp = dest;
    for( ; count !=0 ; count-- ) *temp++ = val;
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

int main( struct MULTIBOOT_INFO * m )
{
	console_init();

    kprintf( "AMOS %d.%d.%d\n", AMOS_MAJOR_VERSION, AMOS_MINOR_VERSION, AMOS_PATCH_VERSION );
    
    kprintf( "gdt: ");
	gdt_init();
	kprintf( "done\n");
	
	kprintf( "idt: ");
	idt_init();
	kprintf( "done\n");
	
	kprintf( "irg: ");
	irq_init();
	kprintf( "done\n");

    kprintf( "mm: ");
	mm_init( m->mem_upper );
	kprintf( "done\n");

	kprintf( "sti: ");
	sti();
	kprintf( "done\n");
	
	//__asm__ __volatile__("int %0" : : "i"(14));
    
    for(;;);

	return 0;
}

