#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>

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

extern struct PAGE_DIRECTORY * pagedirectory;

static inline void init( struct MULTIBOOT_INFO * m  )
{ 
	cli();
	
	mm_init( m->mem_upper );
	
	//idt_init();

	//irq_init();

	// Enable Paging...
	__asm__ __volatile__ ( "movl %%eax, %%cr3" : : "a" ( 0x00000000 ) );
	__asm__ __volatile__ ( "movl %cr0, %eax" );
	__asm__ __volatile__ ( "orl $0x80000000, %eax" );
	__asm__ __volatile__ ( "movl %eax, %cr0" );

	__asm__ __volatile__ ( "loop: jmp loop" );
	
/*
	// Enable flat segmentation... -0xC0000000+0x00101000
	__asm__ __volatile__ ( "ljmp $0x08, $gdtflush" );	
	__asm__ __volatile__ ( "gdtflush:" );
	__asm__ __volatile__ ( "movw $0x10, %ax" );
	__asm__ __volatile__ ( "movw %ax, %ds" );
	__asm__ __volatile__ ( "movw %ax, %es" );
	__asm__ __volatile__ ( "movw %ax, %ss" );
	__asm__ __volatile__ ( "movw %ax, %fs" );
	__asm__ __volatile__ ( "movw %ax, %gs" );
	__asm__ __volatile__ ( "loop: jmp loop" );
*/
	//gdt_init();
	
	//sti();
}

int main( struct MULTIBOOT_INFO * m )
{
	init( m );

	//console_init();
	//kprintf( "AMOS %d.%d.%d\n", AMOS_MAJOR_VERSION, AMOS_MINOR_VERSION, AMOS_PATCH_VERSION );

    //for(;;);

	return 0;
}

