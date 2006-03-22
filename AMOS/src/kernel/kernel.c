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
#include <kernel/kprintf.h>
#include <lib/amos.h>
#include <lib/string.h>
#include <lib/printf.h>

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

void kernel_printInfo( void )
{
	scheduler_printProcessTable();
}

void kernel_printf( char * text, ... )
{
	va_list args;
	// find the first argument
	va_start( args, text );
	// pass print the kernels std output handle the format text and the first argument
	kprintf( kernel_process.handles[PROCESS_CONSOLEHANDLE], text, args );
}

void debug_putstr( char * str )
{
	while( *str )
		outportb( 0xE9, *str++ );
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
	{
		kernel_printf( "%s\n", message );
		// for testing in Bochs
		debug_putstr( "[AMOS KP] " );
		debug_putstr( message );
	}
	// print out the stack contents
	if( stack != NULL )
		process_printStack( stack );
	// hang the system
	while( TRUE );
}

void kernel_idle( void )
{
	while( TRUE )
		ASM( "hlt" );	
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
	if( interrupt_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the interrupt layer." );
	// setup our memory manager
	if( mm_init( m->mem_upper ) == FAIL )
		kernel_panic( NULL, "Failed to initilize the Memory Manager subsystem." );
	// setup the virtual file system
	if( vfs_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the Virtual File System subsystem." );
	// setup the io subsystem
	if( io_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the IO subsystem." );
	// open the kernels console
	kernel_process.handles[PROCESS_CONSOLEHANDLE] = vfs_open( "/device/console0", VFS_MODE_READWRITE );
	if( kernel_process.handles[PROCESS_CONSOLEHANDLE] == NULL )
		kernel_panic( NULL, "Failed to open the kernel console." );
	// setup our system calls
	if( syscall_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the system call layer." );
	// setup scheduling
	if( scheduler_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the Process Manager subsystem." );
	// enable interrutps
	interrupt_enableAll();
	// return success
	return SUCCESS;
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	// initilize the kernel, when we return we will be executing as the kernel process
	kernel_init( m );

	// mount the primary file system
	kernel_printf( "Mounting primary file system. " );
	if( vfs_mount( "/device/floppy1", "/", FAT_TYPE ) == FAIL )
		kernel_panic( NULL, "Kernel failed to mount primary file system." );
	kernel_printf( "Done.\n" );	

	kernel_printf( "\nWelcome! - Press keys F1 to F4 to navigate virtual consoles\n\n" );
	
	// spawn a user shell on each virtual console
	process_spawn( &kernel_process, "/amos/shell.bin", "/device/console1" );
	process_spawn( &kernel_process, "/amos/shell.bin", "/device/console2" );
	process_spawn( &kernel_process, "/amos/shell.bin", "/device/console3" );
	process_spawn( &kernel_process, "/amos/shell.bin", "/device/console4" );
	
	// enter an idle state, the kernel is now our idle process if theirs nothing to do
	kernel_idle();
	
	// we should never reach here
	kernel_panic( NULL, "Kernel trying to exit." );
}

