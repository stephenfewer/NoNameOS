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
#include <kernel/fs/vfs.h>
#include <kernel/lib/printf.h>

// the kernels standard output handle
extern struct VFS_HANDLE * kernel_kout;

void kprintf( char * text, ... )
{
	va_list args;
	// find the first argument
	va_start( args, text );
	// pass printf the kernels std output handle the format text and the first argument
	printf( kernel_kout, text, args );
}
