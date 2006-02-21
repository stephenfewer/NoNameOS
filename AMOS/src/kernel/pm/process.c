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

__inline__ void process_printStack( struct PROCESS_STACK * stack )
{
	kernel_printf( "\tCS:%x EIP:%x\n", stack->cs, stack->eip );
	kernel_printf( "\tDS:%x ES:%x FS:%x GS:%x\n", stack->ds, stack->es, stack->fs, stack->gs );
	kernel_printf( "\tEDI:%x ESI:%x EBP:%x ESP:%x\n", stack->edi, stack->esi, stack->ebp, stack->esp );
	kernel_printf( "\tEBX:%x EDX:%x ECX:%x EAX:%x\n", stack->ebx, stack->edx, stack->ecx, stack->eax );
	kernel_printf( "\tEFLAGS:%x  SS0:%x ESP0:%x\n", stack->eflags, stack->ss0, stack->esp0 );
}

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
		process->handles[PROCESS_CONSOLEHANDLE] = console;
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
	scheduler_getCurrentProcess()->state = BLOCKED;
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
	int i;
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
	// close any open handles
	for( i=0 ; i<PROCESS_MAXHANDLES ; i++ )
	{
		if( process->handles[i] != NULL )
			vfs_close( process->handles[i] );
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
	// clear the handles table
	for( i=0 ; i<PROCESS_MAXHANDLES ; i++ )
		process->handles[i] = NULL;
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
		// save the physical user stack base address
		if( i == 0 )
			process->ustack_base = physicalAddress;
		//  map in the stack to the process's address space
		paging_setPageTableEntry( process, PROCESS_USER_STACK_ADDRESS+(i*PAGE_SIZE), physicalAddress, TRUE );
	}
	
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
	
	// create the process's initial kernel stack so we can perform a context switch
	process->kstack_base = (struct PROCESS_STACK *)kstacks[ process->id ];
	// advance the pointer to the top of the stack, less the size of the stack structure, so we can begin filling it in
	process->kstack = ( process->kstack_base + PROCESS_STACKSIZE - sizeof(struct PROCESS_STACK) );
	// clear the kernel stack structure
	memset( (void *)process->kstack, 0x00, sizeof(struct PROCESS_STACK) );
	// set the data segments, the privilege is set in the low two bits
	process->kstack->ds = USER_DATA_SEL | RING3;
	process->kstack->es = USER_DATA_SEL | RING3;
	process->kstack->fs = USER_DATA_SEL | RING3;
	process->kstack->gs = USER_DATA_SEL | RING3;
	// set the interrupt number we will return from
	process->kstack->intnumber = IRQ0;
	// set our initial entrypoint
	process->kstack->eip = (DWORD)PROCESS_USER_CODE_ADDRESS;
	// set the code segment
	process->kstack->cs = USER_CODE_SEL | RING3;
	// set the eflags register with the IF bit set
	process->kstack->eflags = 0x0202;	
	// set up the initial user ss and esp for the iret to ring3
	process->kstack->esp0 = (DWORD)(PROCESS_USER_STACK_ADDRESS+PROCESS_STACKSIZE);
	process->kstack->ss0 = USER_DATA_SEL | RING3;
	// set the processes current kernel esp to the top of its kernel stack
	//process->current_kesp = process->kstack;
	// return with new process info
	return process;
}
