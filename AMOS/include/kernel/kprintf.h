#ifndef _KERNEL_KPRINTF_H_
#define _KERNEL_KPRINTF_H_

#include <sys/types.h>
#include <kernel/fs/vfs.h>

void kprintf( struct VFS_HANDLE *, char *, va_list );

#endif
