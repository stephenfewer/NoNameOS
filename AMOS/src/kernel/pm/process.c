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
#include <kernel/fs/vfs.h>
#include <kernel/pm/elf.h>
#include <kernel/pm/scheduler.h>
#include <lib/string.h>

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

int process_destroy( struct PROCESS_INFO * process )
{
	int i;
	// should we kill a processes children?
	
	kernel_printf("destroying process %d... ", process->id );
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
	mm_free( process );
	
	//process_total--;

	return SUCCESS;	
}

struct PROCESS_INFO * process_create( struct PROCESS_INFO * parent, void * entrypoint, int size )
{
	struct PROCESS_INFO * process;
	void * physicalAddress;
	int i;
	// create a new process info structure
	process = (struct PROCESS_INFO *)mm_malloc( sizeof( struct PROCESS_INFO ) );
	if( process == NULL )
		return NULL;
	// assign a process id
	process->id = ++process_total;
	// set the process id of the parent
	process->parent_id = parent->id;
	// set its privilege to USER as it is a user process
	process->privilege = USER;
	// clear the handles table
	for( i=0 ; i<PROCESS_MAXHANDLES ; i++ )
		process->handles[i] = NULL;
	// setup the initial user heap, mm_morecore() will take care of the rest
	process->heap.heap_base   = PROCESS_USER_HEAP_ADDRESS;
	process->heap.heap_bottom = NULL;
	process->heap.heap_top    = NULL;
	// give it an initial tick slice
	process->tick_slice = PROCESS_TICKS_NORMAL;
	// set the initial process state
	process->state = READY;
	// create the new process's page directory
	paging_createDirectory( process );
	// map in the kernel including its heap and bottom 4 megs
	paging_mapKernel( process );
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
		void * tempAddress = (void *)((DWORD)0xF0000000+(i*PAGE_SIZE));
		
		void * linearAddress = (void *)((DWORD)PROCESS_USER_CODE_ADDRESS+(i*PAGE_SIZE));
		
		physicalAddress = physical_pageAlloc();
		if( physicalAddress == NULL )
			kernel_panic( NULL, "No physical memory for process creation." );

		paging_setPageTableEntry( parent, tempAddress, physicalAddress, TRUE );
		
		memcpy( tempAddress, entrypoint+(i*PAGE_SIZE), PAGE_SIZE );
		
		paging_setPageTableEntry( parent, tempAddress, NULL, FALSE );
		
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
	// return with new process info
	return process;
}

int process_spawn( struct PROCESS_INFO * parent, char * filename, char * console_path )
{
	struct VFS_HANDLE   * console;
	struct PROCESS_INFO * process;
	struct VFS_HANDLE   * handle;
	BYTE * buffer;
	int size;
	int test = FALSE;

	if( strcmp( filename, "/fat/BOOT/TEST.BIN" ) == 0 )
	{
		kernel_printf("spawning TEST.BIN: filename=%s console_path=%s\n",filename,console_path);	
		test = TRUE;
	}
	
	console = vfs_open( console_path, VFS_MODE_READWRITE );
	if( console == NULL )
		return FAIL;
	// open the process image
	handle = vfs_open( filename, VFS_MODE_READ );
	if( handle == NULL )
	{
		vfs_close( console );
		return FAIL;
	}
		
	// determine what type: elf/coff/flat/...

	size = vfs_seek( handle, 0, VFS_SEEK_END );

	// alloc a page extra for safety, will fix this up later
	buffer = (BYTE *)mm_malloc( size + PAGE_SIZE );
	
	vfs_seek( handle, 0, VFS_SEEK_START );
	if( vfs_read( handle, buffer, size ) == FAIL )
	{
		vfs_close( console );
		vfs_close( handle );
		mm_free( buffer );
		return FAIL;
	}
	//if( (i=elf_load( handle )) < 0 )
	//	kernel_printf("Failed to load ELF [%d]: %s\n", i, filename );

	process = process_create( parent, (void *)buffer, size );
	if( process == NULL )
	{
		vfs_close( console );
		vfs_close( handle );
		mm_free( buffer );
		return FAIL;
	}
	
	// close the process images handle
	vfs_close( handle );

	mm_free( buffer );	
	
	process->handles[PROCESS_CONSOLEHANDLE] = console;
	
	// add the process to the scheduler
	scheduler_addProcess( process );

	// return success
	return SUCCESS;
}

int process_yield( void )
{
	// TO-DO: force the current process into a BLOCKED state
	// for now we just force a new process to run
	//struct PROCESS_INFO * process;
	// retrieve the current process
	//ASM( "movl %%dr0, %%eax" :"=r" ( process ): );
	scheduler_setProcess( PROCESS_CURRENT, READY, PROCESS_TICKS_NONE );
	// force a context switch
	ASM("int $32" );
	// we return here next time the process runs
	return SUCCESS;
}

int process_sleep( struct PROCESS_INFO * process )
{
	if( process == NULL )
		return FAIL;
	// place the process into a blocked state
	scheduler_setProcess( process->id, BLOCKED, PROCESS_TICKS_NONE );
	// we return here when the process wakes
	return SUCCESS;
}

int process_wake( int id )
{
	// force the process into a READY state
	scheduler_setProcess( id, READY, PROCESS_TICKS_NORMAL );
	return SUCCESS;
}

/*
int process_wait( int id )
{
	//wait for the process of id to terminate...
	return SUCCESS;
}*/

int process_kill( int id )
{
	// cant kill the kernel ...we'd get court marshelled! :)
	if( id == KERNEL_PID )
		return FAIL;
	kernel_printf( "process_kill( %d )\n", id );
	return scheduler_setProcess( id, TERMINATED, PROCESS_TICKS_NONE );
}
