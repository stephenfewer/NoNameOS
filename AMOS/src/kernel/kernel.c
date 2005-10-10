#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/debug.h>
#include <kernel/tasking/tasking.h>

int kernel_lockCount = 0;

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
    ASM( "inb %1, %0" : "=a" (rv) : "dN" (port) );
    return rv;
}

void outportb( WORD port, BYTE data )
{
    ASM( "outb %1, %0" : : "dN" (port), "a" (data) );
}

DWORD kernel_getESP()
{
	DWORD kernel_esp;
	ASM( "movl %%esp, %0" : "=r" (kernel_esp) );
	return kernel_esp;
}

void kernel_lock()
{
	cli();
	kernel_lockCount++;
}

void kernel_unlock()
{
	//kernel_lock--;
	if( --kernel_lockCount == 0 )
		sti();
}

void kernel_init( struct MULTIBOOT_INFO * m )
{
	//char * p, * q;

	{
		kernel_lock();
		
		console_init();
		
		gdt_init();
		
		idt_init();
	
		irq_init();
		
		mm_init( m->mem_upper );
		
		tasking_init();
		
		kernel_unlock();
	}
/*
	p = mm_malloc( 32 );
	kprintf( "p = %x\n", p );
	//mm_free( p );
	q = mm_malloc( 32 );
	kprintf( "q = %x\n", q );
*/
    while(TRUE);
}

