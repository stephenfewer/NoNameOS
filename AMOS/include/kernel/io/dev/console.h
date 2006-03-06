#ifndef _KERNEL_IO_DEV_CONSOLE_H_
#define _KERNEL_IO_DEV_CONSOLE_H_

#include <sys/types.h>

#define	CONSOLE_1				1
#define	CONSOLE_2				2
#define	CONSOLE_3				3
#define	CONSOLE_4				4

#define CONSOLE_SETACTIVE		1
#define CONSOLE_SENDCHAR		2
#define CONSOLE_SETBREAK		3
#define CONSOLE_SETECHO			4

#define CONSOLE_PTR				1
#define CONSOLE_PTRPTR			2

#define CONSOLE_ROWS			25
#define CONSOLE_COLUMNS			80
#define CONSOLE_TABS			4

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
	GREY_BG		= 0x70,

	BLINK		= 0x80
};

struct CONSOLE_BUFFER
{
	int number;

	BYTE * in_kbuff;	
	volatile int in_buffIndex;
	volatile int in_buffSize;
	volatile BYTE in_breakByte;
	volatile BYTE in_break;

	struct CONSOLE_BUFFER * prev;
};

struct CONSOLE_DATA
{
	char * name;
	int number;
	volatile BYTE active;
	BYTE attributes;
	BYTE * mem;
	int x;
	int y;
	BYTE echo;
};

struct CONSOLE
{
	struct CONSOLE_DATA * data;
	struct CONSOLE_BUFFER * buffer;
};


int console_init( void );

#endif

