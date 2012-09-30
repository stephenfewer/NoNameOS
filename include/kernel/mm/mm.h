#ifndef _KERNEL_MM_MM_H_
#define _KERNEL_MM_MM_H_

#include <sys/types.h>
#include <kernel/pm/process.h>
#include <kernel/multiboot.h>

struct MM_HEAPITEM
{
	struct MM_HEAPITEM * next;
	unsigned int size;
	unsigned int used;//:1;
	//unsigned int available:7;
} PACKED;

int mm_init( struct MULTIBOOT_INFO * );

void mm_pmemcpyto( void *, void *, int );

void mm_pmemcpyfrom( void *, void *, int );

void * mm_morecore( struct PROCESS_INFO *, DWORD );

void mm_kfree( void * );

void * mm_kmalloc( DWORD );

#endif
