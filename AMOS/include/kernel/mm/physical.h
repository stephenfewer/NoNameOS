#ifndef _KERNEL_MM_PHYSICAL_H_
#define _KERNEL_MM_PHYSICAL_H_

#include <sys/types.h>

#define SIZE_4KB		4096
#define SIZE_1KB		1024

#define BITMAP_BYTE_INDEX( address )( (address / SIZE_4KB) / 8  )

#define BITMAP_BIT_INDEX( address )( 8 - ((address / SIZE_4KB) % 8 ) - 1 )

DWORD physical_pageAlloc();

DWORD physical_pageAllocAddress( DWORD );

void physical_pageFree( DWORD );

void physical_init( DWORD );

inline int physical_isPageFree( DWORD );


#endif
