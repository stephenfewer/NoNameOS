#ifndef _KERNEL_TASKING_TASK_H_
#define _KERNEL_TASKING_TASK_H_

#include <sys/types.h>
//#include <kernel/mm/paging.h>

struct TASK_STACK
{
	DWORD ds;
	DWORD es;
	DWORD fs;
	DWORD gs;
	DWORD edi;
	DWORD esi;
	DWORD ebp;
	DWORD esp;
	DWORD ebx;
	DWORD edx;
	DWORD ecx;
	DWORD eax;
	DWORD intnumber;
	DWORD errorcode;
	DWORD eip;
	DWORD cs;
	DWORD eflags;
	DWORD userstack;
	DWORD ss;
};

struct TASK_INFO
{
	int id;
	int tick_slice;
	DWORD current_esp;
	void * stack;
	struct PAGE_DIRECTORY * page_dir;	
};

void task_destroy( struct TASK_INFO * );

struct TASK_INFO * task_create( void (*thread)() );

#endif 
