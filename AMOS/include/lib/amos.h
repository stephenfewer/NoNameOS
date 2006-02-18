#ifndef _LIB_AMOS_H_
#define _LIB_AMOS_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>

#define READWRITE	VFS_MODE_READWRITE

int open( char *, int );

int close( int );

int read( int, BYTE *, DWORD );

int write( int, BYTE *, DWORD );

int seek( int, DWORD, BYTE );

int control( int, DWORD, DWORD );

#endif 
