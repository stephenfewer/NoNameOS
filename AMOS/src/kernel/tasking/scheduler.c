#include <kernel/tasking/scheduler.h>
#include <kernel/tasking/task.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/mm/paging.h>
#include <kernel/gdt.h>

DWORD current_esp = 0x00000000;

DWORD scheduler_ticks = 0;

struct TASK_INFO * scheduler_currentTask = NULL;

// this really really should be a dynamic linked list
struct TASK_INFO * scheduler_queue[MAX_TASKS];

struct TSS * scheduler_tss = NULL;

void ThreadTest1()
{
	unsigned char* VidMemChar = (unsigned char*)0xB8000;
	*VidMemChar='1';
	for(;;)
	{
		if( *VidMemChar=='1' )
			*VidMemChar='2';
		else
			*VidMemChar='1';
	}
}

void ThreadTest2()
{
	unsigned char* VidMemChar = (unsigned char*)0xB8002;
	*VidMemChar='a';
	for(;;)
	{
		if( *VidMemChar=='a' )
			*VidMemChar='b';
		else
			*VidMemChar='a';
	}
}

void scheduler_addTask( struct TASK_INFO * task )
{
	scheduler_queue[ task->id ] = task;	
}

void scheduler_removeTask( struct TASK_INFO * task )
{
	scheduler_queue[ task->id ] = NULL;	
}

DWORD scheduler_switch( struct TASK_STACK * taskstack )
{
	scheduler_ticks++;

	if( scheduler_currentTask == NULL )
	{
		// To-Do: create a kernel task 0 with current_esp
		scheduler_currentTask = scheduler_queue[ 0 ];
		kprintf("first task switch: current_esp = %x\n", current_esp );
		//scheduler_tss->cr3 = scheduler_currentTask->page_dir;
		//scheduler_tss->esp = scheduler_currentTask->current_esp;
		kernel_unlock();
		return scheduler_currentTask->current_esp;
	}

	kprintf("Timer: [%d] ticks = %d  current_esp = %x\n", scheduler_currentTask->id, scheduler_ticks, current_esp );
	
	scheduler_currentTask->current_esp = current_esp;
	
	// if the current task has reached the end of its tick slice
	// we must switch to a new task
	if( scheduler_currentTask->tick_slice <= 0 )
	{
		// get the next task to switch to in a round robin fashine
		if( scheduler_queue[ scheduler_currentTask->id + 1 ] != NULL )
			scheduler_currentTask = scheduler_queue[ scheduler_currentTask->id + 1 ];
		else
			scheduler_currentTask = scheduler_queue[ 0 ];
		
		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		scheduler_currentTask->tick_slice = 1;
		
		//paging_setCurrentPageDir( scheduler_currentTask->page_dir );
		
		//scheduler_tss->cr3 = scheduler_currentTask->page_dir;
		//scheduler_tss->esp = scheduler_currentTask->current_esp;
	}	
	else
	{
		scheduler_currentTask->tick_slice--;
	}
	
	return scheduler_currentTask->current_esp;
}

void scheduler_ltr( WORD selector )
{
	ASM( "ltr %0" : : "rm" (selector) );
}
   
void scheduler_init()
{
	int i, interval;
	
	// create the empty task queue
	for( i=0 ; i<MAX_TASKS ; i++ )
		scheduler_queue[i] = NULL;

	// create a TSS for our software task switching (6.2)
	//scheduler_tss = mm_malloc( sizeof( struct TSS ) );
	//scheduler_tss->cr3 = paging_getCurrentPageDir();
	
	// setup the TSS Descriptor (6.2.2)
	//gdt_setEntry( KERNEL_TSS_SEL, (DWORD)scheduler_tss, sizeof(struct TSS)-1, 0x89, 0x00 );
	//tasking_ltr( KERNEL_TSS_SEL );

	task_create( ThreadTest1 );
	task_create( ThreadTest2 );
	
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
