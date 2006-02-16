#ifndef _KERNEL_PM_PROCESS_H_
#define _KERNEL_PM_PROCESS_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>
#include <kernel/mm/paging.h>

#define PROCESS_TICKS_LOW				1//64
#define PROCESS_TICKS_NORMAL			1//256
#define PROCESS_TICKS_HIGH				1//512

#define PROCESS_USER_CODE_ADDRESS		(void *)0x10000000
#define PROCESS_USER_STACK_ADDRESS		(void *)0x20000000
#define PROCESS_USER_HEAP_ADDRESS		(void *)0x30000000

#define PROCESS_STACKSIZE				SIZE_4KB

struct PROCESS_STACK
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
	DWORD esp0;
	DWORD ss0;
};

enum
{
	READY=0,
	RUNNING,
	BLOCKED,
};

struct PROCESS_HEAP
{
	void * heap_base;
	void * heap_top;
	void * heap_bottom;
};

struct PROCESS_INFO
{
	DWORD current_esp;
	struct PAGE_DIRECTORY * page_dir;
	int id;
    unsigned int privilege;
	int tick_slice;
	int state;
	void * user_stack;
	void * user_heap;
	void * kernel_stack;
	DWORD current_kesp;
	struct VFS_HANDLE * console;
	struct PROCESS_HEAP heap;
	struct PROCESS_INFO * next;
};

int process_spawn( char *, struct VFS_HANDLE * );

int process_kill( int );

int process_sleep();

int process_wake( int );

struct PROCESS_INFO * process_create( void (*thread)(), int );

#endif 
