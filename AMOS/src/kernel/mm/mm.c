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

#include <kernel/mm/mm.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/paging.h>
#include <kernel/pm/process.h>
#include <kernel/kernel.h>
#include <kernel/lib/string.h>

extern struct PROCESS_INFO kernel_process;

void mm_init( DWORD memUpper )
{	
	// setup the physical memory manager, after this we can use physical_pageAlloc()
	physical_init( memUpper );
	
	// setup and initilise segmentation
	segmentation_init();
	
	// setup and initilise paging
	paging_init();
	
	// setup the kernel heap structure
	kernel_process.heap.heap_base   = KERNEL_HEAP_VADDRESS;
	kernel_process.heap.heap_top    = NULL;
	kernel_process.heap.heap_bottom = NULL;
	
	// from here on in we can use mm_malloc() & mm_free()
}

// increase the processes heap by some amount, this will be rounded up by the page size 
void * mm_morecore( struct PROCESS_INFO * process, DWORD size )
{
	// calculate how many pages we will need
	int pages = ( size / PAGE_SIZE ) + 1;
	// when process->heap.heap_top == NULL we must create the initial heap
	if( process->heap.heap_top == NULL )
		process->heap.heap_bottom = process->heap.heap_top = process->heap.heap_base;
	// set the address to return
	void * prevTop = process->heap.heap_top;
	// create the pages
	for( ; pages-->0 ; process->heap.heap_top+=PAGE_SIZE )
	{
		// alloc a physical page in mamory
		void * physicalAddress = physical_pageAlloc();
		if( physicalAddress == 0L )
			return NULL;
		// map it onto the end of the processes heap
		paging_setPageTableEntry( process, process->heap.heap_top, physicalAddress, TRUE );
		// clear it for safety
		memset( process->heap.heap_top, 0x00, PAGE_SIZE );
	}
	// return the start address of the memory we allocated to the heap
	return prevTop;
}

// free a previously allocated item from the kernel heap
void mm_free( void * address )
{
	struct MM_HEAPITEM * tmp_item;
	struct MM_HEAPITEM * item = (struct MM_HEAPITEM *)( address - sizeof(struct MM_HEAPITEM) );
	// find it
	for( tmp_item=kernel_process.heap.heap_bottom ; tmp_item!=NULL ; tmp_item=tmp_item->next )
	{
		if( tmp_item == item )
			break;
	}
	// not found
	if( tmp_item == NULL )
	{
		return;
	}
	// free it
	tmp_item->used = FALSE;
	// coalesce any free adjacent items
	for( tmp_item=kernel_process.heap.heap_bottom ; tmp_item!=NULL ; tmp_item=tmp_item->next )
	{
		while( !tmp_item->used && tmp_item->next!=NULL && !tmp_item->next->used )
		{
			tmp_item->size += sizeof(struct MM_HEAPITEM) + tmp_item->next->size;
			tmp_item->next = tmp_item->next->next;
		}
	}
}

// allocates an arbiturary size of memory (via first fit) from the kernel heap
void * mm_malloc( DWORD size )
{
	struct MM_HEAPITEM * new_item, * tmp_item;
	int total_size;
	// sanity check
	if( size == 0 )
	{
		return NULL;
	}
	// round up by 8 bytes and add header size
	total_size = ( ( size + 7 ) & ~7 ) + sizeof(struct MM_HEAPITEM);
	// search for first fit
	for( new_item=kernel_process.heap.heap_bottom ; new_item!=NULL ; new_item=new_item->next )
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
		new_item = mm_morecore( &kernel_process, total_size );
		if( new_item == NULL )
		{
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
	return (void *)( (int)new_item + sizeof(struct MM_HEAPITEM) );
}
