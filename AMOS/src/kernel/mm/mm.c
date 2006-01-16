/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    License: GNU General Public License (GPL)
 */

#include <kernel/mm/mm.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/paging.h>
#include <kernel/kprintf.h>
#include <kernel/kernel.h>
#include <kernel/lib/string.h>

extern struct PAGE_DIRECTORY * paging_kernelPageDir;

struct MM_HEAP mm_kernelHeap;

void mm_init( DWORD memUpper )
{	
	// setup the physical memory manager, after this we can use physical_pageAlloc()
	physical_init( memUpper );
	
	// setup and initilise segmentation
	segmentation_init();
	
	// setup and initilise paging
	paging_init();
	
	// setup the kernel heap structure
	mm_kernelHeap.heap_bottom = NULL;
	mm_kernelHeap.heap_top = NULL;
	mm_kernelHeap.page_dir = paging_kernelPageDir;
	
	// from here on in we can use mm_malloc() & mm_free()
}

// increase the heap by some amount, this will be rounded up by the page size 
void * mm_morecore( struct MM_HEAP * heap, DWORD size )
{
	// calculate how many pages we will need
	int pages = ( size / PAGE_SIZE ) + 1;
	// when heap->heap_top == NULL we must create the initial heap
	if( heap->heap_top == NULL )
		heap->heap_bottom = heap->heap_top = KERNEL_HEAP_VADDRESS;
	// set the address to return
	void * prevTop = heap->heap_top;
	// create the pages
	for( ; pages-->0 ; heap->heap_top+=PAGE_SIZE )
	{
		// alloc a physical page in mamory
		void * physicalAddress = physical_pageAlloc();
		if( physicalAddress == 0L )
			return NULL;
		// map it onto the end of the kernel heap
		paging_setPageTableEntry( heap->page_dir, heap->heap_top, physicalAddress, TRUE );
		// clear it for safety
		memset( heap->heap_top, 0x00, PAGE_SIZE );
	}
	// return the start address of the memory we allocated to the heap
	return prevTop;
}

// free a previously allocated item from the kernel heap
void mm_free( void * address )
{
	kernel_lock();
	struct MM_HEAPITEM * tmp_item;
	struct MM_HEAPITEM * item = (struct MM_HEAPITEM *)( address - sizeof(struct MM_HEAPITEM) );
	// find it
	for( tmp_item=mm_kernelHeap.heap_bottom ; tmp_item!=NULL ; tmp_item=tmp_item->next )
	{
		if( tmp_item == item )
			break;
	}
	// not found
	if( tmp_item == NULL )
	{
		kernel_unlock();
		return;
	}
	// free it
	tmp_item->used = FALSE;
	// coalesce any free adjacent items
	for( tmp_item=mm_kernelHeap.heap_bottom ; tmp_item!=NULL ; tmp_item=tmp_item->next )
	{
		while( !tmp_item->used && tmp_item->next!=NULL && !tmp_item->next->used )
		{
			tmp_item->size += sizeof(struct MM_HEAPITEM) + tmp_item->next->size;
			tmp_item->next = tmp_item->next->next;
		}
	}
	kernel_unlock();
}

// allocates an arbiturary size of memory (via first fit) from the kernel heap
void * mm_malloc( DWORD size )
{
	kernel_lock();
	struct MM_HEAPITEM * new_item, * tmp_item;
	int total_size;
	// sanity check
	if( size == 0 )
	{
		kernel_unlock();
		return NULL;
	}
	// round up by 8 bytes and add header size
	total_size = ( ( size + 7 ) & ~7 ) + sizeof(struct MM_HEAPITEM);
	// search for first fit
	for( new_item=mm_kernelHeap.heap_bottom ; new_item!=NULL ; new_item=new_item->next )
	{
		if( !new_item->used && (total_size <= new_item->size) )
			break;
	}
	// if we found one
	if( new_item != NULL )
	{
		tmp_item = (struct MM_HEAPITEM *)( (int)new_item + total_size );
		tmp_item->size = new_item->size - total_size;
		tmp_item->used = FALSE;
		tmp_item->next = new_item->next;
		
		new_item->size = size;
		new_item->used = TRUE;
		new_item->next = tmp_item;
	}
	else
	{
		// didnt find a fit so we must increase the heap to fit
		new_item = mm_morecore( &mm_kernelHeap, total_size );
		if( new_item == NULL )
		{
			kernel_unlock();
			return NULL;	
		}
		// create an empty item for the extra space mm_morecore() gave us
		// we can calculate the size because morecore() allocates space that is page aligned
		tmp_item = (struct MM_HEAPITEM *)( (int)new_item + total_size );
		tmp_item->size = PAGE_SIZE - (total_size%PAGE_SIZE ? total_size%PAGE_SIZE : total_size) - sizeof(struct MM_HEAPITEM);
		tmp_item->used = FALSE;
		tmp_item->next = NULL;
		// create the new item
		new_item->size = size;
		new_item->used = TRUE;
		new_item->next = tmp_item;
	}
	// return the newly allocated memory location
	kernel_unlock();
	return (void *)( (int)new_item + sizeof(struct MM_HEAPITEM) );
}
