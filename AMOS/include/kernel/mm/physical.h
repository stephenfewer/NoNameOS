#ifndef _KERNEL_MM_PHYSICAL_H_
#define _KERNEL_MM_PHYSICAL_H_

#include <sys/types.h>

#define SIZE_4KB		4096
#define SIZE_1KB		1024

#define BITMAP_BYTE_INDEX( address )( ((DWORD)address / SIZE_4KB) / 8  )

#define BITMAP_BIT_INDEX( address )( 8 - (((DWORD)address / SIZE_4KB) % 8 ) - 1 )

void * physical_pageAlloc();

void physical_pageFree( void * );

int physical_init( DWORD );

int physical_getBitmapSize();

#endif
