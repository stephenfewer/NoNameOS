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
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/kprintf.h>
#include <kernel/tasking/scheduler.h>
#include <kernel/tasking/task.h>
#include <kernel/io/io.h>
#include <kernel/fs/vfs.h>
#include <kernel/lib/printf.h>

// the global handle for the kernels standard output
struct VFS_HANDLE * kernel_kout;

volatile int kernel_lockCount = 0;

void kernel_shell( struct VFS_HANDLE * c )
{
	struct VFS_HANDLE * console;
	char buffer[128];
	
	console = c;
	
	while( TRUE )
	{
		print( console, "[AMOS]>" );
		
		get( console, (char *)&buffer, 128 );

		print( console, "got: %s\n", (char *)&buffer );
	}	
}

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

// initilize the kernel and bring up all the subsystems
void kernel_init( struct MULTIBOOT_INFO * m )
{
	// lock protected code
	kernel_lock();
	// setup the interrupt descriptor table
	idt_init();
	// setup interrupts
	irq_init();
	// setup our memory manager
	mm_init( m->mem_upper );
	// setup the virtual file system
	vfs_init();
	// setup the io subsystem
	io_init();
	// setup scheduling
	scheduler_init();
	// unlock protected code
	kernel_unlock();
}

void kernel_panik( void )
{
	// hang the system
	while( TRUE );	
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	// initilize the kernel
	kernel_init( m );

	// open the standard kernel output
	kernel_kout = vfs_open( "/device/console1" );
	if( kernel_kout == NULL )
		kernel_panik();
		
	kprintf( "Welcome!\n" );

	struct VFS_HANDLE * console;
	console = vfs_open( "/device/console2" );
	if( console != NULL )
		kernel_shell( console );

/*	kprintf( "\nMultitasking Test:\n" );
	kprintf( "\tCreating task 1.\n" );
	task_create( task1 );
	kprintf( "\tCreating task 2.\n" );
	task_create( task2 );
	kprintf( "\tEnabling scheduler.\n" );
	scheduler_enable(); */
	
	// after scheduling is enabled we should never reach here
	kernel_panik();
}

