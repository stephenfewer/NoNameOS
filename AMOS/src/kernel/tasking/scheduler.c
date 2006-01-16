/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    License: GNU General Public License (GPL)
 */

#include <kernel/tasking/scheduler.h>
#include <kernel/tasking/task.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/kprintf.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/lib/string.h>

DWORD current_esp = 0x00000000;
DWORD current_cr3 = 0x00000000;

DWORD scheduler_ticks = 0;

struct TASK_INFO * scheduler_currentTask = NULL;

// this really really should be a dynamic linked list
struct TASK_INFO * scheduler_queue[MAX_TASKS];

struct TSS * scheduler_tss = NULL;

void scheduler_addTask( struct TASK_INFO * task )
{
	scheduler_queue[ task->id ] = task;	
}

void scheduler_removeTask( struct TASK_INFO * task )
{
	scheduler_queue[ task->id ] = NULL;	
}

extern struct PAGE_DIRECTORY * paging_kernelPageDir;

DWORD scheduler_switch( struct TASK_STACK * taskstack )
{
	// go into the kernels address space
	//paging_setCurrentPageDir( paging_kernelPageDir );
	
	scheduler_ticks++;
/*
if(scheduler_ticks>128)
{
	while(TRUE);
}
*/
	if( scheduler_currentTask == NULL )
	{
		// To-Do: create a kernel task 0 with current_esp
		scheduler_currentTask = scheduler_queue[ 0 ];
		// if we dont have any tasks yet we perform no task switch
		if( scheduler_currentTask == NULL )
			return (DWORD)NULL;
		// set the current_esp to our new stack
		current_esp = scheduler_currentTask->current_esp;
		// set the task state to running as we are switching into this task
		scheduler_currentTask->state = RUNNING;
	}
	
	scheduler_currentTask->current_esp = current_esp;
	
	// if the current task has reached the end of its tick slice
	// we must switch to a new task
	if( scheduler_currentTask->tick_slice <= 0 )
	{/*
		int i=scheduler_currentTask->id + 1;
		// get the next task to switch to in a round robin fashine
		while( TRUE )
		{
			if( scheduler_queue[ i ] == NULL || i>=MAX_TASKS )
				i=0;
	
			if( (scheduler_queue[ i ])->state == READY )
			{
				// set the current task's state to ready
				scheduler_currentTask->state = READY;
				// select the new task we will switch into
				scheduler_currentTask = scheduler_queue[ i ];
				break;
			}
			i++;
		}*/
		
		if( scheduler_queue[ scheduler_currentTask->id + 1 ] != NULL )
			scheduler_currentTask = scheduler_queue[ scheduler_currentTask->id + 1 ];
		else
			scheduler_currentTask = scheduler_queue[ 0 ];

		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		scheduler_currentTask->tick_slice = 1;
		
		// set the task state to running as we are switching into this task
		scheduler_currentTask->state = RUNNING;
	}	
	else
	{
		scheduler_currentTask->tick_slice--;
	}
		
	//kprintf("Timer: [%d] ticks = %d  current_esp = %x\n", scheduler_currentTask->id, scheduler_ticks, current_esp );
	
	// set the current page directory
	current_cr3 = (DWORD)scheduler_currentTask->page_dir;
	
	// fixup the tss
	//scheduler_tss->cr3 = scheduler_currentTask->page_dir;
	//scheduler_tss->esp0 = scheduler_currentTask->current_esp;
		
	return scheduler_currentTask->current_esp;
}

void scheduler_ltr( WORD selector )
{
	ASM( "ltr %0" : : "rm" (selector) );
}
   
DWORD getESP()
{
	DWORD esp;
	ASM( "movl %%esp, %0" : "=r" ( esp ) );
	return esp;
}

void scheduler_enable()
{
	int interval;
	// calculate the timer interval
	interval = 1193180 / 100000; //100Hz
	// square wave mode
	outportb( PIT_COMMAND_REG, 0x36);
	// set the low interval for timer 0 (mapped to IRQ0)
	outportb( PIT_TIMER_0, interval & 0xFF);
	// set the high interval for timer 0
	outportb( PIT_TIMER_0, interval >> 8);
	// set the scheduler to run via the timer interrupt
	isr_setHandler( IRQ0, scheduler_switch );	
}

void scheduler_disable()
{
	isr_setHandler( IRQ0, NULL );	
}

void scheduler_init()
{
	int i;
	
	// create the empty task queue
	for( i=0 ; i<MAX_TASKS ; i++ )
		scheduler_queue[i] = NULL;

	// create a TSS for our software task switching (6.2)
	//scheduler_tss = (struct TSS *)mm_malloc( sizeof( struct TSS ) );
	//memset( (void*)scheduler_tss, 0x00, sizeof( struct TSS ) );
	//scheduler_tss->cr3 = paging_getCurrentPageDir();
	//scheduler_tss->esp0 = getESP();
	
	// setup the TSS Descriptor (6.2.2)
	//gdt_setEntry( KERNEL_TSS_SEL, (DWORD)scheduler_tss, sizeof(struct TSS)-1, 0x89, 0x00 );
	//scheduler_ltr( KERNEL_TSS_SEL );


}
