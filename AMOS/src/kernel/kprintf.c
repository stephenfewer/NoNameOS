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

#include <kernel/kprintf.h>
#include <kernel/io/io.h>
#include <kernel/lib/printf.h>

extern struct IO_HANDLE * io_kout;

void kprintf( char * text, ... )
{
	va_list args = 0;

	va_start( args, text );
	
	printf( io_kout, text, args );
	
	va_end( args );
}
