#ifndef _KERNEL_PM_SCHEDULER_H_
#define _KERNEL_PM_SCHEDULER_H_

#include <sys/types.h>
#include <kernel/pm/process.h>

#define MAX_TASKS	255

void scheduler_printProcessTable( void );

struct PROCESS_INFO * scheduler_addProcess( struct PROCESS_INFO * );

struct PROCESS_INFO * scheduler_findProcesss( int );

DWORD scheduler_select( void );

struct PROCESS_INFO * scheduler_removeProcesss( int );

void scheduler_enable();

void scheduler_init();

#endif
