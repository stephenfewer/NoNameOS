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

//#include <kernel/tasking/task.h>
#include <kernel/pm/scheduler.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>
#include <kernel/fs/vfs.h>
#include <kernel/pm/elf.h>

int process_total = 0;

int process_spawn( char * filename, struct VFS_HANDLE * console )
{
	int i;
	struct VFS_HANDLE * handle;
	// open the process image
	handle = vfs_open( filename, VFS_MODE_READ );
	if( handle == NULL )
		return -1;
		
	// determine what type: elf/coff/flat/...
	
	BYTE * buffer;
	int size;
	
	size = vfs_seek( handle, 0, VFS_SEEK_END );
	kprintf("size = %d\n", size );
	
	buffer = (BYTE *)mm_malloc( size );
	
	vfs_seek( handle, 0, VFS_SEEK_START );
	if( vfs_read( handle, buffer, size ) == VFS_FAIL )
		return -1;
		
	kprintf("read in buffer\n" );
	
	//if( (i=elf_load( handle )) < 0 )
	//	kprintf("Failed to load ELF [%d]: %s\n", i, filename );
	
	process_create( (void *)buffer );
	
	// close the process images handle
	vfs_close( handle );
	
	// return success
	return 0;
}

/*
extern DWORD scheduler_ticks;

void process_sleep( int ticks )
{
    unsigned long tickend;

    tickend = scheduler_ticks + ticks;
    
    while( scheduler_ticks < tickend );
}
*/
void process_destroy( struct PROCESS_INFO * task )
{
	kernel_lock();
	// remove the task from the scheduler so it cant be switched back in
	scheduler_removeProcesss( task );
	// destroy the tasks page directory, inturn destroying the stack
	paging_destroyDirectory( task->page_dir );
	// destroy the task info structure
	mm_free( task );
	
	process_total--;
	kernel_unlock();
}

extern struct PAGE_DIRECTORY * paging_kernelPageDir;

struct PROCESS_INFO * process_create( void (*entrypoint)() )
{
	struct PROCESS_STACK * stack;
	struct PROCESS_INFO * process;
	void * physicalAddress;
	
	kernel_lock();
	// create a new process info structure
	process = mm_malloc( sizeof( struct PROCESS_INFO ) );
	// assign a process id
	process->id = process_total++;
	// give it an initial tick slice
	process->tick_slice = 1;
	// set the initial process state
	process->state = READY;
	// create the new process's page directory
	process->page_dir = paging_createDirectory();
	// map in the kernel including its heap and bottom 4 megs
	paging_mapKernel( process->page_dir );
	// allocate a page for the stack
	physicalAddress = physical_pageAlloc();
	//  map in the stack to the kernels address space so we can write to it
	paging_setPageTableEntry( paging_kernelPageDir, PROCESS_STACKADDRESS, physicalAddress, TRUE );
	//  map in the stack to the process's address space
	paging_setPageTableEntry( process->page_dir, PROCESS_STACKADDRESS, physicalAddress, TRUE );
	// save the physical stack address
	process->stack = physicalAddress;
	// create the initial stack fo we can perform a context switch
	stack = (struct PROCESS_STACK *)( PROCESS_STACKADDRESS + PROCESS_STACKSIZE - sizeof(struct PROCESS_STACK) );
	// clear the stack
	memset( (void *)stack, 0x00, sizeof(struct PROCESS_STACK) );
	// set the code segment
	stack->cs = 0x08;
	// set the data segments
	stack->ds = 0x10;
	stack->es = 0x10;
	stack->fs = 0x10;
	stack->gs = 0x10;
	// set the eflags register
	stack->eflags = 0x0202;
	// set our initial entrypoint
	stack->eip = (DWORD)entrypoint;
	// set the interrupt number we will return from
	stack->intnumber = IRQ0;
	// set the tasks current esp to the stack
	process->current_esp = (DWORD)stack;
	// unmap the stack from the kernels address space
	paging_setPageTableEntry( paging_kernelPageDir, PROCESS_STACKADDRESS, NULL, FALSE );
	// add the process to the scheduler
	scheduler_addProcess( process );
	
	kernel_unlock();
	// return with new process info
	return process;
}
