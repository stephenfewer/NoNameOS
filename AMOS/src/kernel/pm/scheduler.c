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
#include <kernel/mm/segmentation.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/lib/string.h>

DWORD current_esp = 0x00000000;
DWORD current_cr3 = 0x00000000;

DWORD scheduler_ticks = 0;

struct SEGMENTATION_TSS * scheduler_tss        = NULL;

struct PROCESS_INFO * scheduler_currentProcess = NULL;
struct PROCESS_INFO * scheduler_processTop     = NULL;
struct PROCESS_INFO * scheduler_processBottom  = NULL;

extern struct PROCESS_INFO kernel_process;

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * process )
{
	if( scheduler_processBottom == NULL )
		scheduler_processBottom = process;
	else
		scheduler_processTop->next = process;
	scheduler_processTop = process;
	
	return scheduler_processTop;
}

struct PROCESS_INFO * scheduler_findProcesss( int id )
{
	struct PROCESS_INFO * process;
	for( process=scheduler_processBottom ; process!=NULL ; process=process->next )
	{
		if( process->id == id )
			break;
	}
	return process;
}

struct PROCESS_INFO * scheduler_removeProcesss( int id )
{
	struct PROCESS_INFO * process, * p;
	
	process = scheduler_findProcesss( id );
	if( process == NULL )
		return NULL;
	// remove the mount point from the VFS
	if( process == scheduler_processBottom )
	{
		scheduler_processBottom = process->next;
	}
	else
	{
		for( p=scheduler_processBottom ; p!=NULL ; p=p->next )
		{
			if( p->next == process )
			{
				p->next = process->next;
				break;
			}
		}
	}
	return process;
}



void scheduler_idle( void )
{
	// delay
	inportb( 0x80 );
}

void scheduler_handler( void )
{
	scheduler_ticks++;
	
//	if( scheduler_ticks > 14 )
//		while(TRUE);
	/*	
	if( scheduler_currentProcess == NULL )
	{
		// set the initial values we need for the kernels process
		kernel_process.current_esp = current_esp;
		kernel_process.tick_slice = PROCESS_TICKS_LOW;
		kernel_process.state = READY;
		// add it to the scheduler
		scheduler_addProcess( &kernel_process );
		// select the first process in the queue, gaurenteed to have at least one because we just added the kernel process above
		scheduler_currentProcess = scheduler_processBottom;
		// set the current_esp to our new stack
		current_esp = scheduler_currentProcess->current_esp;
		// set the process state to running as we are switching into this process
		scheduler_currentProcess->state = RUNNING;
	}*/
	
	scheduler_currentProcess->current_esp = current_esp;

//kernel_printf("[%d] id: %d esp: %x\n", scheduler_ticks, scheduler_currentProcess->id, scheduler_currentProcess->current_esp );
	
	// if the current process has reached the end of its tick slice we must switch to a new process
	if( scheduler_currentProcess->tick_slice <= 0 )
	{
		struct PROCESS_INFO * next_process = scheduler_currentProcess->next;
		// search for another process to run in a round robin fashion
		for( ; next_process!=scheduler_currentProcess ; next_process=next_process->next )
		{
			// if we have come to the end of the queue, we start from the begining
			if( next_process == NULL )
				next_process = scheduler_processBottom;
			// if we find one in a READY state we choose it
			if( next_process->state == READY )
				break;
		}
		// test if we found another process to run
		if( next_process != scheduler_currentProcess )
		{
			// set the current process to a READY state
			scheduler_currentProcess->state = READY;
			// chane the current process to the next one we just picked
			scheduler_currentProcess = next_process;
			// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
			scheduler_currentProcess->tick_slice = PROCESS_TICKS_NORMAL;
			// set the process's state to running as we are switching into this process
			scheduler_currentProcess->state = RUNNING;
		}
	}
	// decrement this processes time slice by one
	scheduler_currentProcess->tick_slice--;
	// set the current page directory
	current_cr3 = (DWORD)scheduler_currentProcess->page_dir;
	// set the current kernel stack pointer
	current_esp = scheduler_currentProcess->current_esp;
	// fixup the tss
	//scheduler_tss->ss0 = KERNEL_DATA_SEL;
	//scheduler_tss->cr3 = scheduler_currentProcess->page_dir;
	//scheduler_tss->esp0 = scheduler_currentProcess->current_esp;
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
	// enable the timer interrupt, we dont set a handler as irq00 its hard coded in loader.asm to use scheduler_switch()
	interrupt_enable( IRQ0, NULL );
}

void scheduler_init()
{
	// set the initial values we need for the kernels process
	kernel_process.tick_slice = PROCESS_TICKS_LOW;
	// we set the state to running as when the first context switch occurs it will be for the kernel process
	kernel_process.state = RUNNING;
	// add it to the scheduler and set it as the current process
	scheduler_currentProcess = scheduler_addProcess( &kernel_process );
	// create a TSS for our software task switching (6.2)
	scheduler_tss = (struct SEGMENTATION_TSS *)mm_malloc( sizeof(struct SEGMENTATION_TSS) );
	// clear it
	memset( (void *)scheduler_tss, 0x00, sizeof(struct SEGMENTATION_TSS) );
	// setup the TSS Descriptor (6.2.2)
	segmentation_setEntry( KERNEL_TSS_SEL, (DWORD)scheduler_tss, sizeof(struct SEGMENTATION_TSS)-1, 0x89, 0x00 );
	// reload GDT
	segmentation_reload();
	// load the task register with the TSS selector
	segmentation_ltr( KERNEL_TSS_SEL );
}
