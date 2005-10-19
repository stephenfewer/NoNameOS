//#include <kernel/tasking/task.h>
#include <kernel/tasking/scheduler.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/console.h>


int task_total = 0;

void task_destroy( struct TASK_INFO * task )
{
	scheduler_removeTask( task );
	// if we created a page directory destroy it and all its tables
	//task->page_dir;
	// free the stack
	mm_free( task->stack );
	// destroy the task info structure
	mm_free( task );
	
	task_total--;
}

struct TASK_INFO * task_create( void (*entrypoint)() )
{
	struct TASK_STACK * stack;
	//DWORD * stack;
	struct TASK_INFO * tasking_newTask;
	// create a new task info structure
	tasking_newTask = mm_malloc( sizeof( struct TASK_INFO ) );
	// assign a task id
	tasking_newTask->id = task_total++;
	// give it an initial tick slice
	tasking_newTask->tick_slice = 1;
	// set its page directory
	tasking_newTask->page_dir = paging_getCurrentPageDir();
	// allocate a stack for the task
	tasking_newTask->stack = mm_malloc( 4096 );
	// setup the initial stack fo we can perform a task switch
	stack = (struct TASK_STACK *)( (DWORD)tasking_newTask->stack + 4096 - sizeof(struct TASK_STACK) );
	// clear the stack
	memset( (BYTE *)stack, 0x00, sizeof(struct TASK_STACK) );
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
	tasking_newTask->current_esp = (DWORD)stack;
	// add the task to the scheduler
	scheduler_addTask( tasking_newTask );
	// return with new task info
	return tasking_newTask;
}
