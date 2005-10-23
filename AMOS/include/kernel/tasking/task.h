#ifndef _KERNEL_TASKING_TASK_H_
#define _KERNEL_TASKING_TASK_H_

#include <sys/types.h>
//#include <kernel/mm/paging.h>

#define TASK_CODEADDRESS	(void *)0x10000000
#define TASK_STACKADDRESS	(void *)0x20000000
#define TASK_STACKSIZE		SIZE_4KB

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

enum
{
	READY=0,
	RUNNING,
	BLOCKED,
};

struct TASK_INFO
{
	int id;
	int tick_slice;
	int state;
	DWORD current_esp;
	void * stack;
	struct PAGE_DIRECTORY * page_dir;	
};

void task_destroy( struct TASK_INFO * );

struct TASK_INFO * task_create( void (*thread)() );

#endif 
