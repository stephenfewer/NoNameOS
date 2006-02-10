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

//#include <kernel/pm/process.h>
#include <kernel/pm/scheduler.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>
#include <kernel/fs/vfs.h>
#include <kernel/pm/elf.h>

int process_total = 0;

int process_spawn( char * filename, struct VFS_HANDLE * console )
{
	struct PROCESS_INFO * process;
	struct VFS_HANDLE * handle;
	BYTE * buffer;
	int size;
		
	// open the process image
	handle = vfs_open( filename, VFS_MODE_READ );
	if( handle == NULL )
		return -1;
		
	// determine what type: elf/coff/flat/...

	size = vfs_seek( handle, 0, VFS_SEEK_END );
	kprintf("size = %d\n", size );
	
	// we will need to free this at some point? ...process_destroy()
	buffer = (BYTE *)mm_malloc( size );
	
	vfs_seek( handle, 0, VFS_SEEK_START );
	if( vfs_read( handle, buffer, size ) == VFS_FAIL )
		return -1;
		
	kprintf("read in buffer\n" );
	
	//if( (i=elf_load( handle )) < 0 )
	//	kprintf("Failed to load ELF [%d]: %s\n", i, filename );
	
	process = process_create( (void *)buffer );
	if( process != NULL )
	{
		process->console = console;
		kprintf( "adding process %d to the scheduler\n", process->id );
		// add the process to the scheduler
		scheduler_addProcess( process );
	}
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
	stack->cs = 0x08;//KERNEL_CODE_SEL;
	// set the data segments
	stack->ds = 0x10;//KERNEL_DATA_SEL;
	stack->es = 0x10;//KERNEL_DATA_SEL;
	stack->fs = 0x10;//KERNEL_DATA_SEL;
	stack->gs = 0x10;//KERNEL_DATA_SEL;
	// set the eflags register
	stack->eflags = 0x0202;
	// set our initial entrypoint
	stack->eip = (DWORD)entrypoint;
	// set the interrupt number we will return from
	stack->intnumber = IRQ0;
	
	//stack->ss = 0x10;
	//stack->userstack = (DWORD)stack;
	//stack->esp = (DWORD)stack;
	
	// set the tasks current esp to the stack
	process->current_esp = (DWORD)stack;
	
	kprintf( "creating process -\n" );
	kprintf( "                 - CS:%x EIP:%x\n", stack->cs, stack->eip );
	kprintf( "                 - DS:%x ES:%x FS:%x GS:%x\n", stack->ds, stack->es, stack->fs, stack->gs );
	kprintf( "                 - EDI:%x ESI:%x EBP:%x ESP:%x\n", stack->edi, stack->esi, stack->ebp, stack->esp );
	kprintf( "                 - EBX:%x EDX:%x ECX:%x EAX:%x\n", stack->ebx, stack->edx, stack->ecx, stack->eax );
	kprintf( "                 - EFLAGS:%x  SS:%x userstack:%x\n", stack->eflags, stack->ss, stack->userstack );		
	kprintf( "\n" );
	
	// unmap the stack from the kernels address space
	paging_setPageTableEntry( paging_kernelPageDir, PROCESS_STACKADDRESS, NULL, FALSE );
	
	kernel_unlock();
	// return with new process info
	return process;
}
