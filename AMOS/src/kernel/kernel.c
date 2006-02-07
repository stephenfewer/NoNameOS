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
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/kprintf.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/io/io.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/fat.h>
#include <kernel/lib/printf.h>

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

void kernel_panic( void )
{
	// attempt to display a message
	kprintf( "kernel panic!\n" );
	// hang the system
	while( TRUE );	
}

void printdir( char * dir )
{
	struct VFS_DIRLIST_ENTRY * entry;
	kprintf( "vfs_list( \"%s\" )\n", dir );
	entry = vfs_list( dir );
	while( entry != NULL  )
	{
		if( entry->name[0] == '\0' )
			break;
		kprintf( "\t%d\t%s\t\t%d\n",  entry->attributes, entry->name, entry->size );
		entry++;
	}
	kprintf( "\n" );
	mm_free( entry );
}

void kernel_main( struct MULTIBOOT_INFO * m )
{
	// initilize the kernel
	kernel_init( m );
	
	// open the kernels console
	kernel_console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( kernel_console == NULL )
		kernel_panic();
	kprintf( "Welcome! - Press keys F1 to F4 to navigate virtual consoles\n\n" );
	
	// mount the root file system
	kprintf( "mounting device /device/floppy1 to /fat/ as a FAT file system.\n" );
	vfs_mount( "/device/floppy1", "/fat/", FAT_TYPE );
	
	//printdir( "/" );
	//printdir( "/device/" );

	if( vfs_create( "/fat/BOOT/STEVE.TXT" ) == VFS_SUCCESS )
	{
		kprintf( "create success\n" );
		
		printdir( "/fat/BOOT/" );
		/*
		if( vfs_rename( "/fat/BOOT/STEVE.TXT", "/fat/BOOT/DELETEME.TXT" ) == VFS_SUCCESS )
		{
			kprintf( "rename success\n" );
			if( vfs_delete( "/fat/BOOT/DELETEME.TXT" ) == VFS_SUCCESS )
				kprintf( "delete success\n" );
			else
				kprintf( "delete fail\n" );
		} 
		else
			kprintf( "rename fail\n" );
		printdir( "/fat/BOOT/" );*/
	}
	else
		kprintf( "create fail\n" );
	
/*
	struct VFS_HANDLE * h;
	h = vfs_open( "/fat/BOOT/TEST.TXT", VFS_MODE_READWRITE );
	if( h == NULL )
	{
		kprintf( "Failed to open test file.\n" );
	} else {
		char buff[100];
		int offset=0;
		int i=0;
		int read=0;
		
		kprintf( "file size = %d\n", vfs_seek( h, 0, VFS_SEEK_END ) );
		
		offset = vfs_seek( h, 200, VFS_SEEK_START );
		kprintf( "offset = %d\n", offset );
		///for(i=0;i<7;i++)
		//{
			if( (read=vfs_read( h, (BYTE *)&buff, 100 )) != VFS_FAIL )
			{
				kprintf( "read=%d %s\n",read, buff );
			} else {
				kprintf( "read fail, i = %d\n", i );
				//break;
			}
		//}
	}
*/
/*
	struct VFS_HANDLE * console;
	console = vfs_open( "/device/console1", VFS_MODE_READWRITE );
	if( console != NULL )
	{
		process_spawn( "/fat/BOOT/TEST.BIN", console );
		
		scheduler_enable();
		
		//kernel_shell( console );
	}
*/	
	while(TRUE);
	
/*	kprintf( "\nMultitasking Test:\n" );
	kprintf( "\tCreating task 1.\n" );
	task_create( task1 );
	kprintf( "\tCreating task 2.\n" );
	task_create( task2 );
	kprintf( "\tEnabling scheduler.\n" );
	scheduler_enable(); */
	
	// after scheduling is enabled we should never reach here
	kernel_panic();
}

