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

#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/kprintf.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/lib/string.h>

DWORD current_esp = 0x00000000;
DWORD current_cr3 = 0x00000000;

DWORD scheduler_ticks = 0;

struct PROCESS_INFO * scheduler_currentProcess = NULL;

// this really really should be a dynamic linked list
struct PROCESS_INFO * scheduler_queue[MAX_TASKS];

struct TSS * scheduler_tss = NULL;

void scheduler_addProcess( struct PROCESS_INFO * process )
{
	scheduler_queue[ process->id ] = process;	
}

void scheduler_removeProcesss( struct PROCESS_INFO * process )
{
	scheduler_queue[ process->id ] = NULL;	
}

extern struct PAGE_DIRECTORY * paging_kernelPageDir;

void scheduler_idle( void )
{
	// delay
	inportb( 0x80 );
}

DWORD scheduler_switch( struct PROCESS_STACK * process_stack )
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
	if( scheduler_currentProcess == NULL )
	{
		// To-Do: create a kernel process 0 with current_esp
		scheduler_currentProcess = scheduler_queue[ 0 ];
		// if we dont have any process yet we perform no task switch
		if( scheduler_currentProcess == NULL )
			return (DWORD)NULL;
		// set the current_esp to our new stack
		current_esp = scheduler_currentProcess->current_esp;
		// set the process state to running as we are switching into this process
		scheduler_currentProcess->state = RUNNING;
	}
	
	scheduler_currentProcess->current_esp = current_esp;
	
	// if the current process has reached the end of its tick slice
	// we must switch to a new process
	if( scheduler_currentProcess->tick_slice <= 0 )
	{/*
		int i=scheduler_currentTask->id + 1;
		// get the next process to switch to in a round robin fashine
		while( TRUE )
		{
			if( scheduler_queue[ i ] == NULL || i>=MAX_TASKS )
				i=0;
	
			if( (scheduler_queue[ i ])->state == READY )
			{
				// set the current process's state to ready
				scheduler_currentProcess->state = READY;
				// select the new process we will switch into
				scheduler_currentProcess = scheduler_queue[ i ];
				break;
			}
			i++;
		}*/
		
		if( scheduler_queue[ scheduler_currentProcess->id + 1 ] != NULL )
			scheduler_currentProcess = scheduler_queue[ scheduler_currentProcess->id + 1 ];
		else
			scheduler_currentProcess = scheduler_queue[ 0 ];

		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		scheduler_currentProcess->tick_slice = 1;
		
		// set the process's state to running as we are switching into this process
		scheduler_currentProcess->state = RUNNING;
	}	
	else
	{
		scheduler_currentProcess->tick_slice--;
	}
		
	//kprintf("Timer: [%d] ticks = %d  current_esp = %x\n", scheduler_currentTask->id, scheduler_ticks, current_esp );
	
	// set the current page directory
	current_cr3 = (DWORD)scheduler_currentProcess->page_dir;
	
	// fixup the tss
	//scheduler_tss->cr3 = scheduler_currentTask->page_dir;
	//scheduler_tss->esp0 = scheduler_currentTask->current_esp;
	
	outportb( INTERRUPT_PIC_2, INTERRUPT_EOI );
	
	return scheduler_currentProcess->current_esp;
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
	outportb( INTERRUPT_PIT_COMMAND_REG, 0x36);
	// set the low interval for timer 0 (mapped to IRQ0)
	outportb( INTERRUPT_PIT_TIMER_0, interval & 0xFF);
	// set the high interval for timer 0
	outportb( INTERRUPT_PIT_TIMER_0, interval >> 8);
	// set the scheduler to run via the timer interrupt
	//interrupt_enable( IRQ0, scheduler_switch );	
}
/*
void scheduler_disable()
{
	interrupt_disable( IRQ0, NULL );	
}
*/
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
