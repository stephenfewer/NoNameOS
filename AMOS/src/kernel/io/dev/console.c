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
#include <kernel/kprintf.h>

BYTE * console_mem;

int console_x, console_y;

BYTE console_attrib;

struct IO_HANDLE * console_open( struct IO_HANDLE * handle, char * filename )
{
	// we could use the handle to store the virtual console number if we set them up	
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
		console_putch( buffer[i++] );
	return 0;
}

void console_putch( BYTE c )
{
	switch( c )
	{
		case '\n':
			console_setCursor( 0, ++console_y );
			break;
		case '\r':
			console_setCursor( 0, console_y );
			break;
		case '\t':
			console_putch( ' ' );
			console_putch( ' ' );
			console_putch( ' ' );
			console_putch( ' ' );
			break;
		case '\a':
			console_beep();
			break;
		case '\f':
			console_cls();
			break;
		default:
			console_putChar( console_x, console_y, c, console_attrib );
			console_setCursor( ++console_x, console_y );
			break;
	}


	if( console_x > CONSOLE_COLUMNS-1 )
		console_setCursor( 0, ++console_y );

	if( console_y > CONSOLE_ROWS-1 )
	{
		console_scrollup();
		console_setCursor( console_x, CONSOLE_ROWS-1 );
	}
}

void console_putChar( int x, int y, BYTE c, BYTE attrib )
{
    console_mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ] = c;
    console_mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ] = attrib;
}

BYTE console_getChar( int x, int y )
{
    return console_mem[ (x + (y*CONSOLE_COLUMNS)) * 2 ];
}

void console_setAttrib( BYTE attrib )
{
    console_attrib = attrib;
}

BYTE console_getAttrib( int x, int y )
{
    return console_mem[ ((x + (y*CONSOLE_COLUMNS)) * 2) + 1 ];
}

void console_scrollup( void )
{
	int i, j;

    for( j=2 ; j<CONSOLE_ROWS-1; j++ )
    {
		for( i=0; i<CONSOLE_COLUMNS ; i++ )
			console_putChar( i, j, console_getChar( i, j+1 ), console_getAttrib( i, j+1 ) );
    }

    for( i=0 ; i<CONSOLE_COLUMNS ; i++ )
		console_putChar( i, CONSOLE_ROWS-1, ' ', console_attrib );

}

void console_setCursor( int x, int y )
{
    short index = (y * CONSOLE_COLUMNS) + x;
    
	console_x = x;
    console_y = y;

	outportb( 0x3D4, 14 );
    outportb( 0x3D5, index >> 8 );
    outportb( 0x3D4, 15 );
    outportb( 0x3D5, index );
}

void console_cls( void )
{
	int i, j;
	// clear the entire sconsole
    for( i=0 ; i<CONSOLE_COLUMNS ; i++ )
    {
		for( j=0 ; j<CONSOLE_ROWS ; j++ )
			console_putChar( i, j, ' ', console_attrib );
    }
    // display the banner on the top of the console
    console_setCursor( 0, 0 );
    for( i=0 ; i<CONSOLE_COLUMNS ; i++ )
	    console_putch( ' ' );
	console_setCursor( 0, 0 );
    kprintf( "AMOS %d.%d.%d\n", AMOS_MAJOR_VERSION, AMOS_MINOR_VERSION, AMOS_PATCH_VERSION );
    for( i=0 ; i<CONSOLE_COLUMNS ; i++ )
	    console_putch( '-' );
	console_setCursor( 0, 2 );
}

void console_beep( void )
{

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
	
	device_add( "/device/console0", calltable );
	
    console_mem = (BYTE *)VIDEOMEM_BASE;

    console_setAttrib( GREEN | RED_BG );
    
    console_cls();
}

