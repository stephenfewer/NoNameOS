#ifndef _KERNEL_PM_SYNC_MUTEX_H_
#define _KERNEL_PM_SYNC_MUTEX_H_

#include <sys/types.h>

struct MUTEX
{
	DWORD foo;
};

void mutex_init( struct MUTEX * );

// save the CPU flags and disable interrupts locally 
#define mutex_lock( m ) \
	DWORD flags; \
	ASM( "pushfl" ::: "memory" ); \
	ASM( "popl %0" : "=g" ( flags ) :: "memory" ); \
	interrupt_disableAll(); \

// restore the flags 
#define mutex_unlock( m ) \
    ASM( "pushl %0" :: "g" ( flags ): "memory" ); \
    ASM( "popfl" ::: "memory" ); \

#endif
