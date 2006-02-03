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

#include <kernel/fs/fat.h>
#include <kernel/fs/vfs.h>
#include <kernel/io/io.h>
#include <kernel/mm/mm.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>

struct FAT_MOUNTPOINT * mount0;

int fat_nextCluster( struct FAT_MOUNTPOINT * mount, int cluster )
{
	int next_cluster;

	switch( mount->type )
	{
		case FAT_12:

			next_cluster = *(WORD *)((BYTE *)&mount->fat_data[ ( (cluster * 3) / 2 ) ]);

			// if cluster is odd
			if( cluster & 1 )
				next_cluster >>= 4;
			else
				next_cluster = FAT_CLUSTER12( next_cluster );
		
			if( next_cluster == FAT_12_ENDOFCLUSTER )
				return -1;
			
			break;
			
		case FAT_16:
			next_cluster = ((WORD *)mount->fat_data)[ cluster ];
			if( next_cluster == FAT_16_ENDOFCLUSTER )
				return -1;
			break;	
		case FAT_32:
			next_cluster = ((DWORD *)mount->fat_data)[ cluster ];
			if( next_cluster == FAT_32_ENDOFCLUSTER )
				return -1;
			break;
		default:
			return -1;
	}

	return (int)next_cluster;	
}

int fat_determineType( struct FAT_MOUNTPOINT * mount )
{
	int root_dir_sectors;
	int total_sectors;
	int fats;
	int data_sectors;
	int cluster_count;
	
	root_dir_sectors = (((mount->bootsector.num_root_dir_ents * 32) + (mount->bootsector.bytes_per_sector - 1)) / mount->bootsector.bytes_per_sector);
	
	if( mount->bootsector.sectors_per_fat != 0 )
		fats = mount->bootsector.sectors_per_fat;
	else
		fats = mount->bootsector.bs32.BPB_FATSz32;
		
	if( mount->bootsector.total_sectors != 0 )
		total_sectors = mount->bootsector.total_sectors;
	else
		total_sectors = mount->bootsector.total_sectors_large;

	data_sectors = total_sectors - ( mount->bootsector.reserved_sectors + (mount->bootsector.num_fats * fats) + root_dir_sectors );
	
	cluster_count = data_sectors / mount->bootsector.sectors_per_cluster;

	if( cluster_count < 4085 )
		mount->type = FAT_12;
	else if( cluster_count < 65525 )
		mount->type = FAT_16;
	else
		mount->type = FAT_32;

	return mount->type;
}

int fat_cluster2block( struct FAT_MOUNTPOINT * mount, int cluster )
{
	return cluster * mount->bootsector.sectors_per_cluster
		+ mount->bootsector.hidden_sectors 
    	+ mount->bootsector.num_fats * mount->bootsector.sectors_per_fat 
    	+ mount->bootsector.num_root_dir_ents /(mount->bootsector.bytes_per_sector / sizeof (struct FAT_ENTRY))-1;
}

int fat_loadCluster( struct FAT_MOUNTPOINT * mount, int cluster, BYTE * clusterBuffer )
{
	int i, block;
	// convert cluster to a logical block number
	block = fat_cluster2block( mount, cluster );
	// seek to the correct offset
	vfs_seek( mount->device, (block*mount->bootsector.bytes_per_sector)+1, VFS_SEEK_START );
	// load in the blocks
	for( i=0 ; i<mount->bootsector.sectors_per_cluster ; i++ )
	{
		clusterBuffer += mount->bootsector.bytes_per_sector * i;
		if( vfs_read( mount->device, (void *)clusterBuffer, mount->bootsector.bytes_per_sector ) == -1 )
			return -1;
		//if( vfs_read( mount->device, (void *)(clusterBuffer+(mount->bootsector.bytes_per_sector*i)), mount->bootsector.bytes_per_sector ) == -1 )
	}
	return 0;
}

int fat_compareName( struct FAT_ENTRY * entry, char * name )
{
	int i, x;

	if( entry->name[0] == 0x00 || entry->name[0] == 0xE5 )
		return FALSE;
		
	if( entry->name[0] == 0x05 )
		entry->name[0] = 0xE5;
	
	//to do: check past the end of the extension
	for( i=0 ; i<8 ; i++ )
	{
		if( entry->name[i] == 0x20 )
			break;
			
		if( name[i] != entry->name[i] )
			return FALSE;
	}
	i++;
	if( name[i] == '.' )
	{
		i++;
		for( x=0 ; x<3 ; x++ )
		{
			if( entry->extention[x] == 0x20 )
				break;
			
			if( name[i+x] != entry->extention[x] )
				return FALSE;
		}
	}
	return TRUE;
}

int fat_getIndex( struct FAT_ENTRY * dir, char * name )
{
	int i;
	for( i=0 ; i<32 ; i++ )
	{
		if( dir[i].name[0] == 0x00 )
			break;
			
		if( dir[i].start_cluster == 0x0000 )
			continue;
			
		if( fat_compareName( &dir[i], name ) == TRUE )
			return i;
	}
	return -1;
}

int fat_file2entry( struct FAT_MOUNTPOINT * mount, char * filename, struct FAT_ENTRY * entry )
{
	int i, index = -1, length;
	char * curr_name;
	struct FAT_ENTRY * curr_dir, prevEntry;
	BYTE * clusterBuffer;
	// to-do: convert to uppercase
	// advance past the fisrt forward slash
	if( filename[0] == '/' )
		filename++;
	// get the total length of the filename string
	length = strlen( filename );
	// allocate a buffer of memory the same size as a cluster
	clusterBuffer = (BYTE *)mm_malloc( mount->cluster_size );
	// point the curr_name pointer to the filename
	curr_name = filename;
	// point the curr_dir the the root directory where we begin the search
	curr_dir = mount->rootdir;
	// loop through the filename to find the file/directory
	// we search by decomposing the filename into its component parts of
	// directorys and optional ending file
	for( i=0 ; i<=length ; i++ )
	{
		if( filename[i] == '/' || filename[i] == '\0' )
		{
			// set the forward slash to a null charachter
			filename[i] = '\0';
			// get the index in the entry to the next part of the file name
			if( (index = fat_getIndex( curr_dir, curr_name )) == -1 )
			{
				// break out of the loop
				break;
			}
			else 
			{
				// copy the current entry into the previous entry buffer
				memcpy( &prevEntry, (struct FAT_ENTRY *)&curr_dir[index], sizeof(struct FAT_ENTRY) );
				// load the next cluster to check
				if( fat_loadCluster( mount, curr_dir[index].start_cluster, clusterBuffer ) < 0 )
				{
					// free our cluster buffer
					mm_free( clusterBuffer );
					// return fail
					return -1;
				}
				// associate the current directry with the newly loaded cluster
				curr_dir = (struct FAT_ENTRY *)clusterBuffer;
			}
			// advance to the next part of the file name
			curr_name = &filename[i]+1;		
		}	
	}
	// test if we didnt find the file/directory
	if( index == -1 && strlen( curr_name ) != 0 )
	{
		// free our cluster buffer
		mm_free( clusterBuffer );
		// return fail
		return -1;
	}
	// copy the file entry into the entry truct to return to the caller
	memcpy( entry, &prevEntry, sizeof(struct FAT_ENTRY) );
	// free our cluster buffer
	mm_free( clusterBuffer );
	// return success
	return 0;
}

/*
void fat_displayDir( struct FAT_MOUNTPOINT * mount, struct FAT_ENTRY * dir )
{
	int x, next_cluster;
	
	kprintf("\n-------[dir start]-------");
	
	for(x=0;x<16;x++)
	{
		if( dir[x].start_cluster == 0x0000 )
			continue;
			
		kprintf("\n");
		
		dir[x].name[7] = 0x00;
		if( dir[x].attribute.directory )
			kprintf("D\t-\t");
		else
			kprintf("F\t-\t");
			
		kprintf("%s (%d) - start cluster %x", dir[x].name, dir[x].file_size, dir[x].start_cluster );
		
		next_cluster = fat_getFATEntry( mount, dir[x].start_cluster );
		while( TRUE )
		{
			if( next_cluster == 0x0000 )
				break;
			
			kprintf(" -> %x", next_cluster );
			
			next_cluster = fat_getFATEntry( mount, next_cluster );
		}
	}
	
	kprintf("\n-------[dir end]---------");
}
*/

int fat_mount( char * device, char * mountpoint, int fstype )
{
	int root_dir_offset;
	mount0 = (struct FAT_MOUNTPOINT *)mm_malloc( sizeof(struct FAT_MOUNTPOINT) );
	//open the device we wish to mount
	mount0->device = vfs_open( device, VFS_MODE_READWRITE );
	if( mount0->device == NULL )
		return VFS_FAIL;
	// read in the bootsector
	vfs_read( mount0->device, (void *)&mount0->bootsector, sizeof(struct FAT_BOOTSECTOR) );
	// make sure we have a valid bootsector
	if( mount0->bootsector.magic != FAT_MAGIC )
	{
		vfs_close( mount0->device );
		mm_free( mount0 );
		return VFS_FAIL;
	}
	// determine if we have a FAT 12, 16 or 32 filesystem
	fat_determineType( mount0 );
	// calculate clster size
	mount0->cluster_size = mount0->bootsector.bytes_per_sector * mount0->bootsector.sectors_per_cluster;
	// calculate the fat size
	mount0->fat_size = mount0->bootsector.sectors_per_fat * mount0->bootsector.bytes_per_sector;
	// malloc some space for the fat
	mount0->fat_data = (BYTE *)mm_malloc( mount0->fat_size );
	// and clear it
	memset( mount0->fat_data, 0x00, mount0->fat_size );
	// read in the FAT
	vfs_read( mount0->device, (void *)mount0->fat_data, mount0->fat_size );
	// read in root directory
	mount0->rootdir = (struct FAT_ENTRY *)mm_malloc( mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	memset( mount0->rootdir, 0x00, mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	// find and read in the root directory	
	root_dir_offset = (mount0->bootsector.num_fats * mount0->fat_size) + sizeof(struct FAT_BOOTSECTOR) + 1;
	vfs_seek( mount0->device, root_dir_offset, VFS_SEEK_START );
	vfs_read( mount0->device, (void *)(mount0->rootdir), mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	// return success
	return VFS_SUCCESS;
}

int fat_unmount( char * mountpoint )
{
	// close the device
	vfs_close( mount0->device );
	// free the rootdir structure
	mm_free( mount0->rootdir );
	// free the mount structure
	mm_free( mount0 );
	// return
	return VFS_SUCCESS;	
}

struct VFS_HANDLE * fat_open( struct VFS_HANDLE * handle, char * filename )
{
	struct FAT_FILE * file;
	file = (struct FAT_FILE *)mm_malloc( sizeof(struct FAT_FILE) );
	// try to find the file
	if( fat_file2entry( mount0, filename, &file->entry ) < 0 )
	{
		// if we fail free the file entry structure
		mm_free( file );
		// return null
		return NULL;
	}
	// set the mountpoint this file is on
	file->mount = mount0;
	// set the current file position to zero
	file->current_pos = 0;
	// associate the handle with the file entry
	handle->data_ptr = file;
	// if we open the file in truncate mode we need to set the size to 0
	//if( (handle->mode & VFS_MODE_TRUNCATE) == VFS_MODE_TRUNCATE )
	
	// return success
	return handle;	
}

int fat_close( struct VFS_HANDLE * handle )
{
	// check we have a fat entry associated with this handle
	if( handle->data_ptr == NULL )
		return VFS_FAIL;
	// to do: flush any buffers to disk
	// free the files fat entry
	mm_free( handle->data_ptr );
	// return success
	return VFS_SUCCESS;
}

int fat_read( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	int bytes_to_read=0, bytes_read=0, cluster_offset=0;
	int cluster;
	struct FAT_FILE * file;
	BYTE * clusterBuffer;	
	// retrieve the file structure
	file = (struct FAT_FILE *)handle->data_ptr;
	// reduce size if we are trying to read past the end of the file
	if( file->current_pos + size > file->entry.file_size )
		size = file->entry.file_size - file->current_pos;
	// initially set the cluster number to the first cluster as specified in the file entry
	cluster = file->entry.start_cluster;
	// get the correct cluster to begin reading from
	int i = file->current_pos / file->mount->cluster_size;
	// we traverse the cluster chain i times
	kprintf("fat_read: size = %d i = %d file->current_pos = %d\n",size, i, file->current_pos );
	while( i-- )
	{
	//	kprintf("\ti = %d     cluster = %x\n", i, cluster );
		// get the next cluster in the file
		cluster = fat_nextCluster( file->mount, cluster );
		// fail if we have gone beyond the files cluster chain
		if( cluster == 0x0000 || cluster == -1 )
		{
			kprintf("fat_read: cluster == 0x0000 || cluster == -1\n");
			return VFS_FAIL;
		}
	}
	// handle reads that begin from some point inside a cluster
	cluster_offset = file->current_pos % file->mount->cluster_size;
	
	kprintf("fat_read: cluster_offset = %d\n",cluster_offset);
	// allocate a buffer to read data into
	clusterBuffer = (BYTE *)mm_malloc( file->mount->cluster_size );
	// read in the data
	while( TRUE )
	{
		// se the amount of data we want to read in this loop itteration
		if( size >= file->mount->cluster_size )
			bytes_to_read = file->mount->cluster_size;
		else
			bytes_to_read = size;
		// test if we are reading accross 2 clusters, if we are we can only read up to the end of the
		// first cluster in this itteration of the loop, the next itteration will take care of the rest
		// this solution is ugly, more then likely a much cleaner way of checking for this!
		if( (cluster_offset + bytes_to_read) > (((file->current_pos / file->mount->cluster_size)+1)*file->mount->cluster_size) )
		{
			bytes_to_read = (cluster_offset + bytes_to_read) - (((file->current_pos / file->mount->cluster_size)+1)*file->mount->cluster_size);
			bytes_to_read = size - bytes_to_read;
			kprintf("fat_read: reading accross 2 clusters, bytes_to_read = %d\n", bytes_to_read );	
		}
		// read in the next cluster of data
		if( fat_loadCluster( file->mount, cluster, clusterBuffer ) < 0 )
		{
			kprintf("fat_read: fat_loadCluster failed\n");
			// free the buffer
			mm_free( clusterBuffer );
			// return fail, should we reset the files offset position if its changed?
			return VFS_FAIL;
		}
		// copy it over to the users buffer
		memcpy( buffer, (clusterBuffer+cluster_offset), bytes_to_read );
		// advance the buffer pointer
		buffer += bytes_to_read;
		// increase the bytes read
		bytes_read += bytes_to_read;
		// reduce the size
		size -= bytes_to_read;
		// if size has gone negative we have read enough
		if( size <= 0 )
		{
			//kprintf("fat_read: size < 0 \n");
			break;
		}
		// get the next cluster to read from
		cluster = fat_nextCluster( file->mount, cluster );
		// if the cluster = 0x0000 we have reached the end of the cluster chain
		if( cluster == 0x0000 || cluster == -1 )
		{
			kprintf("fat_read: bottom of loop, cluster == 0x0000 || cluster == -1  \n");
			break;
		}
		// we can now set the cluster offset to 0 if we are reading from more then 1 cluster
		cluster_offset = 0;
	}
	// free the buffer
	mm_free( clusterBuffer );
	// update the files offset position
	file->current_pos += bytes_read;
	// return the total bytes read
	return bytes_read;	
}

int fat_write( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size )
{
	return VFS_FAIL;	
}

int fat_seek( struct VFS_HANDLE * handle, DWORD offset, BYTE origin )
{
	struct FAT_FILE * file;
	int saved_pos;
	// retrieve the file structure
	file = (struct FAT_FILE *)handle->data_ptr;
	// save the origional position in case we nee to roll back
	saved_pos = file->current_pos;
	// set the new position
	if( origin == VFS_SEEK_START )
		file->current_pos = offset;
	else if( origin == VFS_SEEK_CURRENT )
		file->current_pos += offset;
	else if( origin == VFS_SEEK_END )
		file->current_pos = file->entry.file_size - offset;
	else
		return VFS_FAIL;
	// reset if we have gone over the file size
	if( file->current_pos > file->entry.file_size )
		file->current_pos = saved_pos;
	// return the current file position
	return file->current_pos;
}

int fat_control( struct VFS_HANDLE * handle, DWORD request, DWORD arg )
{
	return VFS_FAIL;		
}

int fat_create( char * filename, int mode )
{
	// get the correct directory entry
	// add in a new entry for the file
	// write it back to disk
	return VFS_FAIL;	
}

int fat_delete( char * filename )
{
	return VFS_FAIL;	
}

int fat_rename( char * src, char * dest )
{
	return VFS_FAIL;
}

int fat_copy( char * src, char * dest )
{
	return VFS_FAIL;	
}

struct VFS_DIRLIST_ENTRY * fat_list( char * dirname )
{
	int dirIndex, entryIndex, nameIndex;
	struct FAT_ENTRY * dir;
	struct VFS_DIRLIST_ENTRY * entry;
	
	dir = (struct FAT_ENTRY *)mm_malloc( mount0->cluster_size );
	
	if( fat_file2entry( mount0, dirname, dir ) < 0 )
	{
		mm_free( dir );
		return NULL;
	}
	
	fat_loadCluster( mount0, dir->start_cluster, (BYTE *)dir );
	 
	entry = (struct VFS_DIRLIST_ENTRY *)mm_malloc( sizeof(struct VFS_DIRLIST_ENTRY)*17 );

	for(dirIndex=0,entryIndex=0;dirIndex<16;dirIndex++)
	{
		if( dir[dirIndex].start_cluster == 0x0000 )
			continue;
		// fill in the name
		memset( entry[entryIndex].name, 0x00, 32 );
		for( nameIndex=0 ; nameIndex<8 ; nameIndex++ )
		{
			if( dir[dirIndex].name[nameIndex] == 0x20 )
				break;
			entry[entryIndex].name[nameIndex] = dir[dirIndex].name[nameIndex];
		}
		// and the extension if their is one
		if( dir[dirIndex].extention[0] != 0x20 )
		{
			entry[entryIndex].name[nameIndex] = '.';
			entry[entryIndex].name[nameIndex+1] = ( dir[dirIndex].extention[0] == 0x20 ? 0x00 : dir[dirIndex].extention[0] );
			entry[entryIndex].name[nameIndex+2] = ( dir[dirIndex].extention[1] == 0x20 ? 0x00 : dir[dirIndex].extention[1] );
			entry[entryIndex].name[nameIndex+3] = ( dir[dirIndex].extention[2] == 0x20 ? 0x00 : dir[dirIndex].extention[2] );
		}
		// fill in the attributes
		if( dir[dirIndex].attribute.directory )
			entry[entryIndex].attributes = VFS_DIRECTORY;
		else
			entry[entryIndex].attributes = VFS_FILE;
		// fill in the size
		entry[entryIndex].size = dir[dirIndex].file_size;

		entryIndex++;
	}
	// fill in terminating entry
	entry[entryIndex].name[0] = '\0';
	// free
	mm_free( dir );
	// return to caller. caller *must* free this structure
	return entry;
}

int fat_init()
{
	struct VFS_FILESYSTEM * fs;
	// create the file system structure
	fs = (struct VFS_FILESYSTEM *)mm_malloc( sizeof(struct VFS_FILESYSTEM) );
	// set the file system type
	fs->fstype = FAT_TYPE;
	// setup the file system calltable
	fs->calltable.open    = fat_open;
	fs->calltable.close   = fat_close;
	fs->calltable.read    = fat_read;
	fs->calltable.write   = fat_write;
	fs->calltable.seek    = fat_seek;
	fs->calltable.control = fat_control;
	fs->calltable.create  = fat_create;
	fs->calltable.delete  = fat_delete;
	fs->calltable.rename  = fat_rename;
	fs->calltable.copy    = fat_copy;
	fs->calltable.list    = fat_list;
	fs->calltable.mount   = fat_mount;
	fs->calltable.unmount = fat_unmount;
	// register the file system with the VFS
	return vfs_register( fs );
}
