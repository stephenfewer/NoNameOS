/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/io/io.h>
#include <kernel/fs/dfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
#include <kernel/io/dev/keyboard.h>
#include <kernel/io/dev/floppy.h>
#include <kernel/io/dev/bitbucket.h>
#include <kernel/kernel.h>

int io_add( char * name, struct IO_CALLTABLE * calltable, int type )
{
	if( dfs_add( name, calltable, type ) == NULL )
		return FAIL;
	return SUCCESS;
}

int io_remove( char * name )
{
	return dfs_remove( name );
}

struct IO_HANDLE * io_open( struct DFS_ENTRY * device )
{
	if( device->calltable->open != NULL )
	{
		struct IO_HANDLE * handle;
		handle = (struct IO_HANDLE *)mm_kmalloc( sizeof(struct IO_HANDLE) );
		handle->device = device;
		handle->data_ptr = NULL;
		handle->data_arg = (DWORD)NULL;
		if( handle->device->calltable->open( handle, device->name ) == NULL )
			mm_kfree( handle );
		else
			return handle;
	}
	return NULL;
}

int io_close( struct IO_HANDLE * handle )
{
	if( handle->device->calltable->close != NULL )
	{
		int ret;
		ret = handle->device->calltable->close( handle );
		mm_kfree( handle );
		return ret;
	}
	return FAIL;
}

int io_clone( struct IO_HANDLE * handle, struct IO_HANDLE ** clone )
{
	int ret=FAIL;
	if( handle->device->calltable->clone != NULL )
	{
		struct IO_HANDLE * c = (struct IO_HANDLE *)mm_kmalloc( sizeof(struct IO_HANDLE) );
		if( c == NULL )
			return FAIL;
		c->device = handle->device;
		c->data_ptr = NULL;
		c->data_arg = (DWORD)NULL;
		ret = handle->device->calltable->clone( handle, c );
		if( ret == FAIL )
			mm_kfree( c );
		*clone = c;
	}
	return ret;	
}

int io_read( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	if( handle->device->calltable->read != NULL )
		return handle->device->calltable->read( handle, buffer, size  );
	return FAIL;
}

int io_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size )
{
	if( handle->device->calltable->write != NULL )
		return handle->device->calltable->write( handle, buffer, size );
	return FAIL;
}

int io_seek( struct IO_HANDLE * handle, DWORD offset, BYTE origin )
{
	if( handle->device->calltable->seek != NULL )
		return handle->device->calltable->seek( handle, offset, origin );
	return FAIL;	
}

int io_control( struct IO_HANDLE * handle, DWORD request, DWORD arg )
{
	if( handle->device->calltable->control != NULL )
		return handle->device->calltable->control( handle, request, arg );
	return FAIL;	
}

int io_init( void )
{		
	// init the console driver
	if( console_init() == FAIL )
		return FAIL;
	
	// init the keyboard driver
	if( keyboard_init() == FAIL )
		return FAIL;

	// init the floppy driver
	if( floppy_init() == FAIL )
		return FAIL;

	// init the bit bucket driver
	if( bitbucket_init() == FAIL )
		return FAIL;
	
	return SUCCESS;
}
