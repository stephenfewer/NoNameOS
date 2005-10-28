#include <kernel/io/io.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
/*
 *  we should be able to do this:
 * 
 *  h = io_open( "/dev/console" );
 *  io_write( h, "hello world", 12 );
 *  io_close( h );
 * 
 *  or this:
 * 
 *  h = io_open( "/dev/keyboard" );
 *  io_read( h, (char *)&buffer, 1 );
 *  io_close( h );
 * 
 *  or this:
 *
 *  h = io_open( "/dev/fd0" );
 *  io_read( h, (char *)&buffer, 512 );
 *  io_write( h, (char *)&buffer, 512 );
 *  io_close( h );
 * 
 */
 
// this is our virtual function table to the io calls
struct IO_CALLTABLE io_calltable[IO_TOTALDEVICES];

int strcmp( char * src, char * dest )
{
	return 1;
}

enum DEVICE_TYPE io_getDevice( char * filename )
{
	if( strcmp( filename, "/dev/console" ) )
		return CONSOLE;
	else if( strcmp( filename, "/dev/keyboard" ) )
		return KEYBOARD;

	return UNKNOWN;
}

struct DEVICE_HANDLE * io_open( char * filename )
{
	enum DEVICE_TYPE type;
	
	type = io_getDevice( filename );
	if( type == UNKNOWN )
		return NULL;
		
	if( io_calltable[type].open != NULL )
		return io_calltable[type].open( filename );
	
	return NULL;
}

int io_close( struct DEVICE_HANDLE * handle )
{
	if( io_calltable[handle->type].close != NULL )
		return io_calltable[handle->type].close( handle );
	return -1;
}

int io_read( struct DEVICE_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	if( io_calltable[handle->type].read != NULL )
		return io_calltable[handle->type].read( handle, buffer, size  );
	return -1;
}

int io_write( struct DEVICE_HANDLE * handle, BYTE * buffer, DWORD size )
{
	if( io_calltable[handle->type].write != NULL )
		return io_calltable[handle->type].write( handle, buffer, size );
	return -1;
}

void io_init()
{
	// clear the call table
	mm_memset( (BYTE *)&io_calltable, 0x00, (sizeof(struct IO_CALLTABLE)*IO_TOTALDEVICES) );
	
	// setup the console driver
	io_calltable[CONSOLE].open = console_open;
	io_calltable[CONSOLE].close = console_close;
	io_calltable[CONSOLE].read = console_read;
	io_calltable[CONSOLE].write = console_write;
	// init the driver
	console_init();
}
