/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <lib/amos.h>
#include <kernel/syscall.h>
#include <kernel/fs/vfs.h>

int open( char * filename, int mode )
{
	int handle=FAIL;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (handle) : "a" (SYSCALL_OPEN), "b" (filename), "c" (mode) );
	return handle;
}

int close( int handle )
{
	int ret=FAIL;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_CLOSE), "b" (handle) );
	return ret;	
}

int clone( int handle )
{
	int ret=FAIL;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_CLONE), "b" (handle) );
	return ret;		
}

int read( int handle, BYTE * buffer, DWORD size  )
{
	int ret=FAIL;
	if( handle < 0 || buffer == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_READ), "b" (handle), "c" (buffer), "d" (size) );
	return ret;		
}

int write( int handle, BYTE * buffer, DWORD size  )
{
	int ret=FAIL;
	if( handle < 0 || buffer == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_WRITE), "b" (handle), "c" (buffer), "d" (size) );
	return ret;		
}

int seek( int handle, DWORD offset, BYTE origin )
{
	int ret=FAIL;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_SEEK), "b" (handle), "c" (offset), "d" (origin) );
	return ret;		
}

int control( int handle, DWORD request, DWORD arg )
{
	int ret=FAIL;
	if( handle < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_CONTROL), "b" (handle), "c" (request), "d" (arg) );
	return ret;			
}

int create( char * filename )
{
	int ret=FAIL;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_CREATE), "b" (filename) );
	return ret;		
}

int delete( char * filename )
{
	int ret=FAIL;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_DELETE), "b" (filename) );
	return ret;		
}

int rename( char * src, char * dest )
{
	int ret=FAIL;
	if( src == NULL || dest == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_RENAME), "b" (src), "c" (dest) );
	return ret;		
}

int copy( char * src, char * dest )
{
	int ret=FAIL;
	if( src == NULL || dest == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_COPY), "b" (src), "c" (dest) );
	return ret;		
}

int list( char * dir, DIRLIST_ENTRY * entry, int entry_num )
{
	int ret=FAIL;
	if( dir == NULL || entry == NULL || entry_num <= 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_LIST), "b" (dir), "c" (entry), "d" (entry_num) );
	return ret;		
}

int mount( char * device, char * mountpoint, int fstype )
{
	int ret=FAIL;
	if( device == NULL || mountpoint == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_MOUNT), "b" (device), "c" (mountpoint), "d" (fstype) );
	return ret;			
}

int unmount( char * mountpoint )
{
	int ret=FAIL;
	if( mountpoint == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_UNMOUNT), "b" (mountpoint) );
	return ret;		
}

void * morecore( DWORD size )
{
	void * address = NULL;
	if( size == 0 )
		return NULL;
	ASM( "int $0x30" : "=a" (address) : "a" (SYSCALL_MORECORE), "b" (size) );
	return address;
}

void exit( void )
{
	ASM( "int $0x30" :: "a" (SYSCALL_EXIT) );
}

int spawn( char * filename, char * console_path )
{
	int ret=FAIL;
	if( filename == NULL )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_SPAWN), "b" (filename), "c" (console_path) );
	return ret;			
}

int kill( int id )
{
	int ret=FAIL;
	if( id <= 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_KILL), "b" (id) );
	return ret;			
}

int sleep( void )
{
	int ret=FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_SLEEP) );
	return ret;	
}

int wake( int id )
{
	int ret=FAIL;
	if( id < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_WAKE), "b" (id) );
	return ret;		
}

int wait( int id )
{
	int ret=FAIL;
	if( id < 0 )
		return FAIL;
	ASM( "int $0x30" : "=a" (ret) : "a" (SYSCALL_WAIT), "b" (id) );
	return ret;		
}

