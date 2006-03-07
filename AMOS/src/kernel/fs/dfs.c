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

#include <kernel/fs/dfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/mm/mm.h>
#include <kernel/io/io.h>
#include <lib/string.h>

struct DFS_ENTRY * dfs_deviceHead = NULL;

struct DFS_ENTRY * dfs_add( char * name, struct IO_CALLTABLE * calltable, int type )
{
	struct DFS_ENTRY * device;
	
	device = (struct DFS_ENTRY *)mm_malloc( sizeof(struct DFS_ENTRY) );

	device->prev = NULL;
	
	if( dfs_deviceHead != NULL )
		device->prev = dfs_deviceHead;
	dfs_deviceHead = device;

	device->calltable = calltable;
	
	device->name = name;
	
	device->type = type;
	
	return device;
}

struct DFS_ENTRY * dfs_find( char * name )
{
	struct DFS_ENTRY * device;
	for( device=dfs_deviceHead ; device!=NULL ; device=device->prev )
	{
		if( strcmp( device->name, name ) == 0 )
			break;
	}
	return device;
}

int dfs_remove( char * name  )
{
	struct DFS_ENTRY * device, * d;
	// find the device
	device = dfs_find( name );
	if( device == NULL )
		return FAIL;
	// To Do: test for any open file handles of the requested device
	
	// remove from linked list
	if( device == dfs_deviceHead )
	{
		dfs_deviceHead = device->prev;
	}
	else
	{
		for( d=dfs_deviceHead ; d!=NULL ; d=d->prev )
		{
			if( d->prev == device )
			{
				d->prev = device->prev;
				break;
			}
		}
	}	
	// free all memory allocated
	mm_free( device->calltable );
	mm_free( device->name );
	mm_free( device );
	// return success
	return SUCCESS;
}

int dfs_mount( char * device, char * mountpoint, int fstype )
{
	if( fstype == DFS_TYPE )
		return SUCCESS;
	return FAIL;
}

int dfs_unmount( char * mountpoint )
{
	return FAIL;	
}

struct VFS_HANDLE * dfs_open( struct VFS_HANDLE * handle, char * filename )
{
	struct DFS_ENTRY * device;
	
	device = dfs_find( filename );
	if( device == NULL )
		return NULL;

	handle->data_ptr = (void *)io_open( device );
	if(	handle->data_ptr == NULL )
		return NULL;

	return handle;	
}

int dfs_close( struct VFS_HANDLE * handle )
{
	return io_close( (struct IO_HANDLE *)handle->data_ptr );
}

int dfs_read( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	return io_read( (struct IO_HANDLE *)handle->data_ptr, buffer, size );	
}

int dfs_write( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size )
{
	return io_write( (struct IO_HANDLE *)handle->data_ptr, buffer, size );	
}

int dfs_seek( struct VFS_HANDLE * handle, DWORD offset, BYTE origin )
{
	return io_seek( (struct IO_HANDLE *)handle->data_ptr, offset, origin );	
}

int dfs_control( struct VFS_HANDLE * handle, DWORD request, DWORD arg )
{
	return io_control( (struct IO_HANDLE *)handle->data_ptr, request, arg );		
}

int dfs_create( char * filename )
{
	// we cant create a new file here. new devices are to be loaded
	// into the system with the IO Subsystems load() system call.
	return FAIL;	
}

int dfs_delete( char * filename )
{
	return dfs_remove( filename );
}

int dfs_copy( char * src, char * dest )
{
	struct DFS_ENTRY * device;
	char * name;
	struct IO_CALLTABLE * calltable;
	// find the source device
	device = dfs_find( src );
	if( device == NULL )
		return FAIL;
	// copy the calltable
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	memcpy( calltable, device->calltable, sizeof(struct IO_CALLTABLE) );
	// copy the name
	name = (char *)mm_malloc( strlen(dest) );
	strcpy( name, dest );
	// add a new device with the source devices calltable
	dfs_add( name, calltable, device->type );
	return SUCCESS;	
}

int dfs_rename( char * src, char * dest )
{
	if( dfs_copy( src, dest ) == FAIL )
		return FAIL;
	return dfs_delete( src );
}

struct VFS_DIRLIST_ENTRY * dfs_list( char * dir )
{
	struct DFS_ENTRY * device;
	struct VFS_DIRLIST_ENTRY * entry;
	int i=0;
	// count how many devices we have
	for( device=dfs_deviceHead ; device!=NULL ; device=device->prev )
		i++;
	// return NULL if we dont have any
	if( i == 0 )
		return NULL;
	// create the array of entry structures
	entry = (struct VFS_DIRLIST_ENTRY *)mm_malloc( (sizeof(struct VFS_DIRLIST_ENTRY)*i)+1 );
	// clear it
	memset( entry, 0x00, (sizeof(struct VFS_DIRLIST_ENTRY)*i)+1 );
	for( device=dfs_deviceHead, i=0 ; device!=NULL ; device=device->prev, i++ )
	{
		// fill in the name
		strncpy( entry[i].name, device->name, 32 );
		// fill in the attributes
		entry[i].attributes = VFS_DEVICE;
		// fill in the size
		entry[i].size = 0;
	}
	// return to caller. caller *must* free this structure
	return entry;
}

int dfs_init( void )
{
	struct VFS_FILESYSTEM * fs;
	// create the file system structure
	fs = (struct VFS_FILESYSTEM *)mm_malloc( sizeof(struct VFS_FILESYSTEM) );
	// set the file system type
	fs->fstype = DFS_TYPE;
	// setup the file system calltable
	fs->calltable.open    = dfs_open;
	fs->calltable.close   = dfs_close;
	fs->calltable.read    = dfs_read;
	fs->calltable.write   = dfs_write;
	fs->calltable.seek    = dfs_seek;
	fs->calltable.control = dfs_control;
	fs->calltable.create  = dfs_create;
	fs->calltable.delete  = dfs_delete;
	fs->calltable.rename  = dfs_rename;
	fs->calltable.copy    = dfs_copy;
	fs->calltable.list    = dfs_list;
	fs->calltable.mount   = dfs_mount;
	fs->calltable.unmount = dfs_unmount;
	// register the file system with the VFS
	return vfs_register( fs );
}
