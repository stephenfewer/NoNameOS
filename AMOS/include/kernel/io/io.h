#ifndef _KERNEL_IO_IO_H_
#define _KERNEL_IO_IO_H_

#include <sys/types.h>

#define IO_TOTALDEVICES	2
enum DEVICE_TYPE
{
	UNKNOWN=-1,
	CONSOLE,
	KEYBOARD
};

struct IO_CALLTABLE
{
	struct DEVICE_HANDLE * (*open)(char *);
	int (*close)(struct DEVICE_HANDLE *);
	int (*read)(struct DEVICE_HANDLE *, BYTE *, DWORD );
	int (*write)(struct DEVICE_HANDLE *, BYTE *, DWORD );
};

struct DEVICE_HANDLE
{
	enum DEVICE_TYPE type;
};

struct DEVICE_HANDLE * io_open( char * );

int io_close( struct DEVICE_HANDLE * );

int io_read( struct DEVICE_HANDLE *, BYTE *, DWORD );

int io_write( struct DEVICE_HANDLE *, BYTE *, DWORD );

void io_init();

#endif

