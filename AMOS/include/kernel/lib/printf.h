#ifndef _KERNEL_LIB_PRINTF_H_
#define _KERNEL_LIB_PRINTF_H_

#include <sys/types.h>
#include <kernel/io/io.h>

void printf( struct IO_HANDLE *, char *, va_list );

#endif
