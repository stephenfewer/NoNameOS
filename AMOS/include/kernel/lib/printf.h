#ifndef _KERNEL_LIB_PRINTF_H_
#define _KERNEL_LIB_PRINTF_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>

void printf( struct VFS_HANDLE *, char *, va_list );

void print( struct VFS_HANDLE *, char *, ... );

int get( struct VFS_HANDLE *, char *, int );

#endif
