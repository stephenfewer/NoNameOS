#ifndef _KERNEL_PM_SCHEDULER_H_
#define _KERNEL_PM_SCHEDULER_H_

#include <sys/types.h>
#include <kernel/pm/process.h>
#include <kernel/pm/sync/mutex.h>
#include <kernel/interrupt.h>

#define SCHEDULER_INTERRUPT			IRQ0

struct SCHEDULER_PROCESS_TABLE
{
	struct MUTEX lock;
	// the linked list of processes that defines the process table
	//struct PROCESS_INFO * top;
	struct PROCESS_INFO * bottom;
};

int scheduler_setProcess( int, int, int );

void scheduler_printProcessTable( void );

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * );

struct PROCESS_INFO * scheduler_select( struct PROCESS_INFO * );

__inline__ void scheduler_switch( void );

int scheduler_init( void );

#endif
