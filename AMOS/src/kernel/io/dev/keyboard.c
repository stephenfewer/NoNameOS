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

#include <kernel/io/dev/keyboard.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
#include <kernel/io/io.h>
#include <kernel/fs/vfs.h>

struct VFS_HANDLE * keyboard_output;

unsigned char keymap[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0,				/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
	'-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
	'+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};	

struct IO_HANDLE * keyboard_open( struct IO_HANDLE * handle, char * filename )
{
	return handle;
}

int keyboard_close( struct IO_HANDLE * handle )
{
	return SUCCESS;
}

DWORD keyboard_handler( struct PROCESS_INFO * process )
{
	BYTE scancode;
	
	scancode = inportb( KEYBOARD_DATAREG );
	
	if( scancode & 0x80 )
	{
		// key release
	}
	else
	{
		if( scancode >= 0x3B && scancode <= 0x3E )
		{
			struct VFS_HANDLE * console;
			char * name = NULL;
			
			if( scancode == 0x3B )
				name =  "/device/console1";
			else if( scancode == 0x3C )
				name =  "/device/console2";
			else if( scancode == 0x3D )
				name =  "/device/console3";
			else if( scancode == 0x3E )
				name =  "/device/console4";
				
			console = vfs_open( name, VFS_MODE_WRITE );
			if( console != NULL )
			{
				vfs_control( console, CONSOLE_SETACTIVE, 0L );
				vfs_close( console );
			}
		} else {
			if( keyboard_output != NULL )
				vfs_control( keyboard_output, CONSOLE_SENDCHAR, keymap[scancode] );
		}

	}
	
	return FALSE;
}

int keyboard_init( void )
{
	struct IO_CALLTABLE * calltable;
	
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = keyboard_open;
	calltable->close = keyboard_close;
	calltable->read = NULL;
	calltable->write = NULL;
	calltable->seek = NULL;
	calltable->control = NULL;
	
	keyboard_output = vfs_open( "/device/console0", VFS_MODE_WRITE );
	if( keyboard_output == NULL )
		return FAIL;
	// add the keyboard device
	io_add( "keyboard1", calltable, IO_CHAR );
	// setup the keyboard handler
	interrupt_enable( IRQ1, keyboard_handler, SUPERVISOR );
	// return success
	return SUCCESS;
}
