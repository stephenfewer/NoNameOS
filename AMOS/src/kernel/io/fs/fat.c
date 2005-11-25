#include <kernel/io/fs/fat.h>
#include <kernel/io/io.h>
#include <kernel/kprintf.h>

struct FAT_MOUNTPOINT mount;

int fat_mount( char * filename )
{
	mount.device_handle = io_open( filename );
	if( mount.device_handle == NULL )
		return FALSE;
	kprintf("FAT: device %s open\n", filename );
	
	io_read( mount.device_handle, (void *)&mount.bootsector, sizeof(struct FAT_BOOTSECTOR) );
	
	kprintf("FAT: boot oem %s\n", mount.bootsector.oem_id );
	
	if( mount.bootsector.magic == FAT_MAGIC )
		kprintf("FAT: magic found\n" );
	
	kprintf("FAT: sectors_per_cluster %d\n", mount.bootsector.sectors_per_cluster );
	kprintf("FAT: num_fats %d\n", mount.bootsector.num_fats );
	kprintf("FAT: bytes_per_sector %d\n", mount.bootsector.bytes_per_sector );
	kprintf("FAT: sectors_per_fat %d\n", mount.bootsector.sectors_per_fat );
	
	
	return TRUE;
}
