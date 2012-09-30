#ifndef _LIB_LIBC_STDLIB_H_
#define _LIB_LIBC_STDLIB_H_

#include <sys/types.h>
//#include <kernel/mm/paging.h>
#include <kernel/pm/process.h>

#define PAGESIZE			4096

#define HEAP_ADDRESS		PROCESS_USER_HEAP_VADDRESS

struct HEAPITEM
{
	struct HEAPITEM * next;
	unsigned int size;
	unsigned int used;
} PACKED;

void * malloc( DWORD );

void free( void * );

int atoi( char * );

#endif
