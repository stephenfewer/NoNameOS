#ifndef _KERNEL_MM_MM_H_
#define _KERNEL_MM_MM_H_

#include <sys/types.h>
#include <kernel/mm/paging.h>

#define KERNEL_CODE_VADDRESS	(void *)0xC0000000
#define KERNEL_HEAP_VADDRESS	(void *)0xD0000000

struct MM_HEAPITEM
{
	struct MM_HEAPITEM * next;
	unsigned int size;
	unsigned int used;//:1;
	//unsigned int available:7;
} PACKED;

struct MM_HEAP
{
	void * heap_top;
	void * heap_bottom;
	struct PAGE_DIRECTORY * page_dir;
};

void mm_init( DWORD );

void mm_free( void * );

void * mm_malloc( DWORD );

#endif
