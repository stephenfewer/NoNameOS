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

#include <kernel/kernel.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
#include <kernel/io/device.h>
#include <kernel/io/io.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>
#include <kernel/isr.h>

struct CONSOLE_DATA * console0;
struct CONSOLE_DATA * console1;

struct CONSOLE_DATA * console_active = NULL;

void console_cls( struct CONSOLE_DATA * );

void console_setCursor( struct CONSOLE_DATA * console, int x, int y )
{
    short index = (y * CONSOLE_COLUMNS) + x;
    
	console->x = x;
    console->y = y;

	outportb( 0x3D4, 14 );
    outportb( 0x3D5, index >> 8 );
    outportb( 0x3D4, 15 );
    outportb( 0x3D5, index );
}

void console_putChar( struct CONSOLE_DATA * console, int x, int y, BYTE c )
{
    console->mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ] = c;
    console->mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ] = GREEN | RED_BG;
    
    if( console->active )
    {
    	BYTE * mem = (BYTE *)VIDEOMEM_BASE;
    	mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ] = c;
    	mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ] = GREEN | RED_BG;
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
    
	for( x=0 ; x<strlen( version ) ; x++ )
		console_putch( console, version[x] );
	console_setCursor( console, 0, 1 );
	
    for( x=0 ; x<CONSOLE_COLUMNS-6 ; x++ )
	    console_putch( console, '-' );
	    
	console_putch( console, '[' );
	
	console_putch( console, (BYTE)(console->number+'0') );
	
	console_putch( console, ']' );
	
    for( x=0 ; x<3 ; x++ )
	    console_putch( console, '-' );
	
	console_setCursor( console, 0, 2 );
}

/*
BYTE console_getAttrib( struct CONSOLE_DATA * console, int x, int y )
{
    return console->mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ];
}
*/

void console_setActive(  struct CONSOLE_DATA * console )
{
	BYTE * mem = (BYTE *)VIDEOMEM_BASE;
	
	if( console_active == console )
		return;
		
	if( console_active != NULL )
		console_active->active = FALSE;
	
	console->active = TRUE;
	
	memcpy( mem, console->mem, CONSOLE_COLUMNS*CONSOLE_ROWS*2 );
	
	console_setCursor( console, console->x, console->y );
	
	console_active = console;
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
	// clear the new virtual console
	console_cls( console );
	// return it to caller
	return console;
}

struct IO_HANDLE * console_open( struct IO_HANDLE * handle, char * filename )
{
	// associate the correct virtual console with the handle
	if( strcmp( filename, console0->name ) == 0 )
		handle->data = console0;
	else if( strcmp( filename, console1->name ) == 0 )
		handle->data = console1;
	else
		return NULL;
	// return the virtual console handle
	return handle;
}

int console_close( struct IO_HANDLE * handle )
{
	return 0;
}

int console_write( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	int i=0;
	while( i < size )
		console_putch( (struct CONSOLE_DATA *)handle->data, buffer[i++] );
	return i;
}

int console_control( struct IO_HANDLE * handle, DWORD request, DWORD arg )
{
	struct CONSOLE_DATA * console;
	
	console = (struct CONSOLE_DATA *)handle->data;
	
	switch( request )
	{
		case CONSOLE_SETACTIVE:
			console_setActive( console );
			break;
	}
	return -1;
}

void console_init( void )
{
    struct IO_CALLTABLE * calltable;

	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = console_open;
	calltable->close = console_close;
	calltable->read = NULL;
	calltable->write = console_write;
	calltable->seek = NULL;
	calltable->control = console_control;

	// create the first virtual console
	console0 = console_create( "/device/console0", 0 );
	device_add( console0->name, calltable );

	// create the second
	console1 = console_create( "/device/console1", 1 );
	device_add( console1->name, calltable );

	// set the fist one active
	console_setActive( console0 );
}

