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
#include <kernel/pm/sync/mutex.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/mm.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <lib/string.h>

extern struct PROCESS_INFO kernel_process;

DWORD scheduler_ticks = 0;

struct SEGMENTATION_TSS * scheduler_tss;

struct SCHEDULER_PROCESS_TABLE scheduler_processTable;

struct MUTEX * scheduler_handlerLock;
struct MUTEX * scheduler_processTableLock;

struct PROCESS_INFO * scheduler_findProcesss( int id )
{
	struct PROCESS_INFO * process;
	for( process=scheduler_processTable.bottom ; process!=NULL ; process=process->next )
	{
		if( process->id == id )
			break;
	}
	return process;
}

int scheduler_setProcess( int id, int state, int ticks )
{
	struct PROCESS_INFO * process;
	
	mutex_lock( scheduler_processTableLock );
	
	if( id == PROCESS_CURRENT )
		process = scheduler_processTable.current;
	else
		process = scheduler_findProcesss( id );
		
	if( process == NULL )
	{
		mutex_unlock( scheduler_processTableLock );
		return FAIL;
	}
	
	process->state = state;
	
	process->tick_slice = ticks;
	
	mutex_unlock( scheduler_processTableLock );
	
	return SUCCESS;
}

void scheduler_printProcessTable( void )
{
	struct PROCESS_INFO * process;
	mutex_lock( scheduler_processTableLock );
	for( process=scheduler_processTable.bottom ; process!=NULL ; process=process->next )
		kernel_printf("\tproc %d %x state: %d (%d)", process->id, process, process->state, process->tick_slice );
	mutex_unlock( scheduler_processTableLock );
}

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * process )
{
	mutex_lock( scheduler_processTableLock );
	
	if( scheduler_processTable.bottom == NULL )
		scheduler_processTable.bottom = process;
	else
		scheduler_processTable.top->next = process;

	scheduler_processTable.top = process;

	scheduler_processTable.top->next = NULL;

	scheduler_processTable.total++;
	
	mutex_unlock( scheduler_processTableLock );
	return process;
}

int scheduler_removeProcesss( struct PROCESS_INFO * process )
{
	struct PROCESS_INFO  * p;

	if( process == NULL )
		return FAIL;
	// we cant remove the kernel
	if( process->id == KERNEL_PID )
		return FAIL;
	// remove the process from the schedulers process table
	if( process == scheduler_processTable.bottom )
	{
		scheduler_processTable.bottom = process->next;
	}
	else
	{
		for( p=scheduler_processTable.bottom ; p!=NULL ; p=p->next )
		{
			if( p->next == process )
			{
				p->next = process->next;
				break;
			}
		}
	}
	// decrement the total process count
	scheduler_processTable.total--;
	return SUCCESS;
}

struct PROCESS_INFO * scheduler_select( struct PROCESS_INFO * processNext )
{
	// lock critical section
	mutex_lock( scheduler_processTableLock );
	// search for another process to run in a round robin fashion 
	while( TRUE )
	{
		// try the next one...
		processNext=processNext->next;
		// if we have come to the end of the queue, we start from the begining
		if( processNext == NULL )
			processNext = scheduler_processTable.bottom;
		
		// if there is a terminated process in the queue, remove and destroy it
		if( processNext->state == TERMINATED )
		{
			//struct PROCESS_INFO * next = processNext->next;
			//if( scheduler_removeProcesss( processNext ) == SUCCESS )
			//{	
				//process_destroy( processNext );
			//	processNext = next;
				continue;
			//}
		}
		// if we find one in a READY state we choose it
		else if( processNext->state == READY )
			break;
	}
	// test if we found another process to run || processNext->id == KERNEL_PID
	if( processNext != scheduler_processTable.current )
	{
		if( processNext->state == TERMINATED )
		{
			kernel_printf("processNext TERMINATED! %d\n", processNext->id );
		}
		// set the current process to a READY state
		scheduler_processTable.current->state = READY;
		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		processNext->tick_slice = PROCESS_TICKS_NORMAL;
		// set the process's state to running as we are switching into this process
		processNext->state = RUNNING;
		// change the current process to the next one we just picked
		scheduler_processTable.current = processNext;
	}
	// unlock our critical section
	mutex_unlock( scheduler_processTableLock );
	// we return the new process (possibly) to indicate we may wish to perform a context switch, see isr.asm
	return processNext;
}

struct PROCESS_INFO * scheduler_handler( struct PROCESS_INFO * process )
{
	struct PROCESS_INFO * newProcess;
	// lock this critical section so we are guaranteed mutual exclusion
	mutex_lock( scheduler_handlerLock );
	// increment our tick counter
	scheduler_ticks++;
	// decrement the current processes time slice by one
	process->tick_slice--;
	// if the current process has reached the end of its tick slice we must select a new process to run
	if( process->tick_slice <= PROCESS_TICKS_NONE )// || process->state != RUNNING
		newProcess = scheduler_select( process );
	else
		newProcess = process;
	// unlock the critical section
	mutex_unlock( scheduler_handlerLock );
	// return TRUE if we are to perform a context switch or FALSE if not
	return newProcess;
}

void scheduler_init()
{
	int interval;
	// initilize the process table
	scheduler_processTable.total = 0;
	scheduler_processTable.top = NULL;
	scheduler_processTable.bottom = NULL;
	// create the lock
	scheduler_handlerLock = mutex_create();
	scheduler_processTableLock = mutex_create();
	// set the initial values we need for the kernels process
	kernel_process.tick_slice = PROCESS_TICKS_LOW;
	// we set the state to ready so when the first context switch occurs it will be for the kernel process
	kernel_process.state = READY;
	// add it to the scheduler and set it as the current process
	scheduler_processTable.current = scheduler_addProcess( &kernel_process );
	// we store the systems current process in the unused DR0 debug register ...so nobody use it!!!
	// with debugging enabled this would specify a breakpoint
	ASM( "movl %%eax, %%dr0" :: "r" ( scheduler_processTable.current ) );
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
	// calculate the timer interval
	interval = 1193180 / 1000;
	// square wave mode
	outportb( INTERRUPT_PIT_COMMAND_REG, 0x36 );
	// set the low interval for timer 0 (mapped to IRQ0)
	outportb( INTERRUPT_PIT_TIMER_0, interval & 0xFF );
	// set the high interval for timer 0
	outportb( INTERRUPT_PIT_TIMER_0, interval >> 8 );
	// enable the scheduler handler
	interrupt_enable( IRQ0, scheduler_handler, SUPERVISOR );
}
