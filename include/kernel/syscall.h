#ifndef _KERNEL_SYSCALL_H_
#define _KERNEL_SYSCALL_H_

#include <sys/types.h>
#include <kernel/pm/process.h>

#define SYSCALL_SWITCH			FAIL-1

#define SYSTEM_CALL0( function, process ) function.function0( process )
#define SYSTEM_CALL1( function, process ) function.function1( process, (void *)process->kstack->ebx )
#define SYSTEM_CALL2( function, process ) function.function2( process, (void *)process->kstack->ebx, (void *)process->kstack->ecx )
#define SYSTEM_CALL3( function, process ) function.function3( process, (void *)process->kstack->ebx, (void *)process->kstack->ecx, (void *)process->kstack->edx )

typedef int (*syscall0)( struct PROCESS_INFO * );
typedef int (*syscall1)( struct PROCESS_INFO *, void * );
typedef int (*syscall2)( struct PROCESS_INFO *, void *, void * );
typedef int (*syscall3)( struct PROCESS_INFO *, void *, void *, void * );

struct SYSCALL_FUNCTION
{
	union
	{
		void * function;
		syscall0 function0;
		syscall1 function1;
		syscall2 function2;
		syscall3 function3;
	};
};

struct SYSCALL
{
	int parameters;
	struct SYSCALL_FUNCTION function;
};

enum
{
	SYSCALL_MININDEX=0,
	
	SYSCALL_OPEN=0,
	SYSCALL_CLOSE,
	SYSCALL_CLONE,
	SYSCALL_READ,
	SYSCALL_WRITE,
	SYSCALL_SEEK,
	SYSCALL_CONTROL,
	SYSCALL_MOUNT,
	SYSCALL_UNMOUNT,
	SYSCALL_CREATE,
	SYSCALL_DELETE,
	SYSCALL_RENAME,
	SYSCALL_COPY,
	SYSCALL_LIST,
	SYSCALL_SPAWN,
	SYSCALL_KILL,
	SYSCALL_SLEEP,
	SYSCALL_WAKE,
	SYSCALL_WAIT,
	SYSCALL_LOAD,
	SYSCALL_EXIT,
	SYSCALL_MORECORE,
	
	SYSCALL_MAXINDEX=SYSCALL_MORECORE,
	SYSCALL_MAXCALLS
};

int syscall_init( void );

#endif
