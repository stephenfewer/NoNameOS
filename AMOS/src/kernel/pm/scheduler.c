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

static struct PROCESS_INFO * scheduler_findProcesss( int id )
{
	struct PROCESS_INFO * process;
	// search the process tabel for the process of the specified it
	for( process=scheduler_processTable.bottom ; process!=NULL ; process=process->next )
	{
		if( process->id == id )
			break;
	}
	// return the process or NULL if not found
	return process;
}

int scheduler_setProcess( int id, int state, int ticks )
{
	struct PROCESS_INFO * process;
	int ret=FAIL;
	// acquire tabel lock for critical section
	mutex_lock( &scheduler_processTable.lock );
	// find the specified process
	if( id == PROCESS_CURRENT )
		ASM( "movl %%dr0, %%eax" : "=r" ( process ) : );
	else
		process = scheduler_findProcesss( id );
	// if we successfully found one, set its state and ticks
	if( process != NULL )
	{
		// set the state
		process->state = state;
		// set the ticks if we didnt specify PROCESS_TICKS_CURRENT
		if( ticks >= PROCESS_TICKS_NONE )
			process->tick_slice = ticks;
		ret = SUCCESS;
	}
	// unlock critical section
	mutex_unlock( &scheduler_processTable.lock );
	if( ret==FAIL)
		kernel_printf("FAIL setProcess %d... \n", id, state, ticks );
	
	return ret;
}

void scheduler_printProcessTable( void )
{
	struct PROCESS_INFO * process;
	kernel_printf("scheduler_printProcessTable()\n");
	mutex_lock( &scheduler_processTable.lock );
	for( process=scheduler_processTable.bottom ; process!=NULL ; process=process->next )
		kernel_printf("\tproc %d %x state: %d (%d)", process->id, process, process->state, process->tick_slice );
	mutex_unlock( &scheduler_processTable.lock );
}

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * process )
{
	mutex_lock( &scheduler_processTable.lock );
	// if the process tabel is empty, set it as the bottom
	if( scheduler_processTable.bottom == NULL )
		scheduler_processTable.bottom = process;
	else {
		// otherwise insert it after the bottom
		process->next = scheduler_processTable.bottom->next;
		scheduler_processTable.bottom->next = process;
	}
	mutex_unlock( &scheduler_processTable.lock );
	return process;
}

int scheduler_removeProcesss( struct PROCESS_INFO * process )
{
	struct PROCESS_INFO  * p;
	int ret=FAIL;
	if( process == NULL )
		return FAIL;
	// we cant remove the kernel
	if( process->id == KERNEL_PID )
		return FAIL;
	kernel_printf("remove process %d... \n", process->id );
	// remove the process from the schedulers process table
	if( process == scheduler_processTable.bottom )
	{
		scheduler_processTable.bottom = process->next;
		ret = SUCCESS;
	}
	else
	{
		for( p=scheduler_processTable.bottom ; p!=NULL ; p=p->next )
		{
			if( p->next == process )
			{
				p->next = process->next;
				ret = SUCCESS;
				break;
			}
		}
	}
	return ret;
}

struct PROCESS_INFO * scheduler_select( struct PROCESS_INFO * processNext )
{
	struct PROCESS_INFO * processCurrent;
	// lock critical section
	mutex_lock( &scheduler_processTable.lock );
	processCurrent = processNext;
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
			//kernel_printf("processNext TERMINATED! %d\n", processNext->id );
			struct PROCESS_INFO * next = processNext->next;
			if( scheduler_removeProcesss( processNext ) == SUCCESS )
			{	
				process_destroy( processNext );
				processNext = next;
			//	continue;
			}
		} 
		
		// if we find one in a READY state we choose it
		if( processNext->state == READY )
			break;
	}
	// test if we found another process to run || processNext->id == KERNEL_PID
	if( processNext != processCurrent )
	{
		if( processNext->state == TERMINATED )
		{
			kernel_printf("processNext TERMINATED! %d\n", processNext->id );
		}
		// set the current process to a READY state
		processCurrent->state = READY;
		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		processNext->tick_slice = PROCESS_TICKS_NORMAL;
		// set the process's state to running as we are switching into this process
		processNext->state = RUNNING;
	}
	
	if( processNext->state == TERMINATED )
	{
		kernel_printf("RETURNING processNext as TERMINATED! %d\n", processNext->id );
	}
	
	// unlock our critical section
	mutex_unlock( &scheduler_processTable.lock );
	// we return the new process (possibly) to indicate we may wish to perform a context switch, see isr.asm
	return processNext;
}

struct PROCESS_INFO * scheduler_handler( struct PROCESS_INFO * process )
{
	struct PROCESS_INFO * newProcess;
	// increment our tick counter
	scheduler_ticks++;
	// lock this critical section so we are guaranteed mutual exclusion over the process table
	mutex_lock( &scheduler_processTable.lock );
	// decrement the current processes time slice by one
	process->tick_slice--;
	mutex_unlock( &scheduler_processTable.lock );
	// if the current process has reached the end of its tick slice we must select a new process to run
	if( process->tick_slice <= PROCESS_TICKS_NONE )//|| process->state != RUNNING )
		newProcess = scheduler_select( process );
	else
		newProcess = process;
	// return a possible new process
	return newProcess;
}

int scheduler_init( void )
{
	int interval;
	// initilize the process table
	//scheduler_processTable.top = NULL;
	scheduler_processTable.bottom = NULL;
	// init the lock
	mutex_init( &scheduler_processTable.lock );
	// set the initial values we need for the kernels process
	kernel_process.tick_slice = PROCESS_TICKS_LOW;
	// we set the state to ready so when the first context switch occurs it will be for the kernel process
	kernel_process.state = READY;
	// add it to the scheduler and set it as the current process
	scheduler_addProcess( &kernel_process );
	// we store the systems current process in the unused DR0 debug register ...so nobody use it!!!
	// with debugging enabled this would specify a breakpoint
	ASM( "movl %%eax, %%dr0" :: "r" ( &kernel_process ) );
	// create a TSS for our software task switching (6.2)
	scheduler_tss = (struct SEGMENTATION_TSS *)mm_malloc( sizeof(struct SEGMENTATION_TSS) );
	// clear it
	memset( (void *)scheduler_tss, 0x00, sizeof(struct SEGMENTATION_TSS) );
	// setup the TSS Descriptor (6.2.2)
	segmentation_setEntry( KERNEL_TSS_SEL, (DWORD)scheduler_tss, sizeof(struct SEGMENTATION_TSS)-1, 0x89, 0x00 );
	// load the task register with the TSS selector
	segmentation_ltr( KERNEL_TSS_SEL );
	// calculate the timer interval
	interval = 1193180 / 100;
	// square wave mode
	outportb( INTERRUPT_PIT_COMMAND_REG, 0x36 );
	// set the low interval for timer 0 (mapped to SCHEDULER_INTERRUPT)
	outportb( INTERRUPT_PIT_TIMER_0, interval & 0xFF );
	// set the high interval for timer 0
	outportb( INTERRUPT_PIT_TIMER_0, interval >> 8 );
	// enable the scheduler handler
	interrupt_enable( SCHEDULER_INTERRUPT, scheduler_handler, SUPERVISOR );
	// return with pre emptive scheduling now active
	return SUCCESS;
}
