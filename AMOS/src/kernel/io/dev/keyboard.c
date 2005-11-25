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

#include <kernel/io/dev/keyboard.h>
#include <kernel/isr.h>
#include <kernel/kernel.h>
#include <kernel/mm/mm.h>
#include <kernel/io/device.h>
#include <kernel/io/dev/console.h>

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
	return 0;
}

int keyboard_read( struct IO_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	// read some bytes off the buffer
	return -1;
}

DWORD keyboard_handler( struct TASK_STACK * taskstack )
{
/*	BYTE scancode;
	
	scancode = inportb( KEYBOARD_DATAREG );
	
	if( scancode & 0x80 )
	{
		// key release
	}
	else
	{
		// key press, add it to a buffer
		
		// for testing just dump it out console...
		//if( (scancode & 0x7F) < 128 )
		//	console_putch( keymap[scancode] );
	}
*/	
	return (DWORD)NULL;
}

void keyboard_init()
{
	struct IO_CALLTABLE * calltable;
	
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = keyboard_open;
	calltable->close = keyboard_close;
	calltable->read = keyboard_read;
	calltable->write = NULL;
	calltable->seek = NULL;
	
	device_add( "/device/keyboard", calltable );
	
	// setup the keyboard handler
	isr_setHandler( IRQ1, keyboard_handler );
}
