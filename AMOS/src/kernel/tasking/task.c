//#include <kernel/tasking/task.h>
#include <kernel/tasking/scheduler.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/mm.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/kprintf.h>

int task_total = 0;

/*
extern DWORD scheduler_ticks;

void task_sleep( int ticks )
{
    unsigned long tickend;

    tickend = scheduler_ticks + ticks;
    
    while( scheduler_ticks < tickend );
}
*/
void task_destroy( struct TASK_INFO * task )
{
	// remove the task from the scheduler so it cant be switched back in
	scheduler_removeTask( task );
	// destroy the tasks page directory, inturn destroying the stack
	paging_destroyDirectory( task->page_dir );
	// destroy the task info structure
	mm_free( task );
	
	task_total--;
}

extern struct PAGE_DIRECTORY * paging_kernelPageDir;

struct TASK_INFO * task_create( void (*entrypoint)() )
{
	struct TASK_STACK * stack;
	struct TASK_INFO * task;
	void * physicalAddress;
	// create a new task info structure
	task = mm_malloc( sizeof( struct TASK_INFO ) );
	// assign a task id
	task->id = task_total++;
	// give it an initial tick slice
	task->tick_slice = 1;
	// set the initial task state
	task->state = READY;
	// create the new tasks page directory
	task->page_dir = paging_createDirectory();
	// map in the kernel including its heap and bottom 4 megs
	paging_mapKernel( task->page_dir );
	// allocate a page for the stack
	physicalAddress = physical_pageAlloc();
	//  map in the stack to the kernels address space so we can write to it
	paging_setPageTableEntry( paging_kernelPageDir, TASK_STACKADDRESS, physicalAddress, TRUE );
	//  map in the stack to the tasks address space
	paging_setPageTableEntry( task->page_dir, TASK_STACKADDRESS, physicalAddress, TRUE );
	// save the physical stack address
	task->stack = physicalAddress;
	// create the initial stack fo we can perform a task switch
	stack = (struct TASK_STACK *)( TASK_STACKADDRESS + TASK_STACKSIZE - sizeof(struct TASK_STACK) );
	// clear the stack
	mm_memset( (BYTE *)stack, 0x00, sizeof(struct TASK_STACK) );
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
	task->current_esp = (DWORD)stack;
	// unmap the stack from the kernels address space
	paging_setPageTableEntry( paging_kernelPageDir, TASK_STACKADDRESS, NULL, FALSE );
	// add the task to the scheduler
	scheduler_addTask( task );
	// return with new task info
	return task;
}
