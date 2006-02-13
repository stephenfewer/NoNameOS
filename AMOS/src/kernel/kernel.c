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
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/io/io.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/fat.h>
#include <kernel/syscall.h>
#include <kernel/lib/printf.h>
#include <kernel/lib/string.h>

struct PROCESS_INFO kernel_process;

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
	struct VFS_HANDLE * kernel_console;
	// lock protected code
	kernel_lock();
	// clear the kernels process structure
	memset( &kernel_process, 0x00, sizeof(struct PROCESS_INFO) );
	// set its default id
	kernel_process.id = KERNEL_PID;
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
	// open the kernels console
	kernel_console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( kernel_console == NULL )
		kernel_panic();
	kernel_process.console = kernel_console;	
	// unlock protected code
	kernel_unlock();
}

void kernel_printf( char * text, ... )
{
	va_list args;
	// find the first argument
	va_start( args, text );
	// pass printf the kernels std output handle the format text and the first argument
	printf( kernel_process.console, text, args );
}

void kernel_panic( void )
{
	// attempt to display a message
	kernel_printf( "AMOS Kernel Panic!\n" );
	// hang the system
	while( TRUE );
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	struct VFS_HANDLE * console;
	
	// initilize the kernel
	kernel_init( m );
	
	kernel_printf( "Welcome! - Press keys F1 to F4 to navigate virtual consoles\n\n" );

	// mount the root file system
	kernel_printf( "mounting device /device/floppy1 to /fat/ as a FAT file system. " );
	vfs_mount( "/device/floppy1", "/fat/", FAT_TYPE );
	kernel_printf( "done.\n" );
	
	console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( console != NULL )
	{
		scheduler_addProcess( process_create( (void*)&task2, 4096 ) );

		//scheduler_addProcess( process_create( (void*)&task1, 4096 ) );
		
		process_spawn( "/fat/BOOT/TEST.BIN", console );
		
		scheduler_enable();
		
		kernel_shell( console );
	} else {
		kernel_printf( "failed to open /device/console1\n" );
	}

	kernel_printf( "hanging: -->infiniteloop<--\n" );
	// after scheduling is enabled we should never reach here
	kernel_panic();
}

