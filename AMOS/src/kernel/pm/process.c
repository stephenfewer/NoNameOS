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

#include <kernel/pm/process.h>
#include <kernel/pm/scheduler.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/lib/string.h>
#include <kernel/fs/vfs.h>
#include <kernel/pm/elf.h>
#include <kernel/pm/scheduler.h>

int process_total = 0;

BYTE kstacks[8][PROCESS_STACKSIZE];

extern struct PROCESS_INFO kernel_process;

extern struct PROCESS_INFO * scheduler_processCurrent;

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
	
	// we will need to free this at some point? ...process_destroy()
	buffer = (BYTE *)mm_malloc( size );
	
	vfs_seek( handle, 0, VFS_SEEK_START );
	if( vfs_read( handle, buffer, size ) == FAIL )
		return -1;

	//if( (i=elf_load( handle )) < 0 )
	//	kernel_printf("Failed to load ELF [%d]: %s\n", i, filename );
	
	process = process_create( (void *)buffer, size );
	if( process != NULL )
	{
		process->console = console;
		// add the process to the scheduler
		scheduler_addProcess( process );
	}
	// close the process images handle
	vfs_close( handle );

	// return success
	return 0;
}

int process_yield( void )
{
	// force the current process into a BLOCKED state
	scheduler_processCurrent->state = BLOCKED;
	// select a new process to run
	scheduler_select();
	return 0;
}

int process_wake( int id )
{
	struct PROCESS_INFO * process;
	// find the requested process
	process = scheduler_findProcesss( id );
	if( process == NULL )
	{
		return -1;
	}
	// force the process into a READY state
	process->state = READY;
	return 0;
}

/*
int process_wait( int id )
{
	//wait for the process of id to terminate...
	return 0;
}*/

int process_kill( int id )
{
	struct PROCESS_INFO * process;

	// cant kill the kernel ...we'd get court marshelled! :)
	if( id == KERNEL_PID )
	{
		return -1;
	}
	//scheduler_printProcessTable();
	kernel_printf("Killing process %d... ", id );
	// remove the process from the scheduler so it cant be switched back in
	process = scheduler_removeProcesss( id );
	if( process == NULL )
	{
		return -1;
	}
	// destroy the process's page directory, inturn destroying the stack
	//paging_destroyDirectory( process->page_dir );
	
	// destroy the user & kernel stacks, user heap, ...
	
	// destroy the process info structure
	//mm_free( process );
	
	//process_total--;
	kernel_printf("Done.\n" );
	//scheduler_printProcessTable();

	return 0;
}

struct PROCESS_INFO * process_create( void (*entrypoint)(), int size )
{
	struct PROCESS_STACK * kernel_stack;
	struct PROCESS_INFO * process;
	void * physicalAddress;
	int i;
	// create a new process info structure
	process = mm_malloc( sizeof( struct PROCESS_INFO ) );
	if( process == NULL )
		return NULL;
	// assign a process id
	process->id = ++process_total;
	// set its privilege to USER as it is a user process
	process->privilege = USER;
	// setup the initial user heap, mm_morecore() will take care of the rest
	process->heap.heap_base = PROCESS_USER_HEAP_ADDRESS;
	process->heap.heap_bottom = NULL;
	process->heap.heap_top = NULL;
	// give it an initial tick slice
	process->tick_slice = PROCESS_TICKS_NORMAL;
	// set the initial process state
	process->state = READY;
	// create the new process's page directory
	paging_createDirectory( process );
	// map in the kernel including its heap and bottom 4 megs
	paging_mapKernel( process );
	
	// identity map bottom 4MB's ...THIS IS ONLY FOR TESTING...
	for( physicalAddress=0L ; physicalAddress<(void *)(1024*PAGE_SIZE) ; physicalAddress+=PAGE_SIZE )
		paging_setPageTableEntry( process, physicalAddress, physicalAddress, TRUE );	
	
	// create the user stack
	for( i=0 ; i<(PROCESS_STACKSIZE/PAGE_SIZE) ; i++ )
	{
		// allocate a page for the process's user stack
		physicalAddress = physical_pageAlloc();
		// save the physical user stack address
		if( i == 0 )
			process->user_stack = physicalAddress;
		//  map in the stack to the process's address space
		paging_setPageTableEntry( process, PROCESS_USER_STACK_ADDRESS+(i*PAGE_SIZE), physicalAddress, TRUE );
	}
	// create the process's initial kernel stack so we can perform a context switch
	process->kernel_stack = (struct PROCESS_STACK *)kstacks[ process->id ];
	// advance the pointer to the top of the stack, less the size of the stack structure, so we can begin filling it in
	kernel_stack = ( process->kernel_stack + PROCESS_STACKSIZE - sizeof(struct PROCESS_STACK) );
	// clear the stack structure
	memset( (void *)kernel_stack, 0x00, sizeof(struct PROCESS_STACK) );
	// set the code segment, the privilege is set in the low two bits
	kernel_stack->cs = USER_CODE_SEL | RING3;
	// set the data segments
	kernel_stack->ds = USER_DATA_SEL | RING3;
	kernel_stack->es = USER_DATA_SEL | RING3;
	kernel_stack->fs = USER_DATA_SEL | RING3;
	kernel_stack->gs = USER_DATA_SEL | RING3;
	// set the eflags register with the IF bit set
	kernel_stack->eflags = 0x0202;
	// copy the code segment from kernel space to user space
	for( i=0 ; i<(size/PAGE_SIZE)+1 ; i++ )
	{
		void * linearAddress = (void *)((DWORD)PROCESS_USER_CODE_ADDRESS+(i*PAGE_SIZE));
		
		physicalAddress = physical_pageAlloc();
		
		paging_setPageTableEntry( &kernel_process, linearAddress, physicalAddress, TRUE );
		memcpy( linearAddress, entrypoint+(i*PAGE_SIZE), PAGE_SIZE );
		paging_setPageTableEntry( &kernel_process, linearAddress, NULL, FALSE );
		
		paging_setPageTableEntry( process, linearAddress, physicalAddress, TRUE );
	}
	// set our initial entrypoint
	kernel_stack->eip = (DWORD)PROCESS_USER_CODE_ADDRESS;
	// set the interrupt number we will return from
	kernel_stack->intnumber = IRQ0;
	
	// set up the initial user ss and esp for the iret to ring3
	kernel_stack->ss0 = USER_DATA_SEL | RING3;
	kernel_stack->esp0 = (DWORD)(PROCESS_USER_STACK_ADDRESS+PROCESS_STACKSIZE);

	// should we set kernel_stack->ebp ?
	
	// we could pass in argc, *argv and *env here...
	//kernel_stack->eax;
	//kernel_stack->ebx;
	//kernel_stack->ecx;
	
	// set the processes current esp to the top of its kernel stack
	process->current_esp = process->current_kesp = (DWORD)kernel_stack;

	/*
	kernel_printf( "creating process -\n" );
	kernel_printf( "                 - process->current_esp:%x\n", process->current_esp );
	kernel_printf( "                 - process->page_dir:%x\n", process->page_dir );
	kernel_printf( "                 - CS:%x EIP:%x\n", kernel_stack->cs, kernel_stack->eip );
	kernel_printf( "                 - DS:%x ES:%x FS:%x GS:%x\n", kernel_stack->ds, kernel_stack->es, kernel_stack->fs, kernel_stack->gs );
	kernel_printf( "                 - EDI:%x ESI:%x EBP:%x ESP:%x\n", kernel_stack->edi, kernel_stack->esi, kernel_stack->ebp, kernel_stack->esp );
	kernel_printf( "                 - EBX:%x EDX:%x ECX:%x EAX:%x\n", kernel_stack->ebx, kernel_stack->edx, kernel_stack->ecx, kernel_stack->eax );
	//kernel_printf( "                 - EFLAGS:%x  SS0:%x ESP0:%x\n", kstack->eflags, kstack->ss0, kstack->esp0 );		
	kernel_printf( "\n" );
*/
	// return with new process info
	return process;
}
