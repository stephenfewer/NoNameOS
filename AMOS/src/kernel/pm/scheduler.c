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
#include <kernel/lib/string.h>


extern struct PROCESS_INFO kernel_process;

DWORD scheduler_ticks = 0;

volatile DWORD scheduler_switch = FALSE;

struct SEGMENTATION_TSS * scheduler_tss			= NULL;

struct PROCESS_INFO * scheduler_processCurrent	= NULL;
// the linked list that defines the schedulers process table
struct PROCESS_INFO * scheduler_processTop		= NULL;
struct PROCESS_INFO * scheduler_processBottom	= NULL;

struct MUTEX * scheduler_handlerLock;
struct MUTEX * scheduler_processTableLock;
struct MUTEX * scheduler_processCurrentLock;

void scheduler_printProcessTable( void )
{
	struct PROCESS_INFO * process;
	kernel_printf("[*] printing process table...\n" );
	for( process=scheduler_processBottom ; process!=NULL ; process=process->next )
		kernel_printf("\tprocess %d %x -> %x\n", process->id, process, process->next );
	kernel_printf("[*] done.\n" );
}

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * process )
{
	mutex_lock( scheduler_processTableLock );
	
	if( scheduler_processBottom == NULL )
		scheduler_processBottom = process;
	else
		scheduler_processTop->next = process;
	scheduler_processTop = process;

	scheduler_processTop->next = NULL;

	mutex_unlock( scheduler_processTableLock );
	
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

DWORD scheduler_select( void )
{
	struct PROCESS_INFO * processNext;
	
	processNext = scheduler_processCurrent->next;
	
	if( scheduler_processCurrent == NULL )
	{
		// lock critical section as we are about to modify scheduler_processCurrent
		mutex_lock( scheduler_processCurrentLock );
		scheduler_processCurrent = scheduler_processBottom;
		scheduler_processCurrent->tick_slice = PROCESS_TICKS_NORMAL;
		scheduler_processCurrent->state = RUNNING;
		scheduler_switch = TRUE;
		// unlock our critical section
		mutex_unlock( scheduler_processCurrentLock );
		return TRUE;		
	}
	mutex_lock( scheduler_processTableLock );
	// search for another process to run in a round robin fashion 
	for( ; processNext!=scheduler_processCurrent ; processNext=processNext->next )
	{
		// if we have come to the end of the queue, we start from the begining
		if( processNext == NULL )
			processNext = scheduler_processBottom;
		// if we find one in a READY state we choose it
		if( processNext->state == READY )
			break;
	}
	mutex_unlock( scheduler_processTableLock );
	// test if we found another process to run || processNext->id == KERNEL_PID
	if( processNext != scheduler_processCurrent )
	{
		// lock critical section as we are about to modify scheduler_processCurrent
		mutex_lock( scheduler_processCurrentLock );
		// set the current process to a READY state
		scheduler_processCurrent->state = READY;
		// chane the current process to the next one we just picked
		scheduler_processCurrent = processNext;
		// we could set this higher/lower depending on its priority: LOW, NORMAL, HIGH
		scheduler_processCurrent->tick_slice = PROCESS_TICKS_NORMAL;
		// set the process's state to running as we are switching into this process
		scheduler_processCurrent->state = RUNNING;
		scheduler_switch = TRUE;
		// unlock our critical section
		mutex_unlock( scheduler_processCurrentLock );
		// we want to return TRUE to indicate we wish to perform a context switch, see isr.asm
		return TRUE;
	}
	scheduler_switch = FALSE;
	// we return FALSE if we dont need to perform a context switch
	return FALSE;
}

struct PROCESS_INFO * scheduler_removeProcesss( int id )
{
	struct PROCESS_INFO * process, * p;
	// we cant remove the kernel
	if( id == KERNEL_PID )
		return NULL;
	// find the requested process
	process = scheduler_findProcesss( id );
	// we fail if we cant find it
	if( process == NULL )
		return NULL;
	mutex_lock( scheduler_processTableLock );
	// remove the process from the schedulers process table
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
	// if we have removed the current process we will need to select a new one
	if( process == scheduler_processCurrent )
	{
		mutex_lock( scheduler_processCurrentLock );
		scheduler_processCurrent = NULL;
		mutex_unlock( scheduler_processCurrentLock );
		scheduler_select();
	}
	mutex_unlock( scheduler_processTableLock );
	// return the process we just removed
	return process;
}

DWORD scheduler_handler( struct PROCESS_STACK * stack )
{
	DWORD doswitch = FALSE;
	// lock this critical section so we are guaranteed mutual exclusion
	mutex_lock( scheduler_handlerLock );
	// increment our tick counter
	scheduler_ticks++;
	// decrement the current processes time slice by one
	scheduler_processCurrent->tick_slice--;
	// if the current process has reached the end of its tick slice we must select a new process to run
	if( scheduler_processCurrent->tick_slice <= 0 )
	{
		doswitch = scheduler_select();
		if( doswitch )
		{
			// patch the TSS
			//scheduler_tss->ss0 = KERNEL_DATA_SEL;
			//scheduler_tss->cr3 = scheduler_currentProcess->page_dir;
			//scheduler_tss->esp0 = scheduler_currentProcess->current_esp;
		}
	}
	// unlock the critical section
	mutex_unlock( scheduler_handlerLock );
	// return TRUE if we are to perform a context switch or FALSE if not
	return doswitch;
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
	// enable the timer interrupt
	interrupt_enable( IRQ0, scheduler_handler );
}

void scheduler_init()
{
	// create the locks
	scheduler_handlerLock = mutex_create();
	scheduler_processTableLock = mutex_create();
	scheduler_processCurrentLock = mutex_create();
	// set the initial values we need for the kernels process
	kernel_process.tick_slice = PROCESS_TICKS_LOW;
	// set its privilege to SUPERVISOR as this is the kernel
	kernel_process.privilege = SUPERVISOR;
	// we set the state to running as when the first context switch occurs it will be for the kernel process
	kernel_process.state = READY;
	// add it to the scheduler and set it as the current process
	scheduler_processCurrent = scheduler_addProcess( &kernel_process );
	// create a TSS for our software task switching (6.2)
	//scheduler_tss = (struct SEGMENTATION_TSS *)mm_malloc( sizeof(struct SEGMENTATION_TSS) );
	// clear it
	//memset( (void *)scheduler_tss, 0x00, sizeof(struct SEGMENTATION_TSS) );
	// setup the TSS Descriptor (6.2.2)
	//segmentation_setEntry( KERNEL_TSS_SEL, (DWORD)scheduler_tss, sizeof(struct SEGMENTATION_TSS)-1, 0x89, 0x00 );
	// reload GDT
	//segmentation_reload();
	// load the task register with the TSS selector
	//segmentation_ltr( KERNEL_TSS_SEL );
}
