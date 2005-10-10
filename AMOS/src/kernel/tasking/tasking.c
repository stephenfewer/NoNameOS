#include <kernel/tasking/tasking.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/mm/mm.h>
#include <kernel/mm/paging.h>
#include <kernel/gdt.h>

DWORD tasking_ticks = 0;

struct task * tasking_currentTask;

struct task * tasking_queue[MAX_TASKS];

void tasking_schedule( struct REGISTERS * reg )
{
	kernel_lock();
	
	tasking_ticks++;
	kprintf("Timer: ticks = %d\n", tasking_ticks );
	
	// if the current task has reached the end of its tick slice
	// we must switch to a new task
	if( tasking_currentTask->tick_slice <= 0 )
	{
		// get the next task to switch to in a round robin fashine
		if( tasking_queue[ tasking_currentTask->id + 1 ] != NULL )
			tasking_currentTask = tasking_queue[ tasking_currentTask->id + 1 ];
		else
			tasking_currentTask = tasking_queue[ 0 ];
		
		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		tasking_currentTask->tick_slice = 15;
		
		paging_setCurrentPageDir( tasking_currentTask->page_dir );
	}	
	else
	{
		tasking_currentTask->tick_slice--;
	}
	
	kernel_unlock();
}

void tasking_init()
{
	int i, interval;
	struct tss * new_tss;
	
	// create the empty task queue
	for( i=0 ; i<MAX_TASKS ; i++ )
		tasking_queue[i] = NULL;
	
	// create the initial task
	tasking_currentTask = mm_malloc( sizeof( struct task ) );
	
	tasking_currentTask->id = 0;
	tasking_currentTask->tick_slice = 15;
	tasking_currentTask->page_dir = paging_getCurrentPageDir();
	
	tasking_queue[ tasking_currentTask->id ] = tasking_currentTask;
	
	// create a TSS for our software task switching (6.2)
	new_tss = mm_malloc( sizeof( struct tss ) );
	new_tss->cr3 = paging_getCurrentPageDir();
	new_tss->esp = kernel_getESP();
	
	// setup the TSS Descriptor (6.2.2)
	gdt_setEntry( 3, (DWORD)new_tss, sizeof(struct tss)-1, 0x89, 0x00 );
	ASM( "ltr (%0)" :: "a" ( 24 ) );

	// calculate the timer interval
	interval = 1193180 / 100000; //100Hz
	// square wave mode
	outportb( PIT_COMMAND_REG, 0x36);
	// set the low interval for timer 0 (mapped to IRQ0)
	outportb( PIT_TIMER_0, interval & 0xFF);
	// set the high interval for timer 0
	outportb( PIT_TIMER_0, interval >> 8);
	// set the scheduler to run via the timer interrupt
	isr_setHandler( IRQ0, tasking_schedule );
}
