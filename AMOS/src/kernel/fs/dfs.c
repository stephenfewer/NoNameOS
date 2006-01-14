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

#include <kernel/fs/dfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/mm/mm.h>
#include <kernel/io/io.h>
#include <kernel/lib/string.h>

struct DFS_ENTRY * device_top = NULL;
struct DFS_ENTRY * device_bottom = NULL;

struct DFS_ENTRY * dfs_add( char * name, struct IO_CALLTABLE * calltable )
{
	struct DFS_ENTRY * device;
	
	device = (struct DFS_ENTRY *)mm_malloc( sizeof(struct DFS_ENTRY) );
	
	if( device_bottom == NULL )
	{
		device_bottom = device_top = device ;
	}
	else
	{
		device_top->next = device;
		device_top = device;
	}
	
	device->next = NULL;
	
	device->calltable = calltable;
	
	device->name = name;
	
	return device;
}

struct DFS_ENTRY * dfs_find( char * name )
{
	struct DFS_ENTRY * device;
	
	for( device=device_bottom ; device!=NULL ; device=device->next )
	{
		if( strcmp( device->name, name ) == 0 )
			break;
	}
	
	return device;
}

int dfs_remove( char * name  )
{
	struct DFS_ENTRY * device;
	// find the device
	device = dfs_find( name );
	if( device == NULL )
		return VFS_FAIL;
	// remove from linked list
	
	// free all memory allocated
	mm_free( device->calltable );
	mm_free( device->name );
	mm_free( device );
	
	return VFS_SUCCESS;
}

int dfs_mount( char * device, char * mountpoint, int fstype )
{
	return VFS_FAIL;
}

int dfs_unmount( char * mountpoint )
{
	return VFS_FAIL;	
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

int dfs_create( char * filename, int flags )
{
	// we cant create a new file here. new devices are to be loaded
	// into the system with the IO Subsystems load() system call.
	return VFS_FAIL;	
}

int dfs_delete( char * filename )
{
	return dfs_remove( filename );	
}

int dfs_copy( char * src, char * dest )
{
	struct DFS_ENTRY * device;
	struct IO_CALLTABLE * calltable;
	// find the source device
	device = dfs_find( src );
	if( device == NULL )
		return VFS_FAIL;
	// copy the calltable
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	memcpy( calltable, device->calltable );
	// add a new device with the source devices calltable
	dfs_add( dest, calltable );
	return VFS_SUCCESS;	
}

int dfs_rename( char * src, char * dest )
{
	if( dfs_copy( src, dest ) == VFS_FAIL )
		return VFS_FAIL;
	return dfs_delete( src );
}

int dfs_list( char * directoryname )
{
	return VFS_FAIL;
}

int dfs_init()
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
