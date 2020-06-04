/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/kernel.h>
#include <kernel/multiboot.h>
#include <kernel/interrupt.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/io/io.h>
#include <kernel/io/pci.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/fat.h>
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/debug.h>
#include <lib/amos.h>
#include <lib/libc/string.h>

struct PROCESS_INFO kernel_process;

void kernel_printInfo( void )
{
	scheduler_printProcessTable();
}

void kernel_printf( char * text, ... )
{
	va_list args;
	// find the first argument
	va_start( args, text );
	// print it out to the debug device
	debug_printf( text, args );
	// pass print the kernels std output handle the format text and the first argument
	kprintf( kernel_process.handles[PROCESS_CONSOLEHANDLE], text, args );
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
	// set its name
	strncpy( (char *)&kernel_process.name, KERNEL_FILENAME, strlen(KERNEL_FILENAME) );
	// set its default id
	kernel_process.id = KERNEL_PID;
	// set its privilege to KERNEL as this is the kernel
	kernel_process.privilege = KERNEL;
	// setup interrupts
	if( interrupt_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the interrupt layer." );
	// setup our memory manager
	if( mm_init( m ) == FAIL )
		kernel_panic( NULL, "Failed to initilize the Memory Manager subsystem." );
	// setup the virtual file system
	if( vfs_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the Virtual File System subsystem." );
	// setup the io subsystem
	if( io_init() == FAIL )
		kernel_panic( NULL, "Failed to initilize the IO subsystem." );
	// open the kernels console
	kernel_process.handles[PROCESS_CONSOLEHANDLE] = vfs_open( "/amos/device/console0", VFS_MODE_READWRITE );
	if( kernel_process.handles[PROCESS_CONSOLEHANDLE] == NULL )
		kernel_panic( NULL, "Failed to open the kernel console." );		
	// init the PCI layer
	//if( pci_init() == FAIL )
	//	return FAIL;			
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
	if( vfs_mount( "/amos/device/floppy1", "/", FAT_TYPE ) == FAIL )
		kernel_panic( NULL, "Kernel failed to mount primary file system." );

	// spawn the user init process
	process_spawn( &kernel_process, "/amos/init.bin", "/amos/device/console0" );
	
	// enter an idle state, the kernel is now our idle process if theirs nothing to do
	kernel_idle();
	
	// we should never reach here
	kernel_panic( NULL, "Kernel trying to exit." );
}

