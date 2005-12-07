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
#include <kernel/io/dev/bitbucket.h>
#include <kernel/kernel.h>

// the global handle for the kernels standard output
struct IO_HANDLE * io_kout;

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
		handle->data_ptr = NULL;
		handle->data_arg = (DWORD)NULL;
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
	return IO_FAIL;
}

int io_read( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	if( handle->device->calltable->read != NULL )
		return handle->device->calltable->read( handle, buffer, size  );
	return IO_FAIL;
}

int io_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size )
{
	if( handle->device->calltable->write != NULL )
		return handle->device->calltable->write( handle, buffer, size );
	return IO_FAIL;
}

int io_seek( struct IO_HANDLE * handle, DWORD offset, BYTE origin )
{
	if( handle->device->calltable->seek != NULL )
		return handle->device->calltable->seek( handle, offset, origin );
	return IO_FAIL;	
}

int io_control( struct IO_HANDLE * handle, DWORD request, DWORD arg )
{
	if( handle->device->calltable->control != NULL )
		return handle->device->calltable->control( handle, request, arg );
	return IO_FAIL;	
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

	// init the bit bucket driver
	bitbucket_init();
	
	// open the standard kernel output
	io_kout = io_open( "/device/console1" );
	
	// lock again
	kernel_lock();
}
