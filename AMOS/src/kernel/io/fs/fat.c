#include <kernel/io/fs/fat.h>
#include <kernel/io/io.h>
#include <kernel/mm/mm.h>
#include <kernel/kprintf.h>
#include <kernel/lib/string.h>

struct FAT_MOUNTPOINT * mount0;

int fat_getEntry( struct FAT_MOUNTPOINT * mount, int index )
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
	io_seek( mount->device, (block*mount->bootsector.bytes_per_sector)+1, SEEK_START );
	// load in the blocks
	for( i=0 ; i<mount->bootsector.sectors_per_cluster ; i++ )
	{
		buffer += mount->bootsector.bytes_per_sector * i;
		if( io_read( mount->device, (void *)(buffer+(mount->bootsector.bytes_per_sector*i)), mount->bootsector.bytes_per_sector ) == -1 )
		{
			kprintf("FAT: fat_loadCluster failed to read buffer\n" );
			mm_free( buffer );
			return NULL;	
		}
	}
	return buffer;
}

void fat_displayDir( struct FAT_MOUNTPOINT * mount, struct FAT_ENTRY * dir )
{
	int x, next_cluster;
	
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
		
		next_cluster = fat_getEntry( mount, dir[x].start_cluster );
		while( TRUE )
		{
			if( next_cluster == 0x0000 )
				break;
			
			kprintf(" -> %x", next_cluster );
			
			next_cluster = fat_getEntry( mount, next_cluster );
		}
	}
}

int fat_mount( char * device_filename )
{
	int i;
	int root_dir_offset;
	struct FAT_ENTRY * dir;
	
	mount0 = (struct FAT_MOUNTPOINT *)mm_malloc( sizeof(struct FAT_MOUNTPOINT) );
	
	//open the device we wish to mount
	mount0->device = io_open( device_filename );
	if( mount0->device == NULL )
		return FALSE;
	kprintf("FAT: device %s open\n", device_filename );
	
	// read in the bootsector
	io_read( mount0->device, (void *)&mount0->bootsector, sizeof(struct FAT_BOOTSECTOR) );
	kprintf("FAT: boot oem %s\n", mount0->bootsector.oem_id );
	// make sure we have a valid bootsector
	if( mount0->bootsector.magic != FAT_MAGIC )
	{
		kprintf("FAT: magic not found\n" );
		io_close( mount0->device );
		mm_free( mount0 );
		return FALSE;
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
		io_read( mount0->device, (void *)(mount0->fat_data+(mount0->bootsector.bytes_per_sector*i)), mount0->bootsector.bytes_per_sector );
		
	// read in root directory
	dir = (struct FAT_ENTRY *)mm_malloc( mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	memset( dir, 0x00, mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ) );
	
	root_dir_offset = (mount0->bootsector.num_fats * mount0->fat_size) + sizeof(struct FAT_BOOTSECTOR) + 1;
	
	io_seek( mount0->device, root_dir_offset, SEEK_START );
	
	//((mount0->bootsector.num_root_dir_ents * sizeof( struct FAT_ENTRY ))/512)-1
	//for( i=0 ; i<1 ; i++ )
		io_read( mount0->device, (void *)(dir), 512 );

	// display root dir
	fat_displayDir( mount0, dir );
	
	// get dir in data section and display it
	// only works because first entry happens to be a dir
	dir = (struct FAT_ENTRY *)fat_loadCluster( mount0, 2 );
	fat_displayDir( mount0, dir );

	
	return TRUE;
}
