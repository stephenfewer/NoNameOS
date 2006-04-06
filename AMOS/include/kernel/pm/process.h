#ifndef _KERNEL_PM_PROCESS_H_
#define _KERNEL_PM_PROCESS_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>
#include <kernel/mm/paging.h>

#define PROCESS_CURRENT					-1

#define PROCESS_CONSOLEHANDLE			0
#define PROCESS_MAXHANDLES				256

#define PROCESS_TICKS_CURRENT			-1
#define PROCESS_TICKS_NONE				0
#define PROCESS_TICKS_LOW				1
#define PROCESS_TICKS_NORMAL			5
#define PROCESS_TICKS_HIGH				15

#define PROCESS_USER_CODE_VADDRESS		(void *)0x10000000
#define PROCESS_USER_STACK_VADDRESS		(void *)0x20000000
#define PROCESS_USER_HEAP_VADDRESS		(void *)0x30000000

#define PROCESS_STACKSIZE				4096

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
	CREATED=0,
	READY,
	RUNNING,
	BLOCKED,
	TERMINATED
};

struct PROCESS_HEAP
{
	void * heap_base;
	void * heap_top;
};

struct PROCESS_INFO
{
	// -> the order of these is important, see isr.asm
	struct PROCESS_STACK * kstack;
	struct PAGE_DIRECTORY * page_dir;
	unsigned int privilege;
	void * kstack_base;
	// <-
	int id;
	int parent_id;
	int tick_slice;
	int state;
	char name[VFS_NAMESIZE];
	struct VFS_HANDLE * handles[PROCESS_MAXHANDLES];
	struct PROCESS_HEAP heap;
	
	struct PROCESS_INFO * prev;
};

void process_printStack( struct PROCESS_STACK * );

int process_destroy( struct PROCESS_INFO * );

int process_spawn( struct PROCESS_INFO *, char *, char * );

int process_kill( int );

void process_yield( void );

int process_sleep( struct PROCESS_INFO * );

int process_wake( int );

int process_wait( int );

#endif 
