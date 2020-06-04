/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/mm/physical.h>
#include <kernel/multiboot.h>
#include <kernel/mm/paging.h>
#include <kernel/mm/dma.h>
#include <kernel/kernel.h>
#include <kernel/pm/sync/mutex.h>
#include <kernel/interrupt.h>
#include <lib/libc/string.h>

extern unsigned char * start;
extern unsigned char * end;

struct MUTEX physical_bitmapLock;

unsigned char * physical_bitmap;

int physical_bitmapSize;

void * physical_maxAddress;

int physical_isPageFree( void * physicalAddress )
{
	if( physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] & ( 1 << BITMAP_BIT_INDEX( physicalAddress ) ) )
		return FAIL;
	return SUCCESS;
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
		{
			// if it is, allocate it and return the address
			physical_pageAllocAddress( low );
			return low;
		}
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

// allocate a page in low memory, typically for DMA operations as ISA DMA memory must be below 16 MB
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
	// we could let the caller try to handle this but for now kernel panic!
	if( physicalAddress == NULL )
		kernel_panic( NULL, "No more physical memory available." );
	// return the allocated page address or NULL if no physical memory left
	return physicalAddress;
}

void physical_pageFree( void * physicalAddress )
{
	// lock the critical section
	mutex_lock( &physical_bitmapLock );
	// catch any bad calls so we dont page fault then mark the address as freed
	if( physicalAddress != NULL && physicalAddress < physical_maxAddress )
		physical_bitmap[ BITMAP_BYTE_INDEX( physicalAddress ) ] &= ~( 1 << BITMAP_BIT_INDEX( physicalAddress ) );
	// unlock the critical section
	mutex_unlock( &physical_bitmapLock );
}

int physical_init( struct MULTIBOOT_INFO * m )
{
	void * physicalAddress;
	// create the lock
	mutex_init( &physical_bitmapLock );
	// we need the multiboot info to give us the mem size so we can init the bitmap
	if( (m->flags & MULTIBOOT_FLAG_MEM) != MULTIBOOT_FLAG_MEM )
		return FAIL;
	// calculate the size of the bitmap so we have 1bit for every 4KB page in actual physical memory
	physical_bitmapSize = ( ( ( m->mem_upper / SIZE_1KB ) + 1 ) * 256 ) / 8;
	// calculate the maximum physical memory address we can use
	physical_maxAddress = (void *)( (SIZE_1KB * SIZE_1KB) * ( ( m->mem_upper / SIZE_1KB ) + 1 ) );
	// set the address of the bitmap pointer to the end location of the kernel
	physical_bitmap = (unsigned char *)&end;
	// initially we clear the bitmap so all pages are marked as free
	memset( (void *)physical_bitmap, 0x00, physical_bitmapSize );
	// if the multiboot info contains a memory map we will use it
	if( (m->flags & MULTIBOOT_FLAG_MMAP) == MULTIBOOT_FLAG_MMAP )
	{
		struct MULTIBOOT_MEMORY_MAP * mmap = m->mmap;
		// mark all pages as allocated
		memset( (void *)physical_bitmap, 0xFF, physical_bitmapSize );
		// loop through all entries in the memory map
		while( (DWORD)mmap < ((DWORD)m->mmap + m->mmap_length) )
		{
			// free all pages marked as available in the memory map
			if( mmap->type == MULTIBOOT_MMAP_AVAILABLE )
			{
				for( physicalAddress=(void *)mmap->base_low ; physicalAddress<(void *)(mmap->base_low+mmap->length_low) ; physicalAddress+=SIZE_4KB )
					physical_pageFree( physicalAddress );
			}
			// get the next entry in the memory map
			mmap = (struct MULTIBOOT_MEMORY_MAP *)( (DWORD)mmap + mmap->size + sizeof(DWORD) );
		}
	}
	else
	{
		// if we dont have a memory map, allocate the system memory we know should be reserved
		physical_pageAllocAddress( KERNEL_VGA_PADDRESS );
	}
	// finally, reserve all the physical memory currently being taken up by the kernel and the physical memory
	// bitmap tacked on to the end of the kernel image, this avoids us allocating this memory later on.
	// ...lets hope it works :)
	for( physicalAddress=V2P(&start) ; physicalAddress<V2P(&end)+physical_bitmapSize ; physicalAddress+=SIZE_4KB )
		physical_pageAllocAddress( physicalAddress );
	// we finished successfully
	return SUCCESS;
}
