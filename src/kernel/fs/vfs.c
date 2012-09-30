/*
 *    Author:  Stephen Fewer
 *    Contact: stephen_fewer [AT] harmonysecurity [DOT] com
 *    Web:     http://www.harmonysecurity.com/
 *    License: GNU General Public License (GPLv3)
 */

#include <kernel/fs/vfs.h>
#include <kernel/fs/fat.h>
#include <kernel/fs/dfs.h>
#include <kernel/mm/mm.h>
#include <kernel/pm/process.h>
#include <lib/libc/string.h>

struct VFS_FILESYSTEM * vfs_fsHead = NULL;

struct VFS_MOUNTPOINT * vfs_mpHead = NULL;
struct VFS_MOUNTPOINT * vfs_mpTail = NULL;

// register a new file system with the VFS
int vfs_register( struct VFS_FILESYSTEM * fs )
{
	// add the new file system to a linked list of file system drivers present in the system
	fs->prev = vfs_fsHead;	
	vfs_fsHead = fs;
	// we can now mount volumes of this file system type
	return SUCCESS;
}

// unregister a file system driver from the VFS
int vfs_unregister( int fstype )
{
	return FAIL;
}

// find a file system driver of specified type
struct VFS_FILESYSTEM * vfs_find( int fstype )
{
	struct VFS_FILESYSTEM * fs;
	// search through the linked list of file system drivers
	for( fs=vfs_fsHead ; fs!=NULL ; fs=fs->prev )
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
	mount = (struct VFS_MOUNTPOINT *)mm_kmalloc( sizeof(struct VFS_MOUNTPOINT) );
	if( mount == NULL )
		return FAIL;
	// find the correct file system driver for the mount command
	mount->fs = vfs_find( fstype );
	if( mount->fs == NULL )
	{
		// failed to find it
		mm_kfree( mount );
		return FAIL;	
	}
	mount->mountpoint = (char *)mm_kmalloc( strlen(mountpoint)+1 );
	strcpy( mount->mountpoint, mountpoint );
	mount->device = (char *)mm_kmalloc( strlen(device)+1 );
	strcpy( mount->device, device );	

	// add the fs and the mountpoint to a linked list
	if( vfs_mpTail == NULL )
		vfs_mpTail = mount;
	else
		vfs_mpHead->next = mount;
	vfs_mpHead = mount;
/*
	mount->next = NULL;
	if( vfs_mpTail != NULL )
		mount->next = vfs_mpTail;
	vfs_mpTail = mount;
*/
	// call the file system driver to mount
	if( mount->fs->calltable.mount == NULL )
	{
		mm_kfree( mount );
		return FAIL;
	}
	mount->data_ptr = mount->fs->calltable.mount( device, mountpoint, fstype );
	if( mount->data_ptr == NULL )
	{
		mm_kfree( mount );
		return FAIL;
	}
	return SUCCESS;
}

int vfs_unmount( char * mountpoint )
{
	struct VFS_MOUNTPOINT * mount, * m;
	char name[VFS_MAXFILENAME], * name_ptr;
	// copy the name so we can modify it
	name_ptr = (char *)&name;
	strcpy( name_ptr, mountpoint );
	// find the mountpoint
	for( mount=vfs_mpTail ; mount!=NULL ; mount=mount->next )
	{
		// if we have a match we break from the search
		if( strcmp( mount->mountpoint, name_ptr ) == 0 )
			break;
	}
	// fail if we cant find it
	if( mount == NULL )
		return FAIL;
	// call the file system driver to unmount
	if( mount->fs->calltable.unmount == NULL )
		return FAIL;
	mount->fs->calltable.unmount( mount, name_ptr );
	// remove the mount point from the VFS
	if( mount == vfs_mpTail )
	{
		vfs_mpTail = mount->next;
	}
	else
	{
		// search through the linked list of file system drivers
		for( m=vfs_mpTail ; m!=NULL ; m=m->next )
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
	mm_kfree( mount->mountpoint );
	mm_kfree( mount->device );
	mm_kfree( mount );
	return SUCCESS;	
}

struct VFS_MOUNTPOINT * vfs_file2mountpoint( char * filename )
{
	struct VFS_MOUNTPOINT * mount;
	// find the mountpoint
	for( mount=vfs_mpTail ; mount!=NULL ; mount=mount->next )
	{
		// if we have a match we break from the search. we use strncmp instead of
		// strcmp to avoid comparing the null char at the end of the mountpoint
		if( strncmp( mount->mountpoint, filename, strlen(mount->mountpoint) ) == 0 )
			break;
	}
	// return the mountpoint
	return mount;
}

struct VFS_HANDLE * vfs_open( char * filename, int mode )
{
	struct VFS_HANDLE * handle;
	struct VFS_MOUNTPOINT * mount;
	char name[VFS_MAXFILENAME], * name_ptr;
	// copy the name so we can modify it
	name_ptr = (char *)&name;
	strcpy( name_ptr, filename );
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( name_ptr );
	if( mount == NULL )
		return NULL;
	// advance the filname past the mount point
	name_ptr = (char *)( name_ptr + strlen(mount->mountpoint) );
	// call the file system driver to open
	if( mount->fs->calltable.open == NULL )
		return NULL;
	// create the new virtual file handle	
	handle = (struct VFS_HANDLE *)mm_kmalloc( sizeof(struct VFS_HANDLE) );
	handle->mount = mount;
	handle->mode = mode;
	// try to open the file on the mounted file system
	if( mount->fs->calltable.open( handle, name_ptr ) != NULL )
	{	
		// set the file position to the end of the file if in append mode
		if( (handle->mode & VFS_MODE_APPEND) == VFS_MODE_APPEND )
			vfs_seek( handle, 0, VFS_SEEK_END );
		return handle;
	}
	else
	{
		// if we fail to open the file but are in create mode, we can create the file
		//     TO-DO: test the failed open() result for a value like FILE_NOT_FOUND
		//     otherwise we could end up in an infinite recurive loop
		if( (handle->mode & VFS_MODE_CREATE) == VFS_MODE_CREATE )
		{	
			if( mount->fs->calltable.create != NULL )
			{
				// try to create it
				name_ptr = (char *)&name;
				strcpy( name_ptr, filename );
				name_ptr = (char *)( name_ptr + strlen(mount->mountpoint) );

				if( mount->fs->calltable.create( mount, name_ptr ) != FAIL )
				{
					if( mount->fs->calltable.open( handle, name_ptr ) != NULL )
					{
						if( (handle->mode & VFS_MODE_APPEND) == VFS_MODE_APPEND )
							vfs_seek( handle, 0, VFS_SEEK_END );
						return handle;
					}
				}
			}
		}		
	}
	// if we fail, free the handle and return NULL
	mm_kfree( handle );
	return NULL;
}

int vfs_close( struct VFS_HANDLE * handle )
{
	if( handle == NULL )
		return FAIL;
	if( handle->mount->fs->calltable.close != NULL )
	{
		int ret;
		ret = handle->mount->fs->calltable.close( handle );
		mm_kfree( handle );
		return ret;
	}
	return FAIL;	
}

struct VFS_HANDLE * vfs_clone( struct VFS_HANDLE * handle )
{
	struct VFS_HANDLE * clone;

	if( handle == NULL )
		return NULL;

	if( handle->mount->fs->calltable.clone != NULL )
	{
		clone = (struct VFS_HANDLE *)mm_kmalloc( sizeof(struct VFS_HANDLE) );
		clone->mount = handle->mount;
		clone->mode = handle->mode;

		if( handle->mount->fs->calltable.clone( handle, clone ) == FAIL )
		{
			mm_kfree( clone );
			return NULL;
		}
		
		return clone;
	}

	return NULL;
}

int vfs_read( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	if( handle == NULL )
		return FAIL;
	// test if the file has been opened in read mode first
	if( (handle->mode & VFS_MODE_READ) != VFS_MODE_READ )
		return FAIL;
	// try to call the file system driver to read
	if( handle->mount->fs->calltable.read != NULL )
		return handle->mount->fs->calltable.read( handle, buffer, size  );
	// if we get here we have failed
	return FAIL;
}

int vfs_write( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size )
{
	int ret;
	if( handle == NULL )
		return FAIL;
	// test if the file ha been opened in read mode first
	if( (handle->mode & VFS_MODE_WRITE) != VFS_MODE_WRITE )
		return FAIL;
	// try to call the file system driver to write
	if( handle->mount->fs->calltable.write != NULL )
	{
		ret = handle->mount->fs->calltable.write( handle, buffer, size );
		if( ret != FAIL )
		{
			// set the file position to the end of the file if in append mode
			if( (handle->mode & VFS_MODE_APPEND) == VFS_MODE_APPEND )
				vfs_seek( handle, 0, VFS_SEEK_END );
			// return the write result
			return ret;
		}
	}
	// if we get here we have failed
	return FAIL;
}

int vfs_seek( struct VFS_HANDLE * handle, DWORD offset, BYTE origin )
{
	if( handle == NULL )
		return FAIL;
	if( handle->mount->fs->calltable.seek != NULL )
		return handle->mount->fs->calltable.seek( handle, offset, origin  );
	return FAIL;
}

int vfs_control( struct VFS_HANDLE * handle, DWORD request, DWORD arg )
{
	if( handle == NULL )
		return FAIL;
	if( handle->mount->fs->calltable.control != NULL )
		return handle->mount->fs->calltable.control( handle, request, arg  );
	return FAIL;
}

int vfs_create( char * filename )
{
	struct VFS_MOUNTPOINT * mount;
	char name[VFS_MAXFILENAME], * name_ptr;
	// copy the name so we can modify it
	name_ptr = (char *)&name;
	strcpy( name_ptr, filename );
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( name_ptr );
	if( mount == NULL )
		return FAIL;
	// advance the filname past the mount point
	name_ptr = (char *)( name_ptr + strlen(mount->mountpoint) );
	// try to create the file on the mounted file system
	if( mount->fs->calltable.create != NULL )
		return mount->fs->calltable.create( mount, name_ptr );
	// return fail
	return FAIL;	
}

int vfs_delete( char * filename )
{
	struct VFS_MOUNTPOINT * mount;
	char name[VFS_MAXFILENAME], * name_ptr;
	// copy the name so we can modify it
	name_ptr = (char *)&name;
	strcpy( name_ptr, filename );
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( name_ptr );
	if( mount == NULL )
		return FAIL;
	// advance the filname past the mount point
	name_ptr = (char *)( name_ptr + strlen(mount->mountpoint) );
	// try to delete the file on the mounted file system
	if( mount->fs->calltable.delete != NULL )
		return mount->fs->calltable.delete( mount, name_ptr );
	// return fail
	return FAIL;		
}

int vfs_rename( char * src, char * dest )
{
	struct VFS_MOUNTPOINT * mount;
	char srcname[VFS_MAXFILENAME], * srcname_ptr;
	char destname[VFS_MAXFILENAME], * destname_ptr;
	// copy the name so we can modify it
	srcname_ptr = (char *)&srcname;
	strcpy( srcname_ptr, src );
	destname_ptr = (char *)&destname;
	strcpy( destname_ptr, dest );
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( srcname_ptr );
	if( mount == NULL )
		return FAIL;
	// advance the filnames past the mount point, we should sanity check this better
	srcname_ptr = (char *)( srcname_ptr + strlen(mount->mountpoint) );
	destname_ptr = (char *)( destname_ptr + strlen(mount->mountpoint) );
	// try to rename the file on the mounted file system
	if( mount->fs->calltable.rename != NULL )
		return mount->fs->calltable.rename( mount, srcname_ptr, destname_ptr );
	// return fail
	return FAIL;
}

int vfs_copy( char * src, char * dest )
{
	struct VFS_MOUNTPOINT * mount;
	char srcname[VFS_MAXFILENAME], * srcname_ptr;
	char destname[VFS_MAXFILENAME], * destname_ptr;
	// copy the name so we can modify it
	srcname_ptr = (char *)&srcname;
	strcpy( srcname_ptr, src );
	destname_ptr = (char *)&destname;
	strcpy( destname_ptr, dest );
	// find the correct mountpoint for this file
	mount = vfs_file2mountpoint( srcname_ptr );
	if( mount == NULL )
		return FAIL;
	// advance the filnames past the mount point, we should sanity check this better
	// also dest may be on another mountpoint
	srcname_ptr = (char *)( srcname_ptr + strlen(mount->mountpoint) );
	destname_ptr = (char *)( destname_ptr + strlen(mount->mountpoint) );
	// try to copy the file on the mounted file system
	if( mount->fs->calltable.copy != NULL )
		return mount->fs->calltable.copy( mount, srcname_ptr, destname_ptr );
	// return fail
	return FAIL;	
}

struct VFS_DIRLIST_ENTRY * vfs_list( char * dir )
{
	struct VFS_MOUNTPOINT * mount;
	char name[VFS_MAXFILENAME], * name_ptr;
	// copy the name so we can modify it
	name_ptr = (char *)&name;
	strcpy( name_ptr, dir );
	// sanity check that this is a dir and not a file
	if( dir[ strlen(name_ptr)-1 ] != '/' )
		return NULL;
	// find the correct mountpoint for this dir
	mount = vfs_file2mountpoint( name_ptr );
	if( mount == NULL )
		return NULL;
	// advance the dir name past the mount point
	name_ptr = (char *)( name_ptr + strlen(mount->mountpoint) );
	// try to list the dir on the mounted file system
	if( mount->fs->calltable.list != NULL )
	{
		struct VFS_DIRLIST_ENTRY * entry;
		entry = mount->fs->calltable.list( mount, name_ptr );
		/*
		// to-do: add any virtual mount points
		struct VFS_MOUNTPOINT * mount;
		// find the mountpoint
		for( mount=vfs_mpBottom ; mount!=NULL ; mount=mount->next )
		{
			if( strncmp( mount->mountpoint, dir, strlen(mount->mountpoint) ) == 0 )
			{
				int i,c;
				for(  i=0, c=0;i<strlen(dir);i++)
				{
					if( dir[i] == '/' )
						c++;
				}
				if( c <= 2 )
					kernel_printf("vfs: mountpoint %s\n", mount->mountpoint );
			}
		}*/		
		
		// to-do: maby sort the list...
		
		return entry;
	}
	// return fail
	return NULL;
}

int vfs_init( void )
{
	// initilize Device File System driver
	dfs_init();
	// mount the DFS
	vfs_mount( NULL, "/amos/device/", DFS_TYPE );

	// initilize FAT File System driver
	fat_init();

	return SUCCESS;
}
