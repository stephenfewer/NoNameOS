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
#include <kernel/mm/dma.h>
#include <kernel/kernel.h>
#include <kernel/pm/sync/mutex.h>
#include <kernel/interrupt.h>
#include <lib/string.h>

extern void start;
extern void end;

struct MUTEX physical_bitmapLock;

char * physical_bitmap;

int physical_bitmapSize;

void * physical_maxAddress;

inline int physical_isPageFree( void * physicalAddress )
{
	if( physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] & ( 1 << BITMAP_BIT_INDEX( physicalAddress ) ) )
		return FAIL;
	return SUCCESS;
}

inline void * physical_pageAllocAddress( void * physicalAddress )
{
	physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] |= ( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
	return physicalAddress;
}

int physical_getBitmapSize()
{
	return physical_bitmapSize;
}

void * physical_pageAllocLimit( void * low, void * high )
{
	// linear search from our low address untill our high address
	while( low < high )
	{
		// test if the current physical address is free
		if( physical_isPageFree( low ) == SUCCESS )
			// if it is, allocate it and return the address
			return physical_pageAllocAddress( low );
		// increment the physical address to the next page
		low += SIZE_4KB;
	}
	// return NULL if we couldn't allocate a page
	return NULL;
}

void * physical_pageAllocHigh()
{
	void * physicalAddress = NULL;
	// lock the critical section
	mutex_lock( &physical_bitmapLock );
	// try to allocate a page in high memory
	if( physical_maxAddress > PHYSICAL_HIGH_PADDRESS )
		physicalAddress = physical_pageAllocLimit( PHYSICAL_HIGH_PADDRESS, physical_maxAddress );	
	// unlock the critical section
	mutex_unlock( &physical_bitmapLock );
	// return physical page address or NULL if no physical memory left
	return physicalAddress;
}

// allocate a page in low memory, typically for DMA operations
void * physical_pageAllocLow()
{
	void * physicalAddress;
	// lock the critical section
	mutex_lock( &physical_bitmapLock );
	// try to allocate a page in low memory
	if( PHYSICAL_HIGH_PADDRESS > physical_maxAddress )
		physicalAddress = physical_pageAllocLimit( PHYSICAL_LOW_PADDRESS, physical_maxAddress );
	else
		physicalAddress = physical_pageAllocLimit( PHYSICAL_LOW_PADDRESS, PHYSICAL_HIGH_PADDRESS );
	// unlock the critical section
	mutex_unlock( &physical_bitmapLock );
	// return physical page address or NULL if no physical memory left
	return physicalAddress;
}

// allocate a physical page in either high memory or low memory. we try high first
// to reserve as much low memory as possible for things like DMA that need low mem
void * physical_pageAlloc()
{
	void * physicalAddress;
	// first try to alloc from high memory
	physicalAddress = physical_pageAllocHigh();
	// if no high mem available, try from low memory
	if( physicalAddress == NULL )
		physicalAddress = physical_pageAllocLow();
	// return the allocated page address or NULL if no physical memory left
	return physicalAddress;
}

void physical_pageFree( void * physicalAddress )
{
	// lock the critical section
	mutex_lock( &physical_bitmapLock );
	// if the address has been allocated, mark it as freed
	if( physical_isPageFree( physicalAddress ) == FAIL )
		physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] &= ~( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
	// unlock the critical section
	mutex_unlock( &physical_bitmapLock );
}

int physical_init( DWORD memUpper )
{
	void * physicalAddress;
	
	// we cant create the lock as we dont have access to mm_kmalloc() yet so we just init it
	mutex_init( &physical_bitmapLock );
	
	mutex_lock( &physical_bitmapLock );
	
	// calculate the size of the bitmap so we have 1bit
	// for every 4KB page in actual physical memory
	physical_bitmapSize = ( ( ( memUpper / SIZE_1KB ) + 1 ) * 256 ) / 8;
	
	// calculate the maximum physical memory address we can use
	physical_maxAddress = (void *)( (SIZE_1KB * SIZE_1KB) * ( ( memUpper / SIZE_1KB ) + 1 ) );
	
	// set the address of the bitmap pointer to the end location of the kernel
	physical_bitmap = (char *)&end;

	// clear the bitmap so all pages are marked as free
	memset( (void *)physical_bitmap, 0x00, physical_bitmapSize );

	// reserve the bios and video memory
	for( physicalAddress=(void *)0xA0000 ; physicalAddress<(void *)0xD8000 ; physicalAddress+=SIZE_4KB )
		physical_pageAllocAddress( physicalAddress );
	physical_pageAllocAddress( KERNEL_VGA_PADDRESS );

	// reserve all the physical memory currently being taken up by
	// the kernel and the physical memory bitmap tacked on to the
	// end of the kernel image, this avoids us allocating this memory
	// later on, lets hope it works :)
	for( physicalAddress=V2P(&start) ; physicalAddress<V2P(&end)+physical_bitmapSize ; physicalAddress+=SIZE_4KB )
		physical_pageAllocAddress( physicalAddress );
		
	mutex_unlock( &physical_bitmapLock );
	
	return SUCCESS;
}
