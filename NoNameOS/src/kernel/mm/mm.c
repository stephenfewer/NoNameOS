/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/mm/mm.h>
#include <kernel/mm/physical.h>
#include <kernel/mm/segmentation.h>
#include <kernel/mm/paging.h>
#include <kernel/pm/process.h>
#include <kernel/pm/sync/mutex.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <lib/libc/string.h>

extern struct PROCESS_INFO kernel_process;

struct MUTEX mm_kmallocLock;

int mm_init( struct MULTIBOOT_INFO * m )
{	
	// setup the physical memory manager, after this we can use physical_pageAlloc()
	physical_init( m );
	
	// setup and initilise segmentation
	segmentation_init();
	
	// setup and initilise paging
	paging_init();
	
	// setup the kernel heap structure
	kernel_process.heap.heap_base   = KERNEL_HEAP_VADDRESS;
	kernel_process.heap.heap_top    = NULL;
	
	// init the lock that the kernel mm_kmalloc/mm_kfree will use
	// we cant use mutex_create() as it need mm_kmalloc()
	mutex_init( &mm_kmallocLock );
	
	// from here on in we can use mm_kmalloc() & mm_kfree()
	return SUCCESS;
}

// rename these two functions!, also size must be <= PAGE_SIZE
void mm_pmemcpyto( void * dest_paddress, void * src_vaddress, int size )
{
	// dissable interrupts for atomicity
	interrupt_disableAll();
	// use quickMap() to map in the physical page into our current address space so we may write to it
	void * tempAddress = paging_mapQuick( dest_paddress );
	// copy the data accross
	memcpy( tempAddress, src_vaddress, size );
	// enable interrupts
	interrupt_enableAll();
}

void mm_pmemcpyfrom( void * dest_vaddress, void * src_paddress, int size )
{
	// dissable interrupts for atomicity
	interrupt_disableAll();
	// use quickMap() to map in the physical page into our current address space so we may read from it
	void * tempAddress = paging_mapQuick( src_paddress );
	// copy the data accross
	memcpy( dest_vaddress, tempAddress, size );
	// enable interrupts
	interrupt_enableAll();
}

// increase the processes heap by some amount, this will be rounded up by the page size 
void * mm_morecore( struct PROCESS_INFO * process, DWORD size )
{
	// calculate how many pages we will need
	int pages = ( size / PAGE_SIZE ) + 1;
	// when process->heap.heap_top == NULL we must create the initial heap
	if( process->heap.heap_top == NULL )
		process->heap.heap_top = process->heap.heap_base;
	// set the address to return
	void * prevTop = process->heap.heap_top;
	// create the pages
	for( ; pages-->0 ; process->heap.heap_top+=PAGE_SIZE )
	{
		// alloc a physical page in mamory
		void * physicalAddress = physical_pageAlloc();
		// map it onto the end of the processes heap
		paging_map( process, process->heap.heap_top, physicalAddress, TRUE );
		// clear it for safety, relativly expensive operation
		memset( process->heap.heap_top, 0x00, PAGE_SIZE );
	}
	// return the start address of the memory we allocated to the heap
	return prevTop;
}

// free a previously allocated item from the kernel heap
void mm_kfree( void * address )
{
	struct MM_HEAPITEM * tmp_item, * item;
	// sanity check
	if( address == NULL )
		return;
	// lock this critical section as we are about to modify the kernels heap
	mutex_lock( &mm_kmallocLock );
	// set the item to remove
	item = (struct MM_HEAPITEM *)( address - sizeof(struct MM_HEAPITEM) );
	// find it
	for( tmp_item=kernel_process.heap.heap_base ; tmp_item!=NULL ; tmp_item=tmp_item->next )
	{
		if( tmp_item == item )
		{
			// free it
			tmp_item->used = FALSE;
			// coalesce any adjacent free items
			for( tmp_item=kernel_process.heap.heap_base ; tmp_item!=NULL ; tmp_item=tmp_item->next )
			{
				while( !tmp_item->used && tmp_item->next!=NULL && !tmp_item->next->used )
				{
					tmp_item->size += sizeof(struct MM_HEAPITEM) + tmp_item->next->size;
					tmp_item->next = tmp_item->next->next;
				}
			}
			// break and return as we are finished
			break;
		}
	}
	// unlock this critical section
	mutex_unlock( &mm_kmallocLock );
}

// allocates an arbiturary size of memory (via first fit) from the kernel heap
void * mm_kmalloc( DWORD size )
{
	struct MM_HEAPITEM * new_item=NULL, * tmp_item;
	int total_size;
	// sanity check
	if( size == 0 )
		return NULL;
	// lock this critical section as we are modifying the kernel heap
	mutex_lock( &mm_kmallocLock );
	// round up by 8 bytes and add header size
	total_size = ( ( size + 7 ) & ~7 ) + sizeof(struct MM_HEAPITEM);
	// if the heap exists
	if( kernel_process.heap.heap_top != NULL )
	{
		// search for first fit
		for( new_item=kernel_process.heap.heap_base ; new_item!=NULL ; new_item=new_item->next )
		{
			if( !new_item->used && (total_size <= new_item->size) )
				break;
		}
	}
	// if we found one
	if( new_item != NULL )
	{
		tmp_item = (struct MM_HEAPITEM *)( (int)new_item + total_size );
		tmp_item->size = new_item->size - total_size;
		tmp_item->used = FALSE;
		tmp_item->next = new_item->next;
	}
	else
	{
		// didnt find a fit so we must increase the heap to fit
		new_item = mm_morecore( &kernel_process, total_size );
		if( new_item == NULL )
		{
			// unlock the critical section
			mutex_unlock( &mm_kmallocLock );
			// return NULL as we are out of physical memory!
			return NULL;
		}
		// create an empty item for the extra space mm_morecore() gave us
		// we can calculate the size because morecore() allocates space that is page aligned
		tmp_item = (struct MM_HEAPITEM *)( (int)new_item + total_size );
		tmp_item->size = PAGE_SIZE - (total_size%PAGE_SIZE ? total_size%PAGE_SIZE : total_size) - sizeof(struct MM_HEAPITEM);
		tmp_item->used = FALSE;
		tmp_item->next = NULL;
	}
	// create the new item
	new_item->size = size;
	new_item->used = TRUE;
	new_item->next = tmp_item;
	// unlock our critical section
	mutex_unlock( &mm_kmallocLock );
	// return the newly allocated memory location
	return (void *)( (int)new_item + sizeof(struct MM_HEAPITEM) );
}
