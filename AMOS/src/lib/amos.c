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

#include <lib/amos.h>
#include <kernel/syscall.h>
#include <kernel/fs/vfs.h>

int open( char * filename, int mode )
{
	int handle=0, num=SYSCALL_OPEN;
	if( filename == NULL )
		return 0;
	ASM( "int $0x30" : "=a" (handle) : "a" (num), "b" (filename), "c" (mode) );
	return handle;
}

int close( int handle )
{
	int ret=FAIL, num=SYSCALL_CLOSE;
	if( handle == 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle) );
	return ret;	
}

int read( int handle, BYTE * buffer, DWORD size  )
{
	int ret=FAIL, num=SYSCALL_READ;
	if( handle == 0 || buffer == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (buffer), "d" (size) );
	return ret;		
}

int write( int handle, BYTE * buffer, DWORD size  )
{
	int ret=FAIL, num=SYSCALL_WRITE;
	if( handle == 0 || buffer == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (buffer), "d" (size) );
	return ret;		
}

int seek( int handle, DWORD offset, BYTE origin )
{
	int ret=FAIL, num=SYSCALL_SEEK;
	if( handle == 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (offset), "d" (origin) );
	return ret;		
}

int control( int handle, DWORD request, DWORD arg )
{
	int ret=FAIL, num=SYSCALL_CONTROL;
	if( handle == 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (request), "d" (arg) );
	return ret;			
}
