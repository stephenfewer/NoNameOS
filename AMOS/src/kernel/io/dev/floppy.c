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

#include <kernel/io/dev/floppy.h>
#include <kernel/kernel.h>
#include <kernel/mm/mm.h>
#include <kernel/io/device.h>
#include <kernel/io/io.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>

struct FLOPPY_DRIVE * floppy0 = NULL;
struct FLOPPY_DRIVE * floppy1 = NULL;
/*
struct FLOPPY_FORMAT floppy_formats[6] = {
	{ 0, 0, 0 },
	{ 40, 2, 9 },
	{ 80, 2, 15 },
	{ 80, 2, 9 },
	{ 80, 2, 18 },
	{ 80, 2, 36 }
};
*/
struct DEVICE_HANDLE * floppy_open( struct DEVICE_HANDLE * handle, char * filename )
{
	if( strcmp( filename, "/device/floppy0" ) == 0 )
		handle->data = floppy0;
	else if( strcmp( filename, "/device/floppy1" ) == 0 )
		handle->data = floppy1;
	else
	{
		mm_free( handle );
		handle = NULL;
	}
	
	return handle;	
}

int floppy_close( struct DEVICE_HANDLE * handle)
{
	return -1;	
}

int floppy_read( struct DEVICE_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	return -1;	
}

int floppy_write( struct DEVICE_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	return -1;	
}
#define FLOPPY_DATA		0x3F5
#define FLOPPY_MSR		0x3F4
#define FLOPPY_TIMEOUT	128

#define FDC_DOR  (0x3f2)   /* Digital Output Register */
#define FDC_DRS  (0x3f4)   /* Data Rate Select Register (output) */
#define FDC_DIR  (0x3f7)   /* Digital Input Register (input) */
#define FDC_CCR  (0x3f7)   /* Configuration Control Register (output) */
/* command bytes (these are 765 commands + options such as MFM, etc) */
#define CMD_SPECIFY (0x03)  /* specify drive timings */
#define CMD_WRITE   (0xc5)  /* write data (+ MT,MFM) */
#define CMD_READ    (0xe6)  /* read data (+ MT,MFM,SK) */
#define CMD_RECAL   (0x07)  /* recalibrate */
#define CMD_SENSEI  (0x08)  /* sense interrupt status */
#define CMD_FORMAT  (0x4d)  /* format track (+ MFM) */
#define CMD_SEEK    (0x0f)  /* seek track */
#define CMD_VERSION (0x10)  /* FDC version */

void floppy_sendbyte( BYTE b )
{
    BYTE msr;
    int timeout = FLOPPY_TIMEOUT;
    
    while( timeout-- )
    {
		msr = inportb( FLOPPY_MSR );
		if( ( msr & 0xC0 ) == 0x80 )
		{
		    outportb( FLOPPY_DATA, b );
		    return;
		}
		inportb( 0x80 );
    }	
}

BYTE floppy_getbyte()
{
    BYTE msr;
    int timeout = FLOPPY_TIMEOUT;
    
    while( timeout-- )
    {
		msr = inportb( FLOPPY_MSR );
		if( ( msr & 0xD0 ) == 0xD0 )
		    return inportb( FLOPPY_DATA );
		inportb( 0x80 );
    }
    
    return -1;	
}

void floppy_init()
{
	BYTE i, floppy_type;
	struct IO_CALLTABLE * calltable;
	
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = floppy_open;
	calltable->close = floppy_close;
	calltable->read = floppy_read;
	calltable->write = floppy_write;
    
    // ask the CMOS if we have any floppy drives
	outportb( 0x70, 0x10 );
	i = inportb( 0x71 );

	// detect the first floppy drive
	floppy_type = i >> 4;
	if( floppy_type != 0 )
	{
		floppy0 = (struct FLOPPY_DRIVE *)mm_malloc( sizeof(struct FLOPPY_DRIVE) );
		floppy0->type = floppy_type;	
	/*	floppy0->format.cylinders = floppy_formats[floppy_type].cylinders;
		floppy0->format.heads = floppy_formats[floppy_type].heads;
		floppy0->format.tracks = floppy_formats[floppy_type].tracks;
		*/
		device_add( "/device/floppy0", calltable );
	}
	
	// detect the second floppy drive
    floppy_type = i & 0x0F;
 	if( floppy_type != 0 )
	{
		floppy1 = (struct FLOPPY_DRIVE *)mm_malloc( sizeof(struct FLOPPY_DRIVE) );
		floppy1->type = floppy_type;
	/*	floppy1->format.cylinders = floppy_formats[floppy_type].cylinders;
		floppy1->format.heads = floppy_formats[floppy_type].heads;
		floppy1->format.tracks = floppy_formats[floppy_type].tracks;
		*/		
		device_add( "/device/floppy1", calltable );
	}

}
