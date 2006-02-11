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
int process_kill( int id )
{
	struct PROCESS_INFO * process;
	// cant kill the kernel ...we'd get court marshelled! :)
	if( id == 0 )
		return -1;
	// remove the process from the scheduler so it cant be switched back in
	process = scheduler_removeProcesss( id );
	if( process == NULL )
		return -1;
	// destroy the process's page directory, inturn destroying the stack
	paging_destroyDirectory( process->page_dir );
	
	// destroy he user & kernel stacks, user heap, ...
	physical_pageFree( process->user_stack );
	
	// destroy the process info structure
	mm_free( process );
	
	process_total--;
	
	return 0;
}

extern struct PAGE_DIRECTORY * paging_kernelPageDir;

BYTE kstacks[8][PROCESS_STACKSIZE];

struct PROCESS_INFO * process_create( void (*entrypoint)() )
{
	struct PROCESS_STACK * kernel_stack;
	struct PROCESS_INFO * process;
	void * physicalAddress;
	// create a new process info structure
	process = mm_malloc( sizeof( struct PROCESS_INFO ) );
	if( process == NULL )
		return NULL;
	// assign a process id
	process->id = ++process_total;
	// give it an initial tick slice
	process->tick_slice = PROCESS_TICKS_NORMAL;
	// set the initial process state
	process->state = READY;
	// create the new process's page directory
	process->page_dir = paging_createDirectory();
	// map in the kernel including its heap and bottom 4 megs
	paging_mapKernel( process->page_dir );
	// allocate a page for the process's user stack
	physicalAddress = physical_pageAlloc();
	//  map in the stack to the process's address space
	paging_setPageTableEntry( process->page_dir, PROCESS_USER_STACKADDRESS, physicalAddress, TRUE );
	// save the physical user stack address
	process->user_stack = physicalAddress;
	// create the process's initial kernel stack so we can perform a context switch
	process->kernel_stack = (struct PROCESS_STACK *)kstacks[ process->id ];
	// advance the pointer to the top of the stack, less the size of the stack structure, so we can begin filling it in
	kernel_stack = ( process->kernel_stack + PROCESS_STACKSIZE - sizeof(struct PROCESS_STACK) );
	// clear the stack structure
	memset( (void *)kernel_stack, 0x00, sizeof(struct PROCESS_STACK) );
	// set the code segment
	kernel_stack->cs = KERNEL_CODE_SEL;
	// set the data segments
	kernel_stack->ds = KERNEL_DATA_SEL;
	kernel_stack->es = KERNEL_DATA_SEL;
	kernel_stack->fs = KERNEL_DATA_SEL;
	kernel_stack->gs = KERNEL_DATA_SEL;
	// set the eflags register
	kernel_stack->eflags = 0x0202;
	// set our initial entrypoint
	kernel_stack->eip = (DWORD)entrypoint;
	// set the interrupt number we will return from
	kernel_stack->intnumber = IRQ0;
	
	kernel_stack->ss0 = KERNEL_DATA_SEL;
	kernel_stack->esp0 = (DWORD)kernel_stack;
	
	// set the processes initial esp to the top of its user stack, will get poped off its kernel stack
	kernel_stack->esp = (DWORD)(PROCESS_USER_STACKADDRESS + PROCESS_STACKSIZE);
	
	// set the processes current esp to the top of its kernel stack
	process->current_esp = (DWORD)kernel_stack;
	/*
	kprintf( "creating process -\n" );
	kprintf( "                 - process->current_esp:%x\n", process->current_esp );
	kprintf( "                 - process->page_dir:%x\n", process->page_dir );
	kprintf( "                 - CS:%x EIP:%x\n", kernel_stack->cs, kernel_stack->eip );
	kprintf( "                 - DS:%x ES:%x FS:%x GS:%x\n", kernel_stack->ds, kernel_stack->es, kernel_stack->fs, kernel_stack->gs );
	kprintf( "                 - EDI:%x ESI:%x EBP:%x ESP:%x\n", kernel_stack->edi, kernel_stack->esi, kernel_stack->ebp, kernel_stack->esp );
	kprintf( "                 - EBX:%x EDX:%x ECX:%x EAX:%x\n", kernel_stack->ebx, kernel_stack->edx, kernel_stack->ecx, kernel_stack->eax );
	//kprintf( "                 - EFLAGS:%x  SS0:%x ESP0:%x\n", kstack->eflags, kstack->ss0, kstack->esp0 );		
	kprintf( "\n" );
*/
	// return with new process info
	return process;
}
