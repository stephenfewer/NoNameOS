#ifndef _KERNEL_CONSOLE_H_
#define _KERNEL_CONSOLE_H_

#include <sys/types.h>

#define CONSOLE_ROWS		25
#define CONSOLE_COLUMNS		80
#define CONSOLE_TABS		4
#define VIDEOMEM_BASE		0xB8000

enum { 
	BLACK		= 0x00,
	BLUE		= 0x01,
	GREEN		= 0x02,
	CYAN		= 0x03,
	RED			= 0x04,
	MAGENTA		= 0x05,
	YELLOW		= 0x06,
	WHITE		= 0x07,
	BRIGHT		= 0x08,

	BLACK_BG	= 0x00,
	BLUE_BG		= 0x10,
	GREEN_BG	= 0x20,
	CYAN_BG		= 0x30,
	RED_BG		= 0x40,
	MAGENTA_BG	= 0x50,
	YELLOW_BG	= 0x60,
	WHITE_BG	= 0x70,

	BLINK		= 0x80
};

void kprintf( char *, ... );

void console_putint( int );

void console_puthex( DWORD );

void console_putuint( int );

void console_putch( BYTE );

BYTE console_getChar( int, int );

void console_setAttrib( BYTE );

BYTE console_getAttrib( int, int );

void console_scrollup( void );

void console_putChar( int, int, BYTE, BYTE );

void console_setCursor( int, int );

void console_cls( void );

void console_beep( void );

void console_init( void );

#endif

