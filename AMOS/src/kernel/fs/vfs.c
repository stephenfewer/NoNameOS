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

#include <kernel/fs/vfs.h>
#include <kernel/fs/fat.h>
#include <kernel/mm/mm.h>
#include <kernel/lib/string.h>

struct VFS_FILESYSTEM * fs_top = NULL;
struct VFS_FILESYSTEM * fs_bottom = NULL;

struct VFS_MOUNTPOINT * mount_top = NULL;
struct VFS_MOUNTPOINT * mount_bottom = NULL;

// register a new file system with the VFS
int vfs_register( struct VFS_FILESYSTEM * fs )
{
	// add the new file system to a linked list of file system
	// drivers present in the system
	if( fs_bottom == NULL )
	{
		fs_bottom = fs_top = fs;
	}
	else
	{
		fs_top->next = fs;
		fs_top = fs;
	}
	// we can now mount volumes of this file system type
	return VFS_SUCCESS;
}

// unregister a file system driver from the VFS
int vfs_unregister( int fstype )
{
	return VFS_FAIL;
}

// find a file system driver of specified type
struct VFS_FILESYSTEM * vfs_find( int fstype )
{
	struct VFS_FILESYSTEM * fs;
	// search through the linked list of file system drivers
	for( fs=fs_bottom ; fs!=NULL ; fs=fs->next )
	{
		// if we have a match we break from the search
		if( fs->fstype == fstype )
			break;
	}
	// return the fs driver to caller, or NULL if failed to find
	return fs;
}

int vfs_mount( char * device, char * mountpoint, int fstype )
{
	struct VFS_MOUNTPOINT * mount;
	// create our mountpoint structure
	mount = (struct VFS_MOUNTPOINT *)mm_malloc( sizeof(struct VFS_MOUNTPOINT) );
	// find the correct file system driver for the mount command
	mount->fs = vfs_find( fstype );
	if( mount->fs == NULL )
	{
		// failed to find it
		mm_free( mount );
		return VFS_FAIL;	
	}
	mount->mountpoint = (char *)mm_malloc( strlen(mountpoint)+1 );
	strcpy( mount->mountpoint, mountpoint );
	mount->device = (char *)mm_malloc( strlen(device)+1 );
	strcpy( mount->device, device );	
	// add the fs and the mountpoint to a linked list
	if( mount_bottom == NULL )
	{
		mount_bottom = mount_top = mount;
	}
	else
	{
		mount_top->next = mount;
		mount_top = mount;
	}
	
	// call the file system driver to mount
	if( mount->fs->calltable.mount == NULL )
		return VFS_FAIL;
	return mount->fs->calltable.mount( device, mountpoint, fstype );
}

int vfs_unmount( char * mountpoint )
{
	struct VFS_MOUNTPOINT * mount, * m;
	// find the mountpoint
	for( mount=mount_bottom ; mount!=NULL ; mount=mount->next )
	{
		// if we have a match we break from the search
		if( strcmp( mount->mountpoint, mountpoint ) == 0 )
		{
			break;
		}
	}
	// fail if we cant find it
	if( mount == NULL )
		return VFS_FAIL;
	// call the file system driver to unmount
	if( mount->fs->calltable.unmount == NULL )
		return VFS_FAIL;
	mount->fs->calltable.unmount( mountpoint );
	// remove the mount point from the VFS
	if( mount == mount_bottom )
	{
		mount_bottom = mount->next;
	}
	else
	{
		// search through the linked list of file system drivers
		for( m=mount_bottom ; m!=NULL ; m=m->next )
		{
			// if we have a match we break from the search
			if( m->next == mount )
			{
				// remove the item
				m->next = mount->next;
				break;
			}
		}
	}
	// free the mount structure
	mm_free( mount->mountpoint );
	mm_free( mount->device );
	mm_free( mount );
	return VFS_SUCCESS;	
}

struct VFS_MOUNTPOINT * vfs_file2mountpoint( char * filename )
{
	struct VFS_MOUNTPOINT * mount;
	// find the mountpoint
	for( mount=mount_bottom ; mount!=NULL ; mount=mount->next )
	{
		// if we have a match we break from the search. we use strncmp instead of
		// strcmp to avoid comparing the null char at the end of the mountpoint
		if( strncmp( mount->mountpoint, filename, strlen(mount->mountpoint) ) == 0 )
			break;
	}
	// return the mountpoint
	return mount;
}

struct VFS_HANDLE * vfs_open( char * filename )
{
	struct VFS_HANDLE * handle;
	struct VFS_MOUNTPOINT * mount;
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( filename );
	if( mount == NULL )
		return NULL;
	// advance the filname past the mount point
	filename = (char *)( filename + strlen(mount->mountpoint) );
	// call the file system driver to open
	if( mount->fs->calltable.open == NULL )
		return NULL;
	// create the new virtual file handle	
	handle = (struct VFS_HANDLE *)mm_malloc( sizeof(struct VFS_HANDLE) );
	handle->mount = mount;
	// try to open the file on the mounted file system
	if( mount->fs->calltable.open( handle, filename ) != NULL )
		return handle;
	// if we fail, free the handle and return NULL
	mm_free( handle );
	return NULL;
}

int vfs_close( struct VFS_HANDLE * handle )
{
	if( handle->mount->fs->calltable.close != NULL )
	{
		int ret;
		ret = handle->mount->fs->calltable.close( handle );
		mm_free( handle );
		return ret;
	}
	return VFS_FAIL;	
}

int vfs_read( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	if( handle->mount->fs->calltable.read != NULL )
		return handle->mount->fs->calltable.read( handle, buffer, size  );
	return VFS_FAIL;
}

int vfs_write( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size )
{
	if( handle->mount->fs->calltable.write != NULL )
		return handle->mount->fs->calltable.write( handle, buffer, size  );
	return VFS_FAIL;
}

int vfs_seek( struct VFS_HANDLE * handle, DWORD offset, BYTE origin )
{
	if( handle->mount->fs->calltable.seek != NULL )
		return handle->mount->fs->calltable.seek( handle, offset, origin  );
	return VFS_FAIL;
}

int vfs_control( struct VFS_HANDLE * handle, DWORD request, DWORD arg )
{
	if( handle->mount->fs->calltable.control != NULL )
		return handle->mount->fs->calltable.control( handle, request, arg  );
	return VFS_FAIL;
}

int vfs_create( char * filename, int flags )
{
	struct VFS_MOUNTPOINT * mount;
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( filename );
	if( mount == NULL )
		return VFS_FAIL;
	// try to create the file on the mounted file system
	if( mount->fs->calltable.create != NULL )
		return mount->fs->calltable.create( filename, flags );
	// return fail
	return VFS_FAIL;	
}

int vfs_delete( char * filename )
{
	struct VFS_MOUNTPOINT * mount;
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( filename );
	if( mount == NULL )
		return VFS_FAIL;
	// try to delete the file on the mounted file system
	if( mount->fs->calltable.delete != NULL )
		return mount->fs->calltable.delete( filename );
	// return fail
	return VFS_FAIL;		
}

int vfs_rename( char * src, char * dest )
{
	struct VFS_MOUNTPOINT * mount;
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( src );
	if( mount == NULL )
		return VFS_FAIL;
	// try to rename the file on the mounted file system
	if( mount->fs->calltable.rename != NULL )
		return mount->fs->calltable.rename( src, dest );
	// return fail
	return VFS_FAIL;
}

int vfs_copy( char * src, char * dest )
{
	struct VFS_MOUNTPOINT * mount;
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( src );
	if( mount == NULL )
		return VFS_FAIL;
	// try to copy the file on the mounted file system
	if( mount->fs->calltable.copy != NULL )
		return mount->fs->calltable.copy( src, dest );
	// return fail
	return VFS_FAIL;	
}

struct VFS_DIRLIST_ENTRY * vfs_list( char * dir )
{
	struct VFS_MOUNTPOINT * mount;
	// find the correct mountpoint for this dir
	mount = vfs_file2mountpoint( dir );
	if( mount == NULL )
		return NULL;
	// try to list the dir on the mounted file system
	if( mount->fs->calltable.list != NULL )
		return mount->fs->calltable.list( dir );
	// return fail
	return NULL;
}

int vfs_init()
{
	// initilize Device File System driver
	dfs_init();
	// initilize FAT File System driver
	//fat_init();
	
	// mount the device fils system
	vfs_mount( NULL, "/device/", DFS_TYPE );
	// mount the root file system
	//vfs_mount( "/device/floppy1", "/", FAT_TYPE );
	
	return VFS_SUCCESS;
}
