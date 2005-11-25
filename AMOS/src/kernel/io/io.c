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

#include <kernel/io/io.h>
#include <kernel/io/device.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
#include <kernel/io/dev/keyboard.h>
#include <kernel/io/dev/floppy.h>
/*
 *  we should be able to do this:
 * 
 *  h = io_open( "/device/console" );
 *  io_write( h, "hello world", 12 );
 *  io_close( h );
 * 
 *  or this:
 * 
 *  h = io_open( "/device/keyboard" );
 *  io_read( h, (char *)&buffer, 1 );
 *  io_close( h );
 * 
 *  or this:
 *
 *  h = io_open( "/device/floppy0" );
 *  io_read( h, (char *)&buffer, 512 );
 *  io_write( h, (char *)&buffer, 512 );
 *  io_close( h );
 * 
 */

struct IO_HANDLE * io_open( char * filename )
{
	struct DEVICE_ENTRY * device;

	device = device_find( filename );
	if( device == NULL )
		return NULL;
		
	if( device->calltable->open != NULL )
	{
		struct IO_HANDLE * handle;
		handle = (struct IO_HANDLE *)mm_malloc( sizeof(struct IO_HANDLE) );
		handle->device = device;
		handle->data = NULL;
		if( handle->device->calltable->open( handle, filename ) == NULL )
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
	return -1;
}

int io_read( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	if( handle->device->calltable->read != NULL )
		return handle->device->calltable->read( handle, buffer, size  );
	return -1;
}

int io_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size )
{
	if( handle->device->calltable->write != NULL )
		return handle->device->calltable->write( handle, buffer, size );
	return -1;
}

int io_seek( struct IO_HANDLE * handle, DWORD offset, BYTE origin )
{
	if( handle->device->calltable->seek != NULL )
		return handle->device->calltable->seek( handle, offset, origin );
	return -1;	
}

void io_init()
{
	// we unlock here as the driver init routines may need to use interrupts to setup
	kernel_unlock();
	
	// init the console driver
	console_init();

	// init the keyboard driver
	keyboard_init();

	// init the floppy driver
	floppy_init();

	// lock again
	kernel_lock();
}
