#ifndef _KERNEL_PM_SYNC_MUTEX_H_
#define _KERNEL_PM_SYNC_MUTEX_H_

#include <sys/types.h>

struct MUTEX
{
	volatile DWORD lock;
};

struct MUTEX * mutex_create();

void mutex_lock( struct MUTEX * );

void mutex_unlock( struct MUTEX * );

#endif
