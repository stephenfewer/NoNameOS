#ifndef _KERNEL_SYSCALL_H_
#define _KERNEL_SYSCALL_H_

#include <sys/types.h>

#define SYSCALL_INTERRUPT		255

typedef int (*syscall0)( void );
typedef int (*syscall1)( void * );
typedef int (*syscall2)( void *, void * );
typedef int (*syscall3)( void *, void *, void * );

struct SYSCALL_ENTRY
{
	syscall0 function;
	int parameters;
};

enum
{
	SYSCALL_MININDEX=0,
	
	SYSCALL_OPEN=0,
	SYSCALL_CLOSE,
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
	SYSCALL_MORECORE,
	
	SYSCALL_MAXINDEX=SYSCALL_MORECORE,
	SYSCALL_MAXCALLS
};

void syscall_init( void );

#endif
