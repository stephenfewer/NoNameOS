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

#include <kernel/io/device.h>
#include <kernel/mm/mm.h>
#include <kernel/io/io.h>
#include <kernel/lib/string.h>

struct DEVICE_ENTRY * device_top = NULL;
struct DEVICE_ENTRY * device_bottom = NULL;

struct DEVICE_ENTRY * device_add( char * name, struct IO_CALLTABLE * calltable )
{
	struct DEVICE_ENTRY * device;
	
	device = (struct DEVICE_ENTRY *)mm_malloc( sizeof(struct DEVICE_ENTRY) );
	
	if( device_bottom == NULL )
	{
		device_bottom = device_top = device ;
	}
	else
	{
		device_top->next = device;
		device_top = device;
	}
	
	device->next = NULL;
	
	device->calltable = calltable;
	
	device->name = name;
	
	return device;
}

void device_remove( struct DEVICE_ENTRY * device )
{
	// remove from linked list
	
	// free all memory allocated
	mm_free( device->calltable );
	mm_free( device->name );
	mm_free( device );
}

struct DEVICE_ENTRY * device_find( char * name )
{
	struct DEVICE_ENTRY * device;
	
	for( device=device_bottom ; device!=NULL ; device=device->next )
	{
		if( strcmp( device->name, name ) == 0 )
			break;
	}
	
	return device;
}
