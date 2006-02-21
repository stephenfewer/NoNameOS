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
#include <kernel/lib/string.h>
#include <kernel/pm/scheduler.h>

// our currently active console will point to one of the four virtual consoles below
struct CONSOLE_DATA ** console0 = NULL;
// our four virtual consoles
struct CONSOLE_DATA * console1;
struct CONSOLE_DATA * console2;
struct CONSOLE_DATA * console3;
struct CONSOLE_DATA * console4;

void console_cls( struct CONSOLE_DATA * );

void console_setCursor( struct CONSOLE_DATA * console, int x, int y )
{
    WORD index;
    
	console->x = x;
    console->y = y;
	
	if( console->active )
	{
		index = (y * CONSOLE_COLUMNS) + x;
		outportb( 0x3D4, 14 );
	    outportb( 0x3D5, index >> 8 );
	    outportb( 0x3D4, 15 );
	    outportb( 0x3D5, index );
	}
}

void console_putChar( struct CONSOLE_DATA * console, int x, int y, BYTE c )
{
    console->mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ] = c;
    console->mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ] = console->attributes;
    
    if( console->active )
    {
    	BYTE * mem = (BYTE *)VIDEOMEM_BASE;
    	mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ] = c;
    	mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ] = console->attributes;
    }
}

void console_beep( struct CONSOLE_DATA * console )
{

}

BYTE console_getChar( struct CONSOLE_DATA * console, int x, int y )
{
    return console->mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ];
}

void console_scrollup( struct CONSOLE_DATA * console )
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

void console_putch( struct CONSOLE_DATA * console, BYTE c )
{
	switch( c )
	{
		case '\0':
			break;
		case '\n':
			console_setCursor( console, 0, ++console->y );
			break;
		case '\r':
			console_setCursor( console, 0, console->y );
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
			console_setCursor( console, --console->x, console->y );
			//console_putch( console, ' ' );
			break;
		default:
			console_putChar( console, console->x, console->y, c );
			console_setCursor( console, ++console->x, console->y );
			break;
	}


	if( console->x > CONSOLE_COLUMNS-1 )
		console_setCursor( console, 0, ++console->y );

	if( console->y > CONSOLE_ROWS-1 )
	{
		console_scrollup( console );
		console_setCursor( console, console->x, CONSOLE_ROWS-1 );
	}
}

void console_cls( struct CONSOLE_DATA * console )
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
    
    console->attributes = RED | BLACK_BG;
	for( x=0 ; x<strlen( version ) ; x++ )
		console_putch( console, version[x] );
	console->attributes = WHITE | BLACK_BG;
	console_setCursor( console, 0, 1 );
	
    for( x=0 ; x<CONSOLE_COLUMNS-6 ; x++ )
	    console_putch( console, '-' );
	    
	console_putch( console, '[' );
	console->attributes = RED | BLACK_BG;
	console_putch( console, (BYTE)(console->number+'0') );
	console->attributes = WHITE | BLACK_BG;
	console_putch( console, ']' );
	
    for( x=0 ; x<3 ; x++ )
	    console_putch( console, '-' );
	
	console_setCursor( console, 0, 2 );
}

int console_activate(  struct CONSOLE_DATA * console )
{
	if( console0 != NULL )
	{
		// dont do anything if we are trying to set an allready active
		// console active
		if( (*console0)->number == console->number )
			return FAIL;
		// set the current virtual console not active
		(*console0)->active = FALSE;
	}
	// set the one we are changeing to as active
	console->active = TRUE;
	// copy in the new contents
	memcpy( (BYTE *)VIDEOMEM_BASE, console->mem, CONSOLE_COLUMNS*CONSOLE_ROWS*2 );
	// update the cursor to the correct new position
	console_setCursor( console, console->x, console->y );
	// set the currenty active virtual console to the one we just changed to
	*console0 = console;
	// return success
	return SUCCESS;
}

struct CONSOLE_DATA * console_create( char * name, int number )
{
	struct CONSOLE_DATA * console;
	// alloc memeory for the structure
	console = (struct CONSOLE_DATA *)mm_malloc( sizeof(struct CONSOLE_DATA) );
	// set the virtual console name
	console->name = name;
	// set the virtual console number
	console->number = number;
	// alloc some memory for the virtual console contents
	console->mem = (BYTE *)mm_malloc( (CONSOLE_COLUMNS*CONSOLE_ROWS)*2 );
	// default to not being an active virtual console
	console->active = FALSE;
	// default to not echoing input to screen
	console->echo = FALSE;
	// set the break flag to false;
	console->in_break = FALSE;
	// set no break byte
	console->in_breakByte = 0x00;
	// set the default attributes
	console->attributes = WHITE | BLACK_BG;
	// clear the new virtual console
	console_cls( console );
	// return it to caller
	return console;
}

struct IO_HANDLE * console_open( struct IO_HANDLE * handle, char * filename )
{
	handle->data_arg = CONSOLE_DATA_PTR;
	// associate the correct virtual console with the handle
	if( strcmp( filename, "console0" ) == 0 ) {
		handle->data_ptr = console0;
		handle->data_arg = CONSOLE_DATA_PTRPTR;
	} else if( strcmp( filename, console1->name ) == 0 )
		handle->data_ptr = console1;
	else if( strcmp( filename, console2->name ) == 0 )
		handle->data_ptr = console2;
	else if( strcmp( filename, console3->name ) == 0 )
		handle->data_ptr = console3;
	else if( strcmp( filename, console4->name ) == 0 )
		handle->data_ptr = console4;
	else
		return NULL;
	// return the virtual console handle
	return handle;
}

int console_close( struct IO_HANDLE * handle )
{
	return SUCCESS;
}

int console_read( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	struct CONSOLE_DATA * console;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_DATA_PTRPTR )
		console = *(struct CONSOLE_DATA **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_DATA_PTR )
		console = (struct CONSOLE_DATA *)handle->data_ptr;
	else
		return FAIL;
	
	if(	console->in_buff == NULL )
	{
		console->in_buffIndex = 0;
		console->in_buffSize = size;
		console->in_buff = buffer;
		
		while( console->in_buffIndex < console->in_buffSize  )
		{
			if( console->in_break == TRUE )
				break;
		}
		
		console->in_buff = NULL;
		console->in_break = FALSE;
		if( console->in_breakByte != 0x00 )
			console->in_breakByte = 0x00;
		
		return console->in_buffIndex;
	}
	
	return FAIL;
}

int console_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	int i=0;
	struct CONSOLE_DATA * console;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_DATA_PTRPTR )
		console = *(struct CONSOLE_DATA **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_DATA_PTR )
		console = (struct CONSOLE_DATA *)handle->data_ptr;
	else
		return FAIL;
	// print all the charachters in the buffer to the virtual console
	while( i < size )
		console_putch( console, buffer[i++] );
	// return the number of bytes written
	return i;
}

int console_control( struct IO_HANDLE * handle, DWORD request, DWORD arg )
{
	struct CONSOLE_DATA * console;
	// get the virtual console we are operating on
	if( handle->data_arg == CONSOLE_DATA_PTRPTR )
		console = *(struct CONSOLE_DATA **)handle->data_ptr;
	else if( handle->data_arg == CONSOLE_DATA_PTR )
		console = (struct CONSOLE_DATA *)handle->data_ptr;
	else
		return FAIL;
	// switch the request
	switch( request )
	{
		case CONSOLE_SETECHO:
			if( (BYTE)arg == TRUE )
				console->echo = TRUE;
			else
				console->echo = FALSE;
			return SUCCESS;
			break;
		// we want to set this virtual console as active
		case CONSOLE_SETACTIVE:
			return console_activate( console );
		case CONSOLE_SENDCHAR:
			if( console->echo )
				console_putch( console, arg );
			if( console->in_buff != NULL )
			{
				if( console->in_breakByte != 0x00 )
				{
					if( console->in_breakByte == (BYTE)arg )
					{
						console->in_break = TRUE;
						break;
					}
				}
				if( console->in_buffIndex < console->in_buffSize )
					console->in_buff[ console->in_buffIndex++ ] = (BYTE)arg;	
			}
			return SUCCESS;
		case CONSOLE_SETBREAK:
			console->in_breakByte = (BYTE)arg;
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
	calltable->open = console_open;
	calltable->close = console_close;
	calltable->read = console_read;
	calltable->write = console_write;
	calltable->seek = NULL;
	calltable->control = console_control;

	// create the first virtual console
	console1 = console_create( "console1", 1 );
	io_add( console1->name, calltable, IO_CHAR );
	// create the second
	console2 = console_create( "console2", 2 );
	io_add( console2->name, calltable, IO_CHAR );
	// create the third
	console3 = console_create( "console3", 3 );
	io_add( console3->name, calltable, IO_CHAR );
	// create the fourth
	console4 = console_create( "console4", 4 );
	io_add( console4->name, calltable, IO_CHAR );	
	// set the fist one active
	console_activate( console1 );
	// add the currently active console
	io_add( "console0", calltable, IO_CHAR );
	
	return SUCCESS;
}
