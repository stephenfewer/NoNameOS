/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    License: GNU General Public License (GPL)
 */

#include <kernel/kernel.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/kprintf.h>
#include <kernel/tasking/scheduler.h>
#include <kernel/tasking/task.h>
#include <kernel/io/io.h>

int kernel_lockCount = 0;

void task1()
{
	unsigned char* VidMemChar = (unsigned char*)0xB8000;
	*VidMemChar='1';
	for(;;)
	{
		if( *VidMemChar=='1' )
			*VidMemChar='2';
		else
			*VidMemChar='1';
	}
}

void task2()
{
	unsigned char* VidMemChar = (unsigned char*)0xB8002;
	//unsigned char* crash = (unsigned char*)0xDEADC0DE;
	*VidMemChar='a';
	for(;;)
	{
		if( *VidMemChar=='a' )
			*VidMemChar='b';
		else{
			*VidMemChar='a';
			//*crash=0xDEADBEEF;
		}
	}
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

extern void start;
extern void * mm_heapTop;
extern void * mm_heapBottom;

void kernel_main( struct MULTIBOOT_INFO * m )
{
//	void * p, * q;
	
	kernel_init( m );
	
	kprintf( "AMOS %d.%d.%d\n", AMOS_MAJOR_VERSION, AMOS_MINOR_VERSION, AMOS_PATCH_VERSION );
/*
	kprintf( "\nSystem Info:\n" );
	kprintf( "\tPhysical Memory    = %d MB\n", (m->mem_upper/1024)+1 );
	kprintf( "\tPage Directory     = %x\n", paging_getCurrentPageDir() );
	kprintf( "\tKernel Start       = %x\n", &start );
	kprintf( "\tKernel Heap Bottom = %x\n", mm_heapBottom );
	kprintf( "\tKernel Heap Top    = %x\n", mm_heapTop );

	kprintf( "\nMalloc Test:\n" );
	p = mm_malloc( 512 );
	kprintf( "\tmalloc(512), p  = %x\n", p );
	q = mm_malloc( 4096 );
	kprintf( "\tmalloc(4096), q = %x\n", q );
	mm_free( p );
	kprintf( "\tfree( q )\n" );
	p = mm_malloc( 32 );
	kprintf( "\tmalloc(32)      = %x\n", p );
	kprintf( "\tKernel Heap Top = %x\n", mm_heapTop );

	kprintf( "\nMultitasking Test:\n" );
	kprintf( "\tCreating task 1.\n" );
	task_create( task1 );
	kprintf( "\tCreating task 2.\n" );
	task_create( task2 );
	kprintf( "\tEnabling scheduler.\n" );
	scheduler_enable();
*/
	while(TRUE);
}

