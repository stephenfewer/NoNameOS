#ifndef _KERNEL_MM_MM_H_
#define _KERNEL_MM_MM_H_

#include <sys/types.h>

#define KERNEL_HEAP_VADDRESS	0xD0000000

struct HEAP_ITEM
{
	struct HEAP_ITEM * next;
	unsigned int size;
	unsigned int used;//:1;
	//unsigned int available:7;
} PACKED;

BYTE * mm_memset( BYTE *, BYTE, int );

void mm_init( DWORD );

void mm_free( void * );

void * mm_malloc( DWORD );

#endif
