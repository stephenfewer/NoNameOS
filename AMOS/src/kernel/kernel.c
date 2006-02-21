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

void kernel_printf( char * text, ... )
{
	va_list args;
	// find the first argument
	va_start( args, text );
	// pass print the kernels std output handle the format text and the first argument
	printf( kernel_process.handles[PROCESS_CONSOLEHANDLE], text, args );
}

// ..."this is the end. beautiful friend, the end."
void kernel_panic( struct PROCESS_STACK * stack, char * message )
{
	// disable interrupts as we can go no further
	interrupt_disableAll();
	// attempt to print out some info
	kernel_printf( "AMOS Kernel Panic! " );
	// print out the message
	if( message != NULL )
		kernel_printf( "%s\n", message );
	// print out the stack contents
	if( stack != NULL )
		process_printStack( stack );
	// hang the system
	while( TRUE );
}

// initilize the kernel and bring up all the subsystems
int kernel_init( struct MULTIBOOT_INFO * m )
{
	// we disable interrupts forthe duration of the kernels initilization
	interrupt_disableAll();
	// clear the kernels process structure
	memset( &kernel_process, 0x00, sizeof(struct PROCESS_INFO) );
	// set its default id
	kernel_process.id = KERNEL_PID;
	// set its privilege to SUPERVISOR as this is the kernel
	kernel_process.privilege = SUPERVISOR;
	// setup interrupts
	interrupt_init();
	// setup our memory manager
	mm_init( m->mem_upper );
	// setup the virtual file system
	vfs_init();
	// setup the io subsystem
	io_init();
	// open the kernels console
	kernel_process.handles[PROCESS_CONSOLEHANDLE] = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( kernel_process.handles[PROCESS_CONSOLEHANDLE] == NULL )
		kernel_panic( NULL, "Failed to open the kernel console." );
	// setup our system calls
	syscall_init();
	// setup scheduling
	scheduler_init();
	// enable interrutps
	interrupt_enableAll();
	// return success
	return SUCCESS;
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	struct VFS_HANDLE * console;
	
	// initilize the kernel, when we return we will executing as the kernel process
	kernel_init( m );
	
	kernel_printf( "Welcome! - Press keys F1 to F4 to navigate virtual consoles\n\n" );

	// mount the root file system
	kernel_printf( "mounting device /device/floppy1 to /fat/ as a FAT file system. " );
	vfs_mount( "/device/floppy1", "/fat/", FAT_TYPE );
	kernel_printf( "done.\n" );

	console = vfs_open( "/device/console3", VFS_MODE_READWRITE );
	if( console != NULL )
		process_spawn( "/fat/BOOT/TEST.BIN", console );
	else
		kernel_printf( "failed to open /device/console3\n" );

	kernel_printf( "About to hang the kernel process.\n" );	
	while( TRUE );

	// after scheduling is enabled we should never reach here
	kernel_panic( NULL, "Kernel trying to exit." );
}

