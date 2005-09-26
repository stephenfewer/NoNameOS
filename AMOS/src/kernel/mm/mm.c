#include <kernel/mm/physical.h>
#include <kernel/mm/paging.h>
#include <kernel/console.h>
/*
#define KERNEL_HEAP_VADDRESS	0xD0000000

struct heap_item
{
	struct heap_item * next;
	int size;	
};

struct heap_item * heap_firstItem = NULL;

struct heap_item * heap_nextItem = NULL;
*/
void mm_init( DWORD mem_upper )
{	
	physical_init( mem_upper );

	paging_init();
	
	// setup the kernel heap...
	//paging_setPageTableEntry( KERNEL_HEAP_VADDRESS, physical_pageAlloc() );
}
/*
void * mm_malloc( int size )
{
	struct heap_item * newItem;
	
	if( size == 0 )
		return NULL;
	
	if( heap_firstItem == NULL )
	{
		heap_firstItem = (struct heap_item *)KERNEL_HEAP_VADDRESS;
		heap_firstItem->size = size;
		
		heap_nextItem = (struct heap_item *)( sizeof(struct heap_item) + heap_firstItem->size );
		
		heap_firstItem->next = heap_nextItem;
		
		newItem = heap_lastItem;
	}
	
	return (void *)( newItem + sizeof(struct heap_item) );
}
*/

