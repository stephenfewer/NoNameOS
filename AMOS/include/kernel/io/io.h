#ifndef _KERNEL_IO_IO_H_
#define _KERNEL_IO_IO_H_

#include <sys/types.h>

#define IO_SUCCESS		-1
#define IO_FAIL			-1

// the origin defines for io_seek()
#define SEEK_START		0
#define SEEK_CURRENT	1
#define SEEK_END		2

struct IO_CALLTABLE
{
	struct IO_HANDLE * (*open)(struct IO_HANDLE *, char *);
	int (*close)(struct IO_HANDLE *);
	int (*read)(struct IO_HANDLE *, BYTE *, DWORD );
	int (*write)(struct IO_HANDLE *, BYTE *, DWORD );
	int (*seek)(struct IO_HANDLE *, DWORD, BYTE );
	int (*control)(struct IO_HANDLE *, DWORD, DWORD );
};

struct IO_HANDLE
{
	struct DEVICE_ENTRY * device;
	DWORD data_arg;
	void * data_ptr;
};

struct IO_HANDLE * io_open( char * );

int io_close( struct IO_HANDLE * );

int io_read( struct IO_HANDLE *, BYTE *, DWORD );

int io_write( struct IO_HANDLE *, BYTE *, DWORD );

int io_seek( struct IO_HANDLE *, DWORD, BYTE );

int io_control( struct IO_HANDLE *, DWORD, DWORD );

void io_init();

#endif

