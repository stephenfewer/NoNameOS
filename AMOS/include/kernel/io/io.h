#ifndef _KERNEL_IO_IO_H_
#define _KERNEL_IO_IO_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/dfs.h>

#define IO_BLOCK		0x01
#define IO_CHAR			0x02

struct IO_CALLTABLE
{
	struct IO_HANDLE * (*open)(struct IO_HANDLE *, char *);
	int (*close)(struct IO_HANDLE *);
	int (*clone)( struct IO_HANDLE *, struct IO_HANDLE * );
	int (*read)(struct IO_HANDLE *, BYTE *, DWORD );
	int (*write)(struct IO_HANDLE *, BYTE *, DWORD );
	int (*seek)(struct IO_HANDLE *, DWORD, BYTE );
	int (*control)(struct IO_HANDLE *, DWORD, DWORD );
};

struct IO_HANDLE
{
	struct DFS_ENTRY * device;
	DWORD data_arg;
	void * data_ptr;
};

int io_add( char *, struct IO_CALLTABLE *, int );

int io_remove( char * );

struct IO_HANDLE * io_open( struct DFS_ENTRY * );

int io_close( struct IO_HANDLE * );

int io_clone( struct IO_HANDLE *, struct IO_HANDLE ** );

int io_read( struct IO_HANDLE *, BYTE *, DWORD );

int io_write( struct IO_HANDLE *, BYTE *, DWORD );

int io_seek( struct IO_HANDLE *, DWORD, BYTE );

int io_control( struct IO_HANDLE *, DWORD, DWORD );

int io_init( void );

#endif

