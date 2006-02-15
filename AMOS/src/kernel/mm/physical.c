/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/mm/physical.h>
#include <kernel/mm/paging.h>
#include <kernel/kernel.h>
#include <kernel/lib/string.h>
#include <kernel/pm/sync/mutex.h>

extern void start;
extern void end;

struct MUTEX physical_bitmapLock;

char * physical_bitmap;

int physical_bitmapSize;

inline int physical_isPageFree( void * physicalAddress )
{
	if( physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] & ( 1 << BITMAP_BIT_INDEX( physicalAddress ) ) )
		return FALSE;
	return TRUE;
}

void * physical_pageAllocAddress( void * physicalAddress )
{
	physicalAddress = PAGE_ALIGN( physicalAddress );
	
	if( physical_isPageFree( physicalAddress ) )
	{
		physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] |= ( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
		return physicalAddress;
	}
	return 0L;
}

int physical_getBitmapSize()
{
	return physical_bitmapSize;
}

// To-Do: return 0x00000000 if no physical memory left
void * physical_pageAlloc()
{
	// better to reserver the address 0x00000000 so we can better
	// detect null pointer exceptions...
	void * physicalAddress = (void *)0x00001000;
	// lock this critical section so we cant allocate the smae page twice!
	mutex_lock( &physical_bitmapLock );
	
	// linear search! ohh dear :)
	while( !physical_isPageFree( physicalAddress ) )
		physicalAddress += SIZE_4KB;

	physicalAddress = physical_pageAllocAddress( physicalAddress );
	// unlock the critical section
	mutex_unlock( &physical_bitmapLock );
	// return the physical address we just allocated
	return physicalAddress;
}

void physical_pageFree( void * physicalAddress )
{
	// lock the critical section
	mutex_lock( &physical_bitmapLock );
	if( !physical_isPageFree( physicalAddress ) )
		physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] &= ~( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
	// unlock the critical section
	mutex_unlock( &physical_bitmapLock );
}

void physical_init( DWORD memUpper )
{
	void * physicalAddress;
	
	// we cant create the lock as we dont have access to mm_malloc() yet so we just init it
	mutex_init( &physical_bitmapLock );
	
	mutex_lock( &physical_bitmapLock );
	
	// calculate the size of the bitmap so we have 1bit
	// for every 4KB page in actual physical memory
	physical_bitmapSize = ( ( ( memUpper / SIZE_1KB ) + 1 ) * 256 ) / 8;

	physical_bitmap = (char *)&end;

	// clear the bitmap so all pages are marked as free
	memset( (void *)physical_bitmap, 0x00, physical_bitmapSize );

	// reserve the bios and video memory
	for( physicalAddress=(void *)0xA0000 ; physicalAddress<(void *)0x100000 ; physicalAddress+=SIZE_4KB )
		physical_pageAllocAddress( physicalAddress );
		
	// reserve all the physical memory currently being taken up by
	// the kernel and the physical memory bitmap tacked on to the
	// end of the kernel image, this avoids us allocating this memory
	// later on, lets hope it works :)
	for( physicalAddress=V2P(&start) ; physicalAddress<V2P(&end)+physical_bitmapSize ; physicalAddress+=SIZE_4KB )
		physical_pageAllocAddress( physicalAddress );
		
	mutex_unlock( &physical_bitmapLock );
}
