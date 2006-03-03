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

// This is the Video Graphics Array (VGA) console driver for 80x25 text mode

#include <kernel/kernel.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
#include <kernel/io/io.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/process.h>
#include <kernel/pm/sync/mutex.h>
#include <lib/string.h>

// our currently active console will point to one of the four virtual consoles below
struct CONSOLE ** console0 = NULL;
// our four virtual consoles
struct CONSOLE * console1;
struct CONSOLE * console2;
struct CONSOLE * console3;
struct CONSOLE * console4;

struct CONSOLE_BUFFER * console_bufferTop = NULL;
struct CONSOLE_BUFFER * console_bufferBottom = NULL;
struct MUTEX console_bufferLock;

struct CONSOLE_BUFFER * console_addBuffer( struct CONSOLE_BUFFER * buffer )
{
	if( console_bufferBottom == NULL )
		console_bufferBottom = buffer;
	else
		console_bufferTop->next = buffer;
	console_bufferTop = buffer;
	console_bufferTop->next = NULL;
	return buffer;
}

int console_removeBuffer( struct CONSOLE_BUFFER * buffer )
{
	struct CONSOLE_BUFFER * b;

	if( buffer == console_bufferBottom )
	{
		console_bufferBottom = buffer->next;
	}
	else
	{
		for( b=console_bufferBottom ; b!=NULL ; b=b->next )
		{
			if( b->next == buffer )
			{
				b->next = buffer->next;
				return SUCCESS;
			}
		}
	}
	return FAIL;
}

void console_cls( struct CONSOLE * );

void console_setCursor( struct CONSOLE * console, int x, int y )
{
    WORD index;
    
	console->data->x = x;
    console->data->y = y;
	
	if( console->data->active == TRUE )
	{
		index = (y * CONSOLE_COLUMNS) + x;
		outportb( 0x3D4, 14 );
	    outportb( 0x3D5, index >> 8 );
	    outportb( 0x3D4, 15 );
	    outportb( 0x3D5, index );
	}
}

void console_putChar( struct CONSOLE * console, int x, int y, BYTE c )
{
	int index = ((x + (y*CONSOLE_COLUMNS)) * 2);
	
    console->data->mem[ index ] = c;
    console->data->mem[ index + 1 ] = console->data->attributes;
    
    if( console->data->active == TRUE )
    {
    	BYTE * mem = (BYTE *)KERNEL_VGA_VADDRESS;
    	mem[ index ] = c;
    	mem[ index + 1 ] = console->data->attributes;
    }
}

void console_beep( struct CONSOLE * console )
{

}

BYTE console_getChar( struct CONSOLE * console, int x, int y )
{
    return console->data->mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ];
}

void console_scrollup( struct CONSOLE * console )
{
	int x, y;

    for( y=2 ; y<CONSOLE_ROWS-1; y++ )
    {
		for( x=0; x<CONSOLE_COLUMNS ; x++ )
			console_putChar( console, x, y, console_getChar( console, x, y+1 ) );
    }

    for( x=0 ; x<CONSOLE_COLUMNS ;x++ )
		console_putChar( console, x, CONSOLE_ROWS-1, ' ' );

}

void console_putch( struct CONSOLE * console, BYTE c )
{
	switch( c )
	{
		case '\0':
			break;
		case '\n':
			console_setCursor( console, 0, ++console->data->y );
			break;
		case '\r':
			console_setCursor( console, 0, console->data->y );
			break;
		case '\t':
			console_putch( console, ' ' );
			console_putch( console, ' ' );
			console_putch( console, ' ' );
			console_putch( console, ' ' );
			break;
		case '\a':
			console_beep( console );
			break;
		case '\f':
			console_cls( console );
			break;
		case '\b':
			console_setCursor( console, --console->data->x, console->data->y );
			//console_putch( console, ' ' );
			break;
		default:
			console_putChar( console, console->data->x, console->data->y, c );
			console_setCursor( console, ++console->data->x, console->data->y );
			break;
	}


	if( console->data->x > CONSOLE_COLUMNS-1 )
		console_setCursor( console, 0, ++console->data->y );

	if( console->data->y > CONSOLE_ROWS-1 )
	{
		console_scrollup( console );
		console_setCursor( console, console->data->x, CONSOLE_ROWS-1 );
	}
}

void console_cls( struct CONSOLE * console )
{
	int x, y;
	char * version = AMOS_VERSION_STRING;
	// clear the entire sconsole
    for( x=0 ; x<CONSOLE_COLUMNS ; x++ )
    {
		for( y=0 ; y<CONSOLE_ROWS ; y++ )
			console_putChar( console, x, y, ' ' );
    }
    // display the banner on the top of the console
    console_setCursor( console, 0, 0 );
    
    console->data->attributes = RED | BLACK_BG;
	for( x=0 ; x<strlen( version ) ; x++ )
		console_putch( console, version[x] );
	console->data->attributes = WHITE | BLACK_BG;
	console_setCursor( console, 0, 1 );
	
    for( x=0 ; x<CONSOLE_COLUMNS-6 ; x++ )
	    console_putch( console, '-' );
	    
	console_putch( console, '[' );
	console->data->attributes = RED | BLACK_BG;
	console_putch( console, (BYTE)(console->data->number+'0') );
	console->data->attributes = WHITE | BLACK_BG;
	console_putch( console, ']' );
	
    for( x=0 ; x<3 ; x++ )
	    console_putch( console, '-' );
	
	console_setCursor( console, 0, 2 );
}

int console_activate( DWORD number )
{
	struct CONSOLE * console;
	
	if( number == CONSOLE_1 )
		console = console1;
	else if( number == CONSOLE_2 )
		console = console2;
	else if( number == CONSOLE_3 )
		console = console3;
	else if( number == CONSOLE_4 )
		console = console4;
	else
		return FAIL;

	// set all consoles as not active, theirs a bug if we dont
	console1->data->active = FALSE;
	console2->data->active = FALSE;
	console3->data->active = FALSE;
	console4->data->active = FALSE;
	
	if( console0 != NULL )
	{
		// dont do anything if we are trying to set an allready active console active
		if( (*console0)->data->number == console->data->number )
			return FAIL;
		// set the current virtual console not active
		(*console0)->data->active = FALSE;
	}
	// set the one we are changeing to as active
	console->data->active = TRUE;
	// copy in the new contents
	memcpy( (BYTE *)KERNEL_VGA_VADDRESS, console->data->mem, CONSOLE_COLUMNS*CONSOLE_ROWS*2 );
	// update the cursor to the correct new position
	console_setCursor( console, console->data->x, console->data->y );
	// set the currenty active virtual console to the one we just changed to
	*console0 = console;
	// return success
	return SUCCESS;
}

struct CONSOLE * console_create( char * name, int number )
{
	struct CONSOLE * console;
	// alloc memeory for the structure
	console = (struct CONSOLE *)mm_malloc( sizeof(struct CONSOLE) );
	console->data = (struct CONSOLE_DATA *)mm_malloc( sizeof(struct CONSOLE_DATA) );
	// set the virtual console name
	console->data->name = name;
	// set the virtual console number
	console->data->number = number;
	// alloc some memory for the virtual console contents
	console->data->mem = (BYTE *)mm_malloc( (CONSOLE_COLUMNS*CONSOLE_ROWS)*2 );
	// default to not being an active virtual console
	console->data->active = FALSE;
	// default to not echoing input to screen
	console->data->echo = FALSE;
	// set the default attributes
	console->data->attributes = WHITE | BLACK_BG;
	// we dont create a buffer structure, we do this during calls to console_open()
	console->buffer = NULL;
	// clear the new virtual console
	console_cls( console );
	// return it to caller
	return console;
}

struct IO_HANDLE * console_open( struct IO_HANDLE * handle, char * filename )
{
	struct CONSOLE * console;

	// associate the correct virtual console with the handle
	if( strcmp( filename, "console0" ) == 0 )
	{
		handle->data_ptr = console0;
		handle->data_arg = CONSOLE_PTRPTR;
	}
	else
	{
		console = (struct CONSOLE *)mm_malloc( sizeof(struct CONSOLE) );
		
		if( strcmp( filename, console1->data->name ) == 0 )
			console->data = console1->data;
		else if( strcmp( filename, console2->data->name ) == 0 )
			console->data = console2->data;
		else if( strcmp( filename, console3->data->name ) == 0 )
			console->data = console3->data;
		else if( strcmp( filename, console4->data->name ) == 0 )
			console->data = console4->data;
		else {
			mm_free( console );
			return NULL;
		}

		console->buffer = (struct CONSOLE_BUFFER *)mm_malloc( sizeof(struct CONSOLE_BUFFER) );
		memset( console->buffer, 0x00, sizeof(struct CONSOLE_BUFFER) );
		
		console->buffer->number = console->data->number;
		
		console_addBuffer( console->buffer );
		
		handle->data_arg = CONSOLE_PTR;
		handle->data_ptr = console;
	}
	// return the virtual console handle
	return handle;
}

int console_close( struct IO_HANDLE * handle )
{
	struct CONSOLE * console;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_PTRPTR )
		console = *(struct CONSOLE **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_PTR )
		console = (struct CONSOLE *)handle->data_ptr;
	else
		return FAIL;

	handle->data_ptr = NULL;
	handle->data_arg = 0;
	
	if( console->buffer != NULL )
	{
		console_removeBuffer( console->buffer );	
		mm_free( console->buffer );
	}
	
	mm_free( console );
	
	return SUCCESS;
}

int console_read( struct IO_HANDLE * handle, BYTE * ubuff, DWORD size  )
{
	struct CONSOLE * console;
	struct CONSOLE_BUFFER * buffer;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_PTRPTR )
		console = *(struct CONSOLE **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_PTR )
		console = (struct CONSOLE *)handle->data_ptr;
	else
		return FAIL;

	buffer = console->buffer;
	if( buffer == NULL )
		return FAIL;

	if(	buffer->in_kbuff == NULL )
	{
		char c;
		buffer->in_buffIndex = 0;
		buffer->in_buffSize = size;
		
		if( size == 1 )
			buffer->in_kbuff = (BYTE *)&c;
		else
			buffer->in_kbuff = (BYTE *)mm_malloc( size );
		
		while( buffer->in_buffIndex < buffer->in_buffSize  )
		{
			if( buffer->in_break == TRUE )
				break;
			process_yield();
		}
		
		if( buffer->in_buffIndex > size )
			kernel_printf( "buffer->in_buffIndex > size\n" );
		
		memcpy( ubuff, buffer->in_kbuff, buffer->in_buffIndex );
		
		if( size > 1 )
			mm_free( buffer->in_kbuff );

		buffer->in_kbuff = NULL;	
		buffer->in_break = FALSE;
		buffer->in_breakByte = 0x00;
		
		// return bytes read to caller
		return buffer->in_buffIndex;
	}
	
	return FAIL;
}

int console_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	int i=0;
	struct CONSOLE * console;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_PTRPTR )
		console = *(struct CONSOLE **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_PTR )
		console = (struct CONSOLE *)handle->data_ptr;
	else
		return FAIL;
	// print all the charachters in the buffer to the virtual console
	while( i < size )
		console_putch( console, buffer[i++] );
	// return the number of bytes written
	return i;
}

int console_putchBuffer( int number, BYTE byte )
{
	struct CONSOLE_BUFFER * buffer;

	mutex_lock( &console_bufferLock );
	
	for( buffer=console_bufferBottom ; buffer!=NULL ; buffer=buffer->next )
	{
		if( buffer->number == number )
		{
			if( buffer->in_breakByte != 0x00 )
			{
				if( buffer->in_breakByte == byte )
				{
					buffer->in_break = TRUE;
					continue;
				}
			}
			if( buffer->in_buffIndex < buffer->in_buffSize )
				buffer->in_kbuff[ buffer->in_buffIndex++ ] = byte;				
		}
	}
	
	mutex_unlock( &console_bufferLock );
	
	return SUCCESS;	
}

int console_control( struct IO_HANDLE * handle, DWORD request, DWORD arg )
{
	struct CONSOLE * console;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_PTRPTR )
		console = *(struct CONSOLE **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_PTR )
		console = (struct CONSOLE *)handle->data_ptr;
	else
		return FAIL;
	// switch the request
	switch( request )
	{
		case CONSOLE_SETECHO:
			if( (BYTE)arg == TRUE )
				console->data->echo = TRUE;
			else
				console->data->echo = FALSE;
			return SUCCESS;
		// we want to set this virtual console as active
		case CONSOLE_SETACTIVE:
			return console_activate( arg );
		case CONSOLE_SENDCHAR:
			if( console->data->echo )
				console_putch( console, arg );
			console_putchBuffer( console->data->number, (BYTE)arg );
			return SUCCESS;
		case CONSOLE_SETBREAK:
			console->buffer->in_breakByte = (BYTE)arg;
			return SUCCESS;
	}
	// return fail
	return FAIL;
}

int console_init( void )
{
    struct IO_CALLTABLE * calltable;
	// setup the calltable for this driver
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open    = console_open;
	calltable->close   = console_close;
	calltable->read    = console_read;
	calltable->write   = console_write;
	calltable->seek    = NULL;
	calltable->control = console_control;
	// init the buffer lock
	mutex_init( &console_bufferLock );
	// create the first virtual console
	console1 = console_create( "console1", CONSOLE_1 );
	io_add( console1->data->name, calltable, IO_CHAR );
	// create the second
	console2 = console_create( "console2", CONSOLE_2 );
	io_add( console2->data->name, calltable, IO_CHAR );
	// create the third
	console3 = console_create( "console3", CONSOLE_3 );
	io_add( console3->data->name, calltable, IO_CHAR );
	// create the fourth
	console4 = console_create( "console4", CONSOLE_4 );
	io_add( console4->data->name, calltable, IO_CHAR );	
	// set the fist one active
	console_activate( CONSOLE_1 );
	// add the currently active console
	io_add( "console0", calltable, IO_CHAR );
	return SUCCESS;
}
