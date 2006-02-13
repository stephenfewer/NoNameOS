#ifndef _KERNEL_PM_SCHEDULER_H_
#define _KERNEL_PM_SCHEDULER_H_

#include <sys/types.h>
#include <kernel/pm/process.h>

#define MAX_TASKS	255

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * );

struct PROCESS_INFO * scheduler_removeProcesss( int );

void scheduler_enable();

void scheduler_idle( void );

void scheduler_init();

#endif
