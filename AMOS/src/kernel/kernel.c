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

int kernel_lockCount = 0;

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
		//	*crash=0xDEADBEEF;
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
/*
inline void kernel_lock()
{
	if( kernel_lockCount <= 0 )
		interrupt_disableAll();
	kernel_lockCount++;
}

inline void kernel_unlock()
{
	if( --kernel_lockCount <= 0 )
		interrupt_enableAll();
}
*/
// initilize the kernel and bring up all the subsystems
void kernel_init( struct MULTIBOOT_INFO * m )
{
	// lock protected code
	interrupt_disableAll();
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
	// open the kernels console
	kernel_process.console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( kernel_process.console == NULL )
		kernel_panic( NULL, "Failed to open the kernel console." );
	// setup our system calls
	syscall_init();
	// setup scheduling
	scheduler_init();
	// unlock protected code
	interrupt_enableAll();
}

void kernel_printf( char * text, ... )
{
	va_list args;
	// we can only kernel_printf() if we have a console to do it on
	if( kernel_process.console != NULL )
	{
		// find the first argument
		va_start( args, text );
		// pass printf the kernels std output handle the format text and the first argument
		printf( kernel_process.console, text, args );
	}
}

// ..."this is the end. beautiful friend, the end."
void kernel_panic( struct PROCESS_STACK * stack, char * message )
{
	interrupt_disableAll();
	// attempt to print out some info
	kernel_printf( "AMOS Kernel Panic! " );
	// print out the message
	if( message != NULL )
		kernel_printf( "%s\n", message );
	// print out the stack contents
	if( stack != NULL )
	{
		kernel_printf( "\tCS:%x EIP:%x\n", stack->cs, stack->eip );
		kernel_printf( "\tDS:%x ES:%x FS:%x GS:%x\n", stack->ds, stack->es, stack->fs, stack->gs );
		kernel_printf( "\tEDI:%x ESI:%x EBP:%x ESP:%x\n", stack->edi, stack->esi, stack->ebp, stack->esp );
		kernel_printf( "\tEBX:%x EDX:%x ECX:%x EAX:%x\n", stack->ebx, stack->edx, stack->ecx, stack->eax );
		kernel_printf( "\tEFLAGS:%x  SS0:%x ESP0:%x\n", stack->eflags, stack->ss0, stack->esp0 );
	}
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
		//scheduler_addProcess( process_create( (void*)&task2, 4096 ) );

		//scheduler_addProcess( process_create( (void*)&task1, 4096 ) );
		
		//process_spawn( "/fat/BOOT/TEST.BIN", console );
	
		scheduler_enable();
		
		kernel_shell( console );
	} else {
		kernel_printf( "failed to open /device/console1\n" );
	}

	//while( TRUE )
	//	process_sleep();

	// after scheduling is enabled we should never reach here
	kernel_panic( NULL, "Kernel trying to exit." );
}

