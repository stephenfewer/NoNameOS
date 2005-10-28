#include <kernel/kernel.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/kprintf.h>
#include <kernel/tasking/scheduler.h>
#include <kernel/io/io.h>

int kernel_lockCount = 0;

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

inline void kernel_lock()
{
	cli();
	kernel_lockCount++;
}

inline void kernel_unlock()
{
	if( --kernel_lockCount == 0 )
		sti();
}

void kernel_init( struct MULTIBOOT_INFO * m )
{
	// lock protected code
	kernel_lock();
	// setup the global descriptor table
	gdt_init();
	// setup the interrupt descriptor table
	idt_init();
	// setup interrupts
	irq_init();
	// setup our memory manager
	mm_init( m->mem_upper );
	// setup the io subsystem
	io_init();
	// setup scheduling
	scheduler_init();
	// unlock protected code
	kernel_unlock();
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	kernel_init( m );
	
	kprintf( "AMOS %d.%d.%d\n", AMOS_MAJOR_VERSION, AMOS_MINOR_VERSION, AMOS_PATCH_VERSION );
		
	while(TRUE);
}

