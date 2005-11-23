#ifndef _KERNEL_IO_DEVICE_H_
#define _KERNEL_IO_DEVICE_H_

#include <sys/types.h>

struct DEVICE_ENTRY
{
	struct DEVICE_ENTRY	* next;
	struct IO_CALLTABLE * calltable;
	char * name;
};

struct DEVICE_ENTRY * device_add( char *, struct IO_CALLTABLE * );

void device_remove( struct DEVICE_ENTRY * );

struct DEVICE_ENTRY * device_find( char * );

#endif
