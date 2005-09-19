#include <kernel/mm/physical.h>
#include <kernel/mm/paging.h>
#include <kernel/kernel.h>
#include <kernel/console.h>

extern void start;
extern void end;

static char * pm_bitmap = (char *)&end;

void physical_init( DWORD mem_upper )
{
	DWORD physicalAddress = (DWORD)&start;
	
	int pm_bitmap_size = ( ( ( mem_upper / SIZE_1KB ) + 1 ) * 256 ) / 8;
	
	memset( (BYTE *)pm_bitmap, 0x00, pm_bitmap_size );

	// reserve all the memory currently being taken up by the
	// kernel and the physical memory bitmap tacked on to the
	// end of the kernel image, this avoids us allocating this
	// memory later on, lets hope it works :)
	
	while( physicalAddress < (DWORD)(&end + pm_bitmap_size ) )
	{
		physical_pageAllocAddress( physicalAddress );
		physicalAddress += SIZE_4KB;
	}
}

DWORD physical_pageAlloc()
{
	DWORD physicalAddress = 0L;//0x00500000;

	// linear search! ohh dear :)
	while( !physical_isPageFree( physicalAddress ) )
		physicalAddress += SIZE_4KB;

	return physical_pageAllocAddress( physicalAddress );
}

DWORD physical_pageAllocAddress( DWORD physicalAddress )
{
	if( physical_isPageFree( physicalAddress ) )
	{
		pm_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] |= ( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
		return physicalAddress;
	}
	return 0L;
}

void physical_pageFree( DWORD physicalAddress )
{
	if( !physical_isPageFree( physicalAddress ) )
		pm_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] &= ~( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
}

inline int physical_isPageFree( DWORD physicalAddress )
{
	if( pm_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] & ( 1 << BITMAP_BIT_INDEX( physicalAddress ) ) )
		return FALSE;
	return TRUE;
}

