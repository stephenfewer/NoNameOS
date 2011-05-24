/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
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
#include <kernel/pm/scheduler.h>
#include <lib/libc/string.h>

static int process_uniqueid = 0;

extern struct PROCESS_INFO kernel_process;

inline void process_printStack( struct PROCESS_STACK * stack )
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
	// close any open handles
	for( i=0 ; i<PROCESS_MAXHANDLES ; i++ )
	{
		if( process->handles[i] != NULL )
			vfs_close( process->handles[i] );
	}
	// destroy the process's page directory, inturn destroying and freeing
	// any user memory like the user code, user stack and user heap
	paging_destroyDirectory( process );
	// free the kernel stack
	mm_kfree( process->kstack_base );
	// free the process info structure
	mm_kfree( process );
	return SUCCESS;	
}

struct PROCESS_INFO * process_create( struct PROCESS_INFO * parent, void * entrypoint, int size )
{
	struct PROCESS_INFO * process;
	void * physicalAddress;
	int i;
	// create a new process info structure
	process = (struct PROCESS_INFO *)mm_kmalloc( sizeof( struct PROCESS_INFO ) );
	if( process == NULL )
		return NULL;
	// no link in the chain yet
	process->prev = NULL;
	// assign a unique process id
	process->id = ++process_uniqueid;
	// set the process id of the parent
	process->parent_id = parent->id;
	// set its privilege to USER as it is a user process
	process->privilege = USER;
	// clear the handles table
	for( i=0 ; i<PROCESS_MAXHANDLES ; i++ )
		process->handles[i] = NULL;
	// setup the initial user heap, mm_morecore() will take care of the rest
	process->heap.heap_base   = PROCESS_USER_HEAP_VADDRESS;
	process->heap.heap_top    = NULL;
	// give it an initial tick slice
	process->tick_slice = PROCESS_TICKS_NORMAL;
	// set the initial process state
	process->state = CREATED;
	// create the new process's page directory
	paging_createDirectory( process );
	// map in the kernel including its heap and bottom 4 megs
	paging_mapKernel( process );
	// create the user stack
	for( i=0 ; i<(PROCESS_STACKSIZE/PAGE_SIZE) ; i++ )
	{
		// allocate a page for the process's user stack
		physicalAddress = physical_pageAlloc();
		//  map in the stack to the process's address space
		paging_map( process, PROCESS_USER_STACK_VADDRESS+(i*PAGE_SIZE), physicalAddress, TRUE );
	}
	// copy the code segment from kernel space to user space
	for( i=0 ; i<(size/PAGE_SIZE)+1 ; i++ )
	{
		// get the next linear address for the code
		void * linearAddress = (void *)((DWORD)PROCESS_USER_CODE_VADDRESS+(i*PAGE_SIZE));
		// alloc a physical page for the new process's code
		physicalAddress = physical_pageAlloc();
		// copy the next page of code into the physical page allocated for it
		mm_pmemcpyto( physicalAddress, entrypoint+(i*PAGE_SIZE), PAGE_SIZE );
		// map the code page into the new process's address space
		paging_map( process, linearAddress, physicalAddress, TRUE );
	}
	// create the process's initial kernel stack so we can perform a context switch
	process->kstack_base = mm_kmalloc( PROCESS_STACKSIZE );
	if( process->kstack_base == NULL )
		kernel_panic( NULL, "No physical memory for process creation." );
	// advance the pointer to the top of the stack, less the size of the stack structure, so we can begin filling it in
	process->kstack = (struct PROCESS_STACK *)( process->kstack_base + PROCESS_STACKSIZE - sizeof(struct PROCESS_STACK) );
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
	process->kstack->eip = (DWORD)PROCESS_USER_CODE_VADDRESS;
	// set the code segment
	process->kstack->cs = USER_CODE_SEL | RING3;
	// set the eflags register with the IF bit set
	process->kstack->eflags = 0x0202;	
	// set up the initial user ss and esp for the iret to ring3
	process->kstack->esp0 = (DWORD)(PROCESS_USER_STACK_VADDRESS+PROCESS_STACKSIZE);
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
	char * name;
	int size;
	// if we dont specify a console we clone the parent process's console handle
	if( console_path == NULL )
		console = vfs_clone( parent->handles[PROCESS_CONSOLEHANDLE] );
	else
		console = vfs_open( console_path, VFS_MODE_READWRITE );
	// test for failure
	if( console == NULL )
		return FAIL;
	// open the process image
	handle = vfs_open( filename, VFS_MODE_READ );
	if( handle == NULL )
	{
		vfs_close( console );
		return FAIL;
	}
	// TO-DO: determine what type: elf/coff/flat/...

	// get the file size
	size = vfs_seek( handle, 0, VFS_SEEK_END );
	// alloc a page extra for safety, will fix this up later
	buffer = (BYTE *)mm_kmalloc( size + PAGE_SIZE );
	// seek back to the start and read in all the images contents
	vfs_seek( handle, 0, VFS_SEEK_START );
	if( vfs_read( handle, buffer, size ) == FAIL )
	{
		vfs_close( console );
		vfs_close( handle );
		mm_kfree( buffer );
		return FAIL;
	}
	// create a process from this binary image
	process = process_create( parent, (void *)buffer, size );
	if( process == NULL )
	{
		vfs_close( console );
		vfs_close( handle );
		mm_kfree( buffer );
		return FAIL;
	}
	// close the process images handle
	vfs_close( handle );
	// free the images data buffer
	mm_kfree( buffer );
	// set the process's file name
	name = strrchr( filename, '/' );
	if( name != NULL )
	{
		int len = strlen( ++name );
		if( len >= VFS_NAMESIZE )
			len = VFS_NAMESIZE-1;
		strncpy( (char *)&process->name, name, len );
	}
	// set the process's console handle
	process->handles[PROCESS_CONSOLEHANDLE] = console;
	// add the process to the scheduler
	scheduler_addProcess( process );
	// return success
	return process->id;
}

void process_yield( void )
{
	// set the current process as ready 
	// (but maintain its curren tick slice) - actually just set it to low
	if( scheduler_setProcess( PROCESS_CURRENT, READY, PROCESS_TICKS_LOW ) == SUCCESS )
	{
		// force a context switch
		scheduler_switch();
	}
}

int process_sleep( struct PROCESS_INFO * process )
{
	if( process == NULL )
		return FAIL;
	// place the process into a blocked state
	if( scheduler_setProcess( process->id, BLOCKED, PROCESS_TICKS_CURRENT ) == SUCCESS )
	{
		// force a context switch
		scheduler_switch();
		// we return here when the process wakes
		return SUCCESS;
	}
	return FAIL;
}

int process_wake( int id )
{
	// force the process into a READY state
	return scheduler_setProcess( id, READY, PROCESS_TICKS_NORMAL );
}

int process_wait( int id )
{
	if( id <= 0 )
		return FAIL;
	// wait for the process of id to terminate...
	while( TRUE )
	{
		// if this returns NULL the process of id is no longer in the scheduler
		if( scheduler_getProcess( id ) == NULL )
			break;
		// yield if we get here
		process_yield();
	}
	return SUCCESS;
}

int process_kill( int id )
{
	// cant kill the kernel ...we'd get court marshelled! :)
	if( id == KERNEL_PID )
		return FAIL;
	// mark the process as TERMINATED, scheduler_select() will destroy it later
	return scheduler_setProcess( id, TERMINATED, PROCESS_TICKS_NONE );
}
