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

#include <kernel/kernel.h>
#include <kernel/interrupt.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/kprintf.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/io/io.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/fat.h>
#include <kernel/lib/printf.h>
#include <kernel/syscall.h>

// the global handle for the kernels console
struct VFS_HANDLE * kernel_console = NULL;

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
	struct VFS_HANDLE * console = vfs_open( "/device/console2", VFS_MODE_READWRITE );
	kernel_shell( console );
	/*
	unsigned char* VidMemChar = (unsigned char*)0xB8000;
	*VidMemChar='1';
	for(;;)
	{
		if( *VidMemChar=='1' )
			*VidMemChar='2';
		else
			*VidMemChar='1';
	}*/
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
	if( --kernel_lockCount <= 0 )
		sti();
}

// initilize the kernel and bring up all the subsystems
void kernel_init( struct MULTIBOOT_INFO * m )
{
	// lock protected code
	kernel_lock();
	// setup interrupts
	interrupt_init();
	// setup our memory manager
	mm_init( m->mem_upper );
	// setup the virtual file system
	vfs_init();
	// setup the io subsystem
	io_init();
	// setup scheduling
	scheduler_init();
	// setup our system calls
	syscall_init();
	// unlock protected code
	kernel_unlock();
}

void kernel_panic( void )
{
	// attempt to display a message
	kprintf( "AMOS Kernel Panic!\n" );
	// hang the system
	while( TRUE );
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	struct VFS_HANDLE * console;
	
	// initilize the kernel
	kernel_init( m );

	// open the kernels console
	kernel_console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( kernel_console == NULL )
		kernel_panic();
	kprintf( "Welcome! - Press keys F1 to F4 to navigate virtual consoles\n\n" );

	// mount the root file system
	kprintf( "mounting device /device/floppy1 to /fat/ as a FAT file system. " );
	vfs_mount( "/device/floppy1", "/fat/", FAT_TYPE );
	kprintf( "done.\n" );
	
	console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( console != NULL )
	{
		scheduler_addProcess( process_create( (void*)&task1 ) );

		//scheduler_addProcess( process_create( (void*)&task2 ) );
		
		process_spawn( "/fat/BOOT/TEST.BIN", console );
		
		scheduler_enable();
		
		kernel_shell( console );
	} else {
		kprintf( "failed to open /device/console1\n" );
	}

	kprintf( "hanging: -->infiniteloop<--\n" );
	// after scheduling is enabled we should never reach here
	kernel_panic();
}

