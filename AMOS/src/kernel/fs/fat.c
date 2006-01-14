#include <kernel/fs/fat.h>
#include <kernel/fs/vfs.h>
#include <kernel/io/io.h>
#include <kernel/mm/mm.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>

struct FAT_MOUNTPOINT * mount0;

int fat_getFATEntry( struct FAT_MOUNTPOINT * mount, int index )
{
	int entry=-1;

	switch( mount->type )
	{
		case FAT_12:
			entry = ((WORD *)mount->fat_data)[ ( (index * 3) / 2 ) ];
			
			if( index%2 == 1 )
				entry >>= 4;
			else
				entry &= 0x0FFF; 

			if( entry == FAT_EOC12 )
				return -1;

			break;
		case FAT_16:
			entry = ((WORD *)mount->fat_data)[ index ];
			if( entry == FAT_EOC16 )
				return -1;
			break;	
		case FAT_32:
			entry = ((DWORD *)mount->fat_data)[ index ];
			if( entry == FAT_EOC32 )
				return -1;
			break;
	}
	
	return entry;	
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

	data_sectors = total_sectors - ( mount->bootsector.num_boot_sectors + (mount->bootsector.num_fats * fats) + root_dir_sectors );
	
	cluster_count = data_sectors / mount->bootsector.sectors_per_cluster;
	
	if( cluster_count < 4085 )
		mount->type = FAT_12;
	else if( cluster_count < 65525 )
		mount->type = FAT_16;
	else
		mount->type = FAT_32;
	
	kprintf("FAT: FAT type %d\n", mount->type );
	
	return mount->type;
}

int fat_clusterToBlock( struct FAT_MOUNTPOINT * mount, int cluster )
{
  return cluster * mount->bootsector.sectors_per_cluster
    + mount->bootsector.hidden_sectors 
    + mount->bootsector.num_fats * mount->bootsector.sectors_per_fat 
    + mount->bootsector.num_root_dir_ents /(mount->bootsector.bytes_per_sector / sizeof (struct FAT_ENTRY))-1;
}

void * fat_loadCluster( struct FAT_MOUNTPOINT * mount, int cluster )
{
	int i;
	int block;
	void * buffer;
	// convert cluster to a logical block number
	block = fat_clusterToBlock( mount, cluster );
	// allocate our buffer
	buffer = mm_malloc( mount->cluster_size );
	// seek to the correct offset
	vfs_seek( mount->device, (block*mount->bootsector.bytes_per_sector)+1, SEEK_START );
	// load in the blocks
	for( i=0 ; i<mount->bootsector.sectors_per_cluster ; i++ )
	{
		buffer += mount->bootsector.bytes_per_sector * i;
		if( vfs_read( mount->device, (void *)(buffer+(mount->bootsector.bytes_per_sector*i)), mount->bootsector.bytes_per_sector ) == -1 )
		{
			kprintf("FAT: fat_loadCluster failed to read buffer\n" );
			mm_free( buffer );
			return NULL;	
		}
	}
	return buffer;
}

int fat_compareName( struct FAT_ENTRY * entry, char * name )
{
	int i, x;
	char c;
	
	if( entry->name[0] == 0x00 || entry->name[0] == 0xE5 )
		return FALSE;
		
	if( entry->name[0] == 0x05 )
		entry->name[0] = 0xE5;
			
	c = entry->name[7];
	entry->name[7] = 0x00;
	kprintf( "fat_compareName = %s  to  %s\n", name, entry->name );
	entry->name[7] = c;

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

int fat_getEntryIndex( struct FAT_ENTRY * dir, char * name )
{
	int i;
	kprintf( "fat_getEntry = %s\n", name );
	for( i=0 ; i<16 ; i++ )
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
	int i;
	int root_dir_offset;
	//struct IO_CALLTABLE * calltable;
	
	mount0 = (struct FAT_MOUNTPOINT *)mm_malloc( sizeof(struct FAT_MOUNTPOINT) );
	
	//open the device we wish to mount
	mount0->device = vfs_open( device );
	if( mount0->device == NULL )
		return VFS_FAIL;
	kprintf("FAT: device %s open\n", device );
	
	// read in the bootsector
	vfs_read( mount0->device, (void *)&mount0->bootsector, sizeof(struct FAT_BOOTSECTOR) );
	kprintf("FAT: boot oem %s\n", mount0->bootsector.oem_id );
	// make sure we have a valid bootsector
	if( mount0->bootsector.magic != FAT_MAGIC )
	{
		kprintf("FAT: magic not found\n" );
		vfs_close( mount0->device );
		mm_free( mount0 );
		return VFS_FAIL;
	}
	
	// determine if we have a FAT 12, 16 or 32 filesystem
	fat_determineType( mount0 );
	
	// calculate clster size
	mount0->cluster_size = mount0->bootsector.bytes_per_sector * mount0->bootsector.sectors_per_cluster;

	mount0->fat_size = mount0->bootsector.sectors_per_fat * mount0->bootsector.bytes_per_sector;
	mount0->fat_data = (BYTE *)mm_malloc( mount0->fat_size );
	memset( mount0->fat_data, 0x00, mount0->fat_size );
	// read in the FAT
	for( i=0 ; i<mount0->bootsector.sectors_per_fat ; i++ )
		vfs_read( mount0->device, (void *)(mount0->fat_data+(mount0->bootsector.bytes_per_sector*i)), mount0->bootsector.bytes_per_sector );
		
	// read in root directory
	mount0->rootdir = (struct FAT_ENTRY *)mm_malloc( mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	memset( mount0->rootdir, 0x00, mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	
	root_dir_offset = (mount0->bootsector.num_fats * mount0->fat_size) + sizeof(struct FAT_BOOTSECTOR) + 1;
	
	vfs_seek( mount0->device, root_dir_offset, SEEK_START );

	vfs_read( mount0->device, (void *)(mount0->rootdir), mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );

	// display root dir
	//fat_displayDir( mount0, mount0->rootdir );
	
	//fat_open( NULL, "/BOOT/MENU.CFG" );
	
	//fat_open( NULL, "/TEST/does/NOT/exist.bin" );
	/*
	fat_open( NULL, "/NOWHERE/EXISTS/" );
	*/
	/*

	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open = fat_open;
	calltable->close = NULL;
	calltable->read = NULL;
	calltable->write = NULL;
	calltable->seek = NULL;
	calltable->control = NULL;

	device_add( "/mount/floppy/BOOT/MENU.CFG", calltable );
	*/
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
/*	int length, i, index = -1;
	char * curr_name;
	struct FAT_ENTRY * currdir;

	kprintf( "\nfat_open( %s )\n", filename );
	
	if( filename[0] == '/' )
		filename++;
		
	length = strlen( filename );
	
	if( filename[length] == '/' )
		filename[length] = '\0';
		
	curr_name = filename;
	
	currdir = mount0->rootdir;
	
	for( i=0 ; i<=length ; i++ )
	{
		if( filename[i] == '/' || filename[i] == '\0' )
		{
			filename[i] = '\0';

			index = fat_getEntryIndex( currdir, curr_name );
			if( index == -1 )
			{
				//kprintf( "index == -1\n" );
				break;
			} else {
				//kprintf( "got index = %d\n", index );
				currdir = (struct FAT_ENTRY *)fat_loadCluster( mount0, currdir[index].start_cluster );
			}
			
			curr_name = &filename[i]+1;
		}	
	}
	
	if( currdir == NULL )
		return NULL;
		
	if( index != -1 )
	{
		handle->data_arg = (DWORD)index;
		return handle;
	}
	*/
	return NULL;	
}

int fat_close( struct VFS_HANDLE * handle )
{
	return VFS_FAIL;
}

int fat_read( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size  )
{
	return VFS_FAIL;	
}

int fat_write( struct VFS_HANDLE * handle, BYTE * buffer, DWORD size )
{
	return VFS_FAIL;	
}

int fat_seek( struct VFS_HANDLE * handle, DWORD offset, BYTE origin )
{
	return VFS_FAIL;	
}

int fat_control( struct VFS_HANDLE * handle, DWORD request, DWORD arg )
{
	return VFS_FAIL;		
}

int fat_create( char * filename, int flags )
{
	return VFS_FAIL;	
}

int fat_delete( char * filename )
{
	return VFS_FAIL;	
}

int fat_rename( char * fromfilename, char * tofilename )
{
	return VFS_FAIL;
}

int fat_copy( char * fromfilename, char * tofilename )
{
	return VFS_FAIL;	
}

int fat_list( char * directoryname )
{
	return VFS_FAIL;
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
