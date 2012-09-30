/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/libc/stdlib.h>
#include <lib/amos.h>

static BOOL malloc_first_call = TRUE;

void * malloc( DWORD size )
{
	struct HEAPITEM * new_item=NULL, * tmp_item;
	int total_size;
	// sanity check
	if( size == 0 )
		return NULL;
	// round up by 8 bytes and add header size
	total_size = ( ( size + 7 ) & ~7 ) + sizeof(struct HEAPITEM);
	// on the first call to malloc we need to call morecore() before we can search the heap
	if( !malloc_first_call )
	{
		// search for first fit
		for( new_item=HEAP_ADDRESS ; new_item!=NULL ; new_item=new_item->next )
		{
			if( !new_item->used && (total_size <= new_item->size) )
				break;
		}
	} else
		malloc_first_call = FALSE;
	// if we found one
	if( new_item != NULL )
	{
		tmp_item = (struct HEAPITEM *)( (int)new_item + total_size );
		tmp_item->size = new_item->size - total_size;
		tmp_item->used = FALSE;
		tmp_item->next = new_item->next;
	}
	else
	{
		// didnt find a fit so we must increase the heap to fit
		new_item = morecore( total_size );
		if( new_item == NULL ) // return NULL if we are out of physical memory!
			return NULL;
		// create an empty item for the extra space morecore() gave us
		// we can calculate the size because morecore() allocates space that is page aligned
		tmp_item = (struct HEAPITEM *)( (int)new_item + total_size );
		tmp_item->size = PAGESIZE - (total_size%PAGESIZE ? total_size%PAGESIZE : total_size) - sizeof(struct HEAPITEM);
		tmp_item->used = FALSE;
		tmp_item->next = NULL;
	}
	// create the new item
	new_item->size = size;
	new_item->used = TRUE;
	new_item->next = tmp_item;
	// return the newly allocated memory location
	return (void *)( (int)new_item + sizeof(struct HEAPITEM) );
}

void free( void * address )
{
	struct HEAPITEM * tmp_item, * item;
	// sanity check
	if( address == NULL )
		return;
	// set the item to remove
	item = (struct HEAPITEM *)( address - sizeof(struct HEAPITEM) );
	// find it
	for( tmp_item=HEAP_ADDRESS ; tmp_item!=NULL ; tmp_item=tmp_item->next )
	{
		if( tmp_item == item )
		{
			// free it
			tmp_item->used = FALSE;
			// coalesce any adjacent free items
			for( tmp_item=HEAP_ADDRESS ; tmp_item!=NULL ; tmp_item=tmp_item->next )
			{
				while( !tmp_item->used && tmp_item->next!=NULL && !tmp_item->next->used )
				{
					tmp_item->size += sizeof(struct HEAPITEM) + tmp_item->next->size;
					tmp_item->next = tmp_item->next->next;
				}
			}
			// break and return as we are finished
			break;
		}
	}
}

int atoi( char * s )
{
	long int v=0;
	int sign=1;
	
	while( *s == ' '  ||  (unsigned int)(*s - 9) < 5u ) s++;
	
	switch( *s )
	{
		case '-': sign=-1;
		case '+': ++s;
	}
	while( (unsigned int) (*s - '0') < 10u )
	{
		v=v*10+*s-'0'; ++s;
	}
	return sign==-1?-v:v;
}
