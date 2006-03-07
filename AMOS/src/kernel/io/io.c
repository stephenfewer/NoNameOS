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
		handle = (struct IO_HANDLE *)mm_malloc( sizeof(struct IO_HANDLE) );
		handle->device = device;
		handle->data_ptr = NULL;
		handle->data_arg = (DWORD)NULL;
		if( handle->device->calltable->open( handle, device->name ) == NULL )
			mm_free( handle );
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
		mm_free( handle );
		return ret;
	}
	return FAIL;
}

int io_clone( struct IO_HANDLE * handle, struct IO_HANDLE * clone )
{
	int ret=FAIL;
	if( handle->device->calltable->clone != NULL )
	{
		clone = (struct IO_HANDLE *)mm_malloc( sizeof(struct IO_HANDLE) );
		clone->device = handle->device;
		clone->data_ptr = NULL;
		clone->data_arg = (DWORD)NULL;
		ret = handle->device->calltable->clone( handle, clone );
		if( ret == FAIL )
			mm_free( clone );
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
	console_init();

	// init the keyboard driver
	keyboard_init();

	// init the floppy driver
	floppy_init();

	// init the bit bucket driver
	bitbucket_init();
	
	return SUCCESS;
}
