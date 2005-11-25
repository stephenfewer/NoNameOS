#ifndef _KERNEL_IO_FS_FAT_H_
#define _KERNEL_IO_FS_FAT_H_

#include <sys/types.h>
#include <kernel/io/io.h>

#define FAT_MAGIC	0xAA55

struct FAT_BOOTSECTOR
{
	BYTE jmp[3];
	BYTE oem_id[8];
	WORD bytes_per_sector;
	BYTE sectors_per_cluster;
	WORD num_boot_sectors;
	BYTE num_fats;
	WORD num_root_dir_ents;
	WORD total_sectors;
	BYTE media_id_byte;
	WORD sectors_per_fat;
	WORD sectors_per_track;
	WORD heads;
	DWORD hidden_sectors;
	DWORD total_sectors_large;
	BYTE boot_code[474];
	WORD magic;
} PACKED;  

struct FAT_ENTRY
{
	BYTE name[8];               /* ALL-CAPS, pad right with spaces */
	BYTE ext[3];                /* ALL-CAPS, pad right with spaces */
	BYTE attrib;                /* attribute byte */
	BYTE reserved;              /* =0 */
	BYTE ctimems;               /* file creation time, 10ms units */
	WORD ctime;              	/* file creation time, in DOS format */
	WORD cdate;              	/* file creation date, in DOS format */
	WORD adate;              	/* DOS date of last file access */
	WORD startclustermsw;       /* high 16 bits of starting cluster (FAT32) */
	WORD mtime;              	/* DOS time of last file modification */
	WORD mdate;              	/* DOS date of last file modification */
	WORD startcluster;          /* starting cluster */
	DWORD file_size;          	/* in bytes */
} PACKED; // 32 bytes

struct FAT_ATTRIBUTE
{
	int readonly:1;
	int hidden:1;
	int system:1;
	int volumelabel:1;
	int directory:1;
	int archive:1;
	int reserved:2;
} PACKED;

struct FAT_DOSTIME
{
	unsigned int twosecs:5;  /* low 5 bits: 2-second increments */
	unsigned int minutes:6;   /* middle 6 bits: minutes */
	unsigned int hours:5;     /* high 5 bits: hours (0-23) */
} PACKED;

struct FAT_DOSDATE
{
	unsigned int date:5;      /* low 5 bits: date (1-31) */
	unsigned int month:4;     /* middle 4 bits: month (1-12) */
	unsigned int year:7;      /* high 7 bits: year - 1980 */
} PACKED;  

struct FAT_MOUNTPOINT
{
	struct IO_HANDLE * device_handle;
	struct FAT_BOOTSECTOR bootsector;
};

int fat_mount( char * );

#endif
