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
	int handle=FAIL, num=SYSCALL_OPEN;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (handle) : "a" (num), "b" (filename), "c" (mode) );
	return handle;
}

int close( int handle )
{
	int ret=FAIL, num=SYSCALL_CLOSE;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle) );
	return ret;	
}

int read( int handle, BYTE * buffer, DWORD size  )
{
	int ret=FAIL, num=SYSCALL_READ;
	if( handle < 0 || buffer == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (buffer), "d" (size) );
	return ret;		
}

int write( int handle, BYTE * buffer, DWORD size  )
{
	int ret=FAIL, num=SYSCALL_WRITE;
	if( handle < 0 || buffer == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (buffer), "d" (size) );
	return ret;		
}

int seek( int handle, DWORD offset, BYTE origin )
{
	int ret=FAIL, num=SYSCALL_SEEK;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (offset), "d" (origin) );
	return ret;		
}

int control( int handle, DWORD request, DWORD arg )
{
	int ret=FAIL, num=SYSCALL_CONTROL;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (handle), "c" (request), "d" (arg) );
	return ret;			
}

int create( char * filename )
{
	int ret=FAIL, num=SYSCALL_CREATE;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (filename) );
	return ret;		
}

int delete( char * filename )
{
	int ret=FAIL, num=SYSCALL_DELETE;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (filename) );
	return ret;		
}

int rename( char * src, char * dest )
{
	int ret=FAIL, num=SYSCALL_RENAME;
	if( src == NULL || dest == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (src), "c" (dest) );
	return ret;		
}

int copy( char * src, char * dest )
{
	int ret=FAIL, num=SYSCALL_COPY;
	if( src == NULL || dest == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (src), "c" (dest) );
	return ret;		
}

struct DIRLIST_ENTRY * list( char * dir )
{
	struct DIRLIST_ENTRY * ret=NULL;
	int num=SYSCALL_LIST;
	if( dir == NULL )
		return NULL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (dir) );
	return ret;		
}

int mount( char * device, char * mountpoint, int fstype )
{
	int ret=FAIL, num=SYSCALL_MOUNT;
	if( device == NULL || mountpoint == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (device), "c" (mountpoint), "d" (fstype) );
	return ret;			
}

int unmount( char * mountpoint )
{
	int ret=FAIL, num=SYSCALL_UNMOUNT;
	if( mountpoint == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (num), "b" (mountpoint) );
	return ret;		
}

void * morecore( DWORD size )
{
	int num=SYSCALL_MORECORE;
	void * address = NULL;
	if( size == 0 )
		return NULL;
	ASM( "int $0x30" : "=a" (address) : "a" (num), "b" (size) );
	return address;
}

void exit( void )
{
	int num=SYSCALL_EXIT;
	ASM( "int $0x30" :: "a" (num) );
}
