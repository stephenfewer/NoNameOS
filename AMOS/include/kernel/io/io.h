#ifndef _KERNEL_IO_IO_H_
#define _KERNEL_IO_IO_H_

#include <sys/types.h>

struct IO_CALLTABLE
{
	struct DEVICE_HANDLE * (*open)(struct DEVICE_HANDLE *, char *);
	int (*close)(struct DEVICE_HANDLE *);
	int (*read)(struct DEVICE_HANDLE *, BYTE *, DWORD );
	int (*write)(struct DEVICE_HANDLE *, BYTE *, DWORD );
};

struct DEVICE_HANDLE
{
	struct DEVICE_ENTRY * device;
	void * data;
};

struct DEVICE_HANDLE * io_open( char * );

int io_close( struct DEVICE_HANDLE * );

int io_read( struct DEVICE_HANDLE *, BYTE *, DWORD );

int io_write( struct DEVICE_HANDLE *, BYTE *, DWORD );

void io_init();

#endif

